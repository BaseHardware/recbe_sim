#include "eventviewer/EventViewer.h"

#include "EventViewerDetail.h"

#include "simobj/Metadata.h"
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"

#include "TClonesArray.h"
#include "TFile.h"
#include "TGFileDialog.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGTextView.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TSystem.h"
#include "TTree.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace eventviewer {
    using namespace detail;
    bool EventData::Open(const std::string &inputFilename) {
        file.reset(TFile::Open(inputFilename.c_str(), "READ"));
        if (!file || file->IsZombie()) {
            return false;
        }

        filename       = inputFilename;
        tree           = dynamic_cast<TTree *>(file->Get("tree"));
        persistentTree = dynamic_cast<TTree *>(file->Get("persistent"));
        if (!tree) {
            return false;
        }

        tracks  = nullptr;
        steps   = nullptr;
        primary = nullptr;
        tree->SetBranchAddress("complete", &complete);
        tree->SetBranchAddress("Tracks", &tracks);
        if (tree->GetBranch("Steps")) tree->SetBranchAddress("Steps", &steps);
        if (tree->GetBranch("Primary")) tree->SetBranchAddress("Primary", &primary);

        eventCount   = tree->GetEntries();
        currentEvent = 0;
        LoadMetadata();
        return true;
    }

    bool EventData::LoadMetadata() {
        metadata = nullptr;
        if (!persistentTree) return false;
        persistentTree->SetBranchAddress("Metadata", &metadata);
        persistentTree->GetEntry(0);

        return metadata != nullptr;
    }

    std::string EventData::MetadataText() const {
        if (!metadata) return "No metadata found.";

        std::ostringstream os;
        os << "Simulation: " << metadata->GetSimulationName() << '\n'
           << "Geometry type: " << metadata->GetGeometryType() << '\n'
           << "Output tree: " << metadata->GetOutputTreename() << '\n'
           << "Events: " << metadata->GetNumberOfProcessedEvents() << " processed / "
           << metadata->GetNumberOfRequestedEvents() << " requested\n"
           << "Threads: " << metadata->GetThreadNum() << '\n'
           << "Record steps: " << (metadata->GetAllStepRecorded() ? "yes" : "no") << '\n'
           << "Record primary: " << (metadata->GetPrimaryRecorded() ? "yes" : "no") << '\n'
           << "Max tracks: " << metadata->GetMaxTrackNum() << '\n'
           << "Max steps: " << metadata->GetMaxStepNum() << '\n'
           << "Random seeds: " << metadata->GetRandomSeed(false) << ", "
           << metadata->GetRandomSeed(true) << '\n'
           << "Git hash: " << metadata->GetGitHash() << '\n'
           << "Geometry payload: " << metadata->GetGeometryData().size() << " bytes\n";
        return os.str();
    }

    bool EventData::ImportGeometry(int graphicalVerbosity) {
        if (!metadata) return false;
        const auto geometry = metadata->GetGeometryData();
        if (geometry.empty()) return false;

        CleanupGeometryFile();
        geometryPath = std::string(gSystem->TempDirectory()) + "/evt_viewer_geometry_" +
                       std::to_string(gSystem->GetPid()) + ".gdml";

        std::ofstream out(geometryPath, std::ios::binary);
        if (!out) return false;
        size_t bytesToWrite = geometry.size();
        if (bytesToWrite > 0 && geometry.back() == 0) --bytesToWrite;
        out.write(reinterpret_cast<const char *>(geometry.data()), bytesToWrite);
        out.close();

        TGeoManager::Import(geometryPath.c_str());
        if (gGeoManager) {
            gGeoManager->SetVisLevel(GeometryVisLevel(graphicalVerbosity));
            gGeoManager->SetMaxVisNodes(GeometryMaxVisNodes(graphicalVerbosity));
            gGeoManager->SetNsegments(GeometrySegments(graphicalVerbosity));
            gGeoManager->SetVisOption(0);
        }
        return gGeoManager != nullptr;
    }

    void EventData::CleanupGeometryFile() {
        if (geometryPath.empty()) return;
        std::remove(geometryPath.c_str());
        geometryPath.clear();
    }

    bool EventData::LoadEntry(Long64_t eventIndex) {
        if (!tree || eventCount == 0) return false;
        currentEvent = std::clamp<Long64_t>(eventIndex, 0, eventCount - 1);
        tree->GetEntry(currentEvent);
        BuildStepToTrackMap();
        return true;
    }

    void EventData::BuildStepToTrackMap() {
        stepToTrackId.clear();
        if (!tracks) return;

        const int nTracks = tracks->GetEntriesFast();
        for (int i = 0; i < nTracks; ++i) {
            const auto *track = static_cast<simobj::Track *>(tracks->At(i));
            if (!track) continue;
            for (size_t j = 0; j < track->GetNStep(); ++j)
                stepToTrackId[(*track)[j]] = track->GetTrackID();
        }
    }

    std::string EventData::TrackListText() const {
        std::ostringstream os;
        os << "#  TrackID Parent PDG Particle Steps  Start(x,y,z,t)  Final(x,y,z,t)  FinalProc\n";
        if (!tracks) return os.str();

        const int nTracks = tracks->GetEntriesFast();
        for (int i = 0; i < nTracks; ++i) {
            const auto *track = static_cast<simobj::Track *>(tracks->At(i));
            if (track) os << FormatTrack(*track, i) << '\n';
        }
        return os.str();
    }

    std::string EventData::StepListText() const {
        std::ostringstream os;
        os << "#  TrackID  x y z t  Edep NIEdep KE  Volume Copy Envelope Process NDaughters\n";
        if (!steps) return os.str();

        const int nSteps = steps->GetEntriesFast();
        for (int i = 0; i < nSteps; ++i) {
            const auto *step = static_cast<simobj::Step *>(steps->At(i));
            if (!step) continue;
            const auto found = stepToTrackId.find(i);
            os << FormatStep(*step, i, found == stepToTrackId.end() ? -1 : found->second) << '\n';
        }
        return os.str();
    }

    bool EventViewer::OpenFile(const std::string &filename) {
        fRender.ClearEventPrimitives();
        fRender.cameraInitialized = false;
        if (!fEvent.Open(filename)) {
            LoadText(fUi.metadataText, "Failed to open ROOT file or missing TTree named 'tree': " +
                                           filename);
            return false;
        }

        fUi.eventEntry->SetLimitValues(0, std::max<Long64_t>(0, fEvent.eventCount - 1));
        fUi.eventEntry->SetIntNumber(0);
        fUi.fileLabel->SetText(CompactPath(filename).c_str());
        LoadText(fUi.metadataText, fEvent.MetadataText());
        fEvent.ImportGeometry(fView.graphicalVerbosity);
        UpdateGeometryTree();
        return true;
    }

    void EventViewer::OpenFileDialog() {
        static TString dir(".");
        TGFileInfo fileInfo;
        fileInfo.fIniDir    = StrDup(dir.Data());
        const char *types[] = {"ROOT files", "*.root", "All files", "*", nullptr, nullptr};
        fileInfo.fFileTypes = types;
        new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fileInfo);
        if (!fileInfo.fFilename) return;
        dir = gSystem->DirName(fileInfo.fFilename);
        if (OpenFile(fileInfo.fFilename)) LoadEvent();
    }

    void EventViewer::LoadEvent() {
        if (!fEvent.LoadEntry(fUi.eventEntry->GetIntNumber())) return;
        fUi.eventEntry->SetIntNumber(fEvent.currentEvent);

        UpdateEventSummary();
        UpdateObjectLists();
        RedrawEvent();
    }

    void EventViewer::NextEvent() {
        if (fEvent.currentEvent + 1 >= fEvent.eventCount) return;
        fUi.eventEntry->SetIntNumber(fEvent.currentEvent + 1);
        LoadEvent();
    }

    void EventViewer::PreviousEvent() {
        if (fEvent.currentEvent <= 0) return;
        fUi.eventEntry->SetIntNumber(fEvent.currentEvent - 1);
        LoadEvent();
    }
    void EventViewer::UpdateEventSummary() {
        std::ostringstream os;
        os << "Event " << fEvent.currentEvent << " / "
           << std::max<Long64_t>(0, fEvent.eventCount - 1)
           << "    complete=" << (fEvent.complete ? "yes" : "no")
           << "    tracks=" << (fEvent.tracks ? fEvent.tracks->GetEntriesFast() : 0)
           << "    steps=" << (fEvent.steps ? fEvent.steps->GetEntriesFast() : 0);
        fUi.summaryLabel->SetText(os.str().c_str());
    }

    void EventViewer::UpdateObjectLists() {
        LoadText(fUi.trackText, fEvent.TrackListText());
        LoadText(fUi.stepText, fEvent.StepListText());
    }
} // namespace eventviewer
