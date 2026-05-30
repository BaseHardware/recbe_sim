#include "eventviewer/EventViewer.h"

#include "simobj/Metadata.h"
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"

#include "TCanvas.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGButton.h"
#include "TGClient.h"
#include "TGFileDialog.h"
#include "TGLabel.h"
#include "TGLEmbeddedViewer.h"
#include "TGLEventHandler.h"
#include "TGNumberEntry.h"
#include "TGTab.h"
#include "TGTextView.h"
#include "TGLCamera.h"
#include "TGLViewer.h"
#include "TGLWidget.h"
#include "TGLUtil.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"
#include "TView.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

ClassImp(eventviewer::EventViewer)

namespace {
    class ShiftTruckEventHandler : public TGLEventHandler {
      public:
        ShiftTruckEventHandler(TGWindow *window, TObject *object)
            : TGLEventHandler(window, object), fShiftTruck(false), fLastX(0), fLastY(0) {}

        Bool_t HandleButton(Event_t *event) override {
            if (event->fCode == kButton1 && (event->fState & kKeyShiftMask)) {
                if (event->fType == kButtonPress) {
                    BeginShiftTruck(*event);
                    return kTRUE;
                }
                if (event->fType == kButtonRelease) {
                    EndShiftTruck(*event);
                    return kTRUE;
                }
            }
            if (event->fType == kButtonRelease) EndShiftTruck(*event);
            return TGLEventHandler::HandleButton(event);
        }

        Bool_t HandleMotion(Event_t *event) override {
            if ((event->fState & kKeyShiftMask) && (event->fState & kButton1Mask) && !fShiftTruck) {
                Event_t release = *event;
                release.fType = kButtonRelease;
                release.fCode = kButton1;
                TGLEventHandler::HandleButton(&release);
                BeginShiftTruck(*event);
                return kTRUE;
            }

            if (fShiftTruck && fGLViewer) {
                if (!(event->fState & kKeyShiftMask)) {
                    EndShiftTruck(*event);
                    return kTRUE;
                }

                const int dx = event->fX - fLastX;
                const int dy = event->fY - fLastY;
                SyncMouseState(*event);
                if (dx != 0 || dy != 0) {
                    fGLViewer->SetResetCamerasOnUpdate(false);
                    fGLViewer->CurrentCamera().Truck(dx, -dy, kFALSE, kFALSE);
                    fGLViewer->RequestDraw();
                }
                return kTRUE;
            }
            return TGLEventHandler::HandleMotion(event);
        }

      private:
        void BeginShiftTruck(const Event_t &event) {
            fShiftTruck = true;
            StopMouseTimer();
            SyncMouseState(event);
        }

        void EndShiftTruck(const Event_t &event) {
            if (!fShiftTruck) return;
            fShiftTruck = false;
            SyncMouseState(event);
        }

        void SyncMouseState(const Event_t &event) {
            fLastX = event.fX;
            fLastY = event.fY;
            fLastPos.fX = event.fX;
            fLastPos.fY = event.fY;
            fLastGlobalPos.fX = event.fXRoot;
            fLastGlobalPos.fY = event.fYRoot;
            fLastEventState = event.fState;
        }

        bool fShiftTruck;
        int fLastX;
        int fLastY;
    };

    std::string CompactPath(const std::string &path) {
        constexpr size_t maxLen = 86;
        if (path.size() <= maxLen) return path;
        return path.substr(0, 34) + "..." + path.substr(path.size() - 49);
    }

    void LoadText(TGTextView *view, const std::string &text) {
        view->Clear();
        view->AddLine(text.c_str());
        view->Update();
    }

    TGNumberEntry *AddVectorEntry(TGCompositeFrame *parent, const char *label, double value) {
        auto *row = new TGHorizontalFrame(parent);
        parent->AddFrame(row, new TGLayoutHints(kLHintsExpandX, 0, 0, 2, 2));
        row->AddFrame(new TGLabel(row, label), new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 6, 0, 0));
        auto *entry = new TGNumberEntry(row, value, 8, -1, TGNumberFormat::kNESRealThree,
                                        TGNumberFormat::kNEAAnyNumber);
        row->AddFrame(entry, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY));
        return entry;
    }
} // namespace

namespace eventviewer {
    EventViewer::EventViewer(const TGWindow *parent, const char *filename)
        : TGMainFrame(parent, 1900, 1080), fTree(nullptr), fPersistentTree(nullptr),
          fComplete(false), fTracks(nullptr), fSteps(nullptr), fPrimary(nullptr), fMetadata(nullptr),
          fCurrentEvent(0), fEventCount(0), fEventEntry(nullptr), fFileLabel(nullptr),
          fSummaryLabel(nullptr), fShowGeometry(nullptr), fShowTracks(nullptr), fShowSteps(nullptr),
          fMetadataText(nullptr), fTrackText(nullptr), fStepText(nullptr), fViewXEntry(nullptr),
          fViewYEntry(nullptr), fViewZEntry(nullptr), fUpXEntry(nullptr), fUpYEntry(nullptr),
          fUpZEntry(nullptr), fCanvas(nullptr), fGLViewer(nullptr), fGLHandler(nullptr),
          fCameraInitialized(false) {
        BuildUi();
        OpenFile(filename);
        MapSubwindows();
        Resize(1900, 1080);
        MapWindow();
        LoadEvent();
    }

    EventViewer::~EventViewer() {
        ClearEventPrimitives();
        delete fGLHandler;
        delete fCanvas;
        if (!fGeometryPath.empty()) std::remove(fGeometryPath.c_str());
        Cleanup();
    }

    void EventViewer::CloseWindow() {
        DeleteWindow();
    }

    void EventViewer::BuildUi() {
        SetWindowName("Geant4 Event Viewer");

        auto *top = new TGVerticalFrame(this);
        AddFrame(top, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *toolbar = new TGHorizontalFrame(top, 10, 10);
        top->AddFrame(toolbar, new TGLayoutHints(kLHintsExpandX, 6, 6, 6, 2));

        auto *openButton = new TGTextButton(toolbar, "Open");
        openButton->Connect("Clicked()", "eventviewer::EventViewer", this, "OpenFileDialog()");
        toolbar->AddFrame(openButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 6, 0, 0));

        auto *prevButton = new TGTextButton(toolbar, "Prev");
        prevButton->Connect("Clicked()", "eventviewer::EventViewer", this, "PreviousEvent()");
        toolbar->AddFrame(prevButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 4, 0, 0));

        auto *nextButton = new TGTextButton(toolbar, "Next");
        nextButton->Connect("Clicked()", "eventviewer::EventViewer", this, "NextEvent()");
        toolbar->AddFrame(nextButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 8, 0, 0));

        toolbar->AddFrame(new TGLabel(toolbar, "Event"), new TGLayoutHints(kLHintsCenterY, 0, 4, 0, 0));
        fEventEntry =
            new TGNumberEntry(toolbar, 0, 7, -1, TGNumberFormat::kNESInteger,
                              TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 0);
        fEventEntry->Connect("ValueSet(Long_t)", "eventviewer::EventViewer", this, "LoadEvent()");
        toolbar->AddFrame(fEventEntry, new TGLayoutHints(kLHintsCenterY, 0, 10, 0, 0));

        fShowGeometry = new TGCheckButton(toolbar, "Geometry");
        fShowGeometry->SetState(kButtonDown);
        fShowGeometry->Connect("Clicked()", "eventviewer::EventViewer", this, "ToggleGeometry()");
        toolbar->AddFrame(fShowGeometry, new TGLayoutHints(kLHintsCenterY, 0, 6, 0, 0));

        fShowTracks = new TGCheckButton(toolbar, "Tracks");
        fShowTracks->SetState(kButtonDown);
        fShowTracks->Connect("Clicked()", "eventviewer::EventViewer", this, "ToggleTracks()");
        toolbar->AddFrame(fShowTracks, new TGLayoutHints(kLHintsCenterY, 0, 6, 0, 0));

        fShowSteps = new TGCheckButton(toolbar, "Steps");
        fShowSteps->SetState(kButtonUp);
        fShowSteps->Connect("Clicked()", "eventviewer::EventViewer", this, "ToggleSteps()");
        toolbar->AddFrame(fShowSteps, new TGLayoutHints(kLHintsCenterY, 0, 10, 0, 0));

        fFileLabel = new TGLabel(toolbar, "");
        toolbar->AddFrame(fFileLabel, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY));

        auto *main = new TGHorizontalFrame(top);
        top->AddFrame(main, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 6, 6, 0, 6));

        auto *left = new TGVerticalFrame(main, 280, 920);
        main->AddFrame(left, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 0, 8, 0, 0));

        fSummaryLabel = new TGLabel(left, "No file loaded");
        left->AddFrame(fSummaryLabel, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 6));

        auto *tabs = new TGTab(left, 280, 880);
        left->AddFrame(tabs, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *trackTab = tabs->AddTab("Tracks");
        fTrackText = new TGTextView(trackTab, 260, 820);
        trackTab->AddFrame(fTrackText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *stepTab = tabs->AddTab("Steps");
        fStepText = new TGTextView(stepTab, 260, 820);
        stepTab->AddFrame(fStepText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *metaTab = tabs->AddTab("Metadata");
        fMetadataText = new TGTextView(metaTab, 260, 820);
        metaTab->AddFrame(fMetadataText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *viewTab = tabs->AddTab("View");
        viewTab->AddFrame(new TGLabel(viewTab, "View vector"),
                          new TGLayoutHints(kLHintsLeft, 0, 0, 6, 2));
        fViewXEntry = AddVectorEntry(viewTab, "X", -1.0);
        fViewYEntry = AddVectorEntry(viewTab, "Y", 0.0);
        fViewZEntry = AddVectorEntry(viewTab, "Z", 0.0);
        viewTab->AddFrame(new TGLabel(viewTab, "Up vector"),
                          new TGLayoutHints(kLHintsLeft, 0, 0, 10, 2));
        fUpXEntry = AddVectorEntry(viewTab, "X", 0.0);
        fUpYEntry = AddVectorEntry(viewTab, "Y", 1.0);
        fUpZEntry = AddVectorEntry(viewTab, "Z", 0.0);
        auto *viewButtons = new TGHorizontalFrame(viewTab);
        viewTab->AddFrame(viewButtons, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
        auto *applyViewButton = new TGTextButton(viewButtons, "Apply");
        applyViewButton->Connect("Clicked()", "eventviewer::EventViewer", this, "ApplyCameraFromUi()");
        viewButtons->AddFrame(applyViewButton, new TGLayoutHints(kLHintsExpandX, 0, 4, 0, 0));
        auto *resetViewButton = new TGTextButton(viewButtons, "Reset");
        resetViewButton->Connect("Clicked()", "eventviewer::EventViewer", this, "ResetCameraControls()");
        viewButtons->AddFrame(resetViewButton, new TGLayoutHints(kLHintsExpandX, 4, 0, 0, 0));

        const bool wasBatch = gROOT->IsBatch();
        gROOT->SetBatch(kTRUE);
        fCanvas = new TCanvas("event_backing_canvas", "event_backing_canvas", 1550, 920);
        gROOT->SetBatch(wasBatch);
        fGLViewer = new TGLEmbeddedViewer(main, fCanvas);
        fGLViewer->GetFrame()->Resize(1550, 920);
        main->AddFrame(fGLViewer->GetFrame(),
                       new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        fGLViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
        fGLViewer->SetResetCamerasOnUpdate(false);
        fGLHandler = new ShiftTruckEventHandler(nullptr, fGLViewer);
        fGLViewer->GetGLWidget()->SetEventHandler(fGLHandler);
    }

    bool EventViewer::OpenFile(const std::string &filename) {
        ClearEventPrimitives();
        fCameraInitialized = false;
        fFile.reset(TFile::Open(filename.c_str(), "READ"));
        if (!fFile || fFile->IsZombie()) {
            LoadText(fMetadataText, "Failed to open ROOT file: " + filename);
            return false;
        }

        fFilename = filename;
        fTree = dynamic_cast<TTree *>(fFile->Get("tree"));
        fPersistentTree = dynamic_cast<TTree *>(fFile->Get("persistent"));
        if (!fTree) {
            LoadText(fMetadataText, "Missing TTree named 'tree'");
            return false;
        }

        fTracks = nullptr;
        fSteps = nullptr;
        fPrimary = nullptr;
        fTree->SetBranchAddress("complete", &fComplete);
        fTree->SetBranchAddress("Tracks", &fTracks);
        if (fTree->GetBranch("Steps")) fTree->SetBranchAddress("Steps", &fSteps);
        if (fTree->GetBranch("Primary")) fTree->SetBranchAddress("Primary", &fPrimary);

        fEventCount = fTree->GetEntries();
        fCurrentEvent = 0;
        fEventEntry->SetLimitValues(0, std::max<Long64_t>(0, fEventCount - 1));
        fEventEntry->SetIntNumber(0);
        fFileLabel->SetText(CompactPath(filename).c_str());

        LoadMetadata();
        ImportGeometry();
        return true;
    }

    bool EventViewer::LoadMetadata() {
        fMetadata = nullptr;
        if (!fPersistentTree) return false;
        fPersistentTree->SetBranchAddress("Metadata", &fMetadata);
        fPersistentTree->GetEntry(0);

        if (!fMetadata) return false;

        std::ostringstream os;
        os << "Simulation: " << fMetadata->GetSimulationName() << '\n'
           << "Geometry type: " << fMetadata->GetGeometryType() << '\n'
           << "Output tree: " << fMetadata->GetOutputTreename() << '\n'
           << "Events: " << fMetadata->GetNumberOfProcessedEvents() << " processed / "
           << fMetadata->GetNumberOfRequestedEvents() << " requested\n"
           << "Threads: " << fMetadata->GetThreadNum() << '\n'
           << "Record steps: " << (fMetadata->GetAllStepRecorded() ? "yes" : "no") << '\n'
           << "Record primary: " << (fMetadata->GetPrimaryRecorded() ? "yes" : "no") << '\n'
           << "Max tracks: " << fMetadata->GetMaxTrackNum() << '\n'
           << "Max steps: " << fMetadata->GetMaxStepNum() << '\n'
           << "Random seeds: " << fMetadata->GetRandomSeed(false) << ", "
           << fMetadata->GetRandomSeed(true) << '\n'
           << "Git hash: " << fMetadata->GetGitHash() << '\n'
           << "Geometry payload: " << fMetadata->GetGeometryData().size() << " bytes\n";
        LoadText(fMetadataText, os.str());
        return true;
    }

    bool EventViewer::ImportGeometry() {
        if (!fMetadata) return false;
        const auto geometry = fMetadata->GetGeometryData();
        if (geometry.empty()) return false;

        if (!fGeometryPath.empty()) std::remove(fGeometryPath.c_str());
        fGeometryPath = std::string(gSystem->TempDirectory()) + "/evt_viewer_geometry_" +
                        std::to_string(gSystem->GetPid()) + ".gdml";

        std::ofstream out(fGeometryPath, std::ios::binary);
        if (!out) return false;
        size_t bytesToWrite = geometry.size();
        if (bytesToWrite > 0 && geometry.back() == 0) --bytesToWrite;
        out.write(reinterpret_cast<const char *>(geometry.data()), bytesToWrite);
        out.close();

        TGeoManager::Import(fGeometryPath.c_str());
        if (gGeoManager) {
            gGeoManager->SetVisLevel(4);
            gGeoManager->SetVisOption(0);
        }
        return gGeoManager != nullptr;
    }

    void EventViewer::OpenFileDialog() {
        static TString dir(".");
        TGFileInfo fileInfo;
        fileInfo.fIniDir = StrDup(dir.Data());
        const char *types[] = {"ROOT files", "*.root", "All files", "*", nullptr, nullptr};
        fileInfo.fFileTypes = types;
        new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fileInfo);
        if (!fileInfo.fFilename) return;
        dir = gSystem->DirName(fileInfo.fFilename);
        if (OpenFile(fileInfo.fFilename)) LoadEvent();
    }

    void EventViewer::LoadEvent() {
        if (!fTree || fEventCount == 0) return;
        fCurrentEvent = fEventEntry->GetIntNumber();
        fCurrentEvent = std::clamp<Long64_t>(fCurrentEvent, 0, fEventCount - 1);
        fEventEntry->SetIntNumber(fCurrentEvent);

        fTree->GetEntry(fCurrentEvent);
        fStepToTrackId.clear();
        if (fTracks) {
            const int nTracks = fTracks->GetEntriesFast();
            for (int i = 0; i < nTracks; ++i) {
                const auto *track = static_cast<simobj::Track *>(fTracks->At(i));
                if (!track) continue;
                for (size_t j = 0; j < track->GetNStep(); ++j) {
                    fStepToTrackId[(*track)[j]] = track->GetTrackID();
                }
            }
        }

        UpdateEventSummary();
        UpdateObjectLists();
        RedrawEvent();
    }

    void EventViewer::NextEvent() {
        if (fCurrentEvent + 1 >= fEventCount) return;
        fEventEntry->SetIntNumber(fCurrentEvent + 1);
        LoadEvent();
    }

    void EventViewer::PreviousEvent() {
        if (fCurrentEvent <= 0) return;
        fEventEntry->SetIntNumber(fCurrentEvent - 1);
        LoadEvent();
    }

    void EventViewer::ToggleGeometry() { RedrawEvent(); }
    void EventViewer::ToggleTracks() { RedrawEvent(); }
    void EventViewer::ToggleSteps() { RedrawEvent(); }

    void EventViewer::ApplyCameraFromUi() {
        if (!fGLViewer) return;

        const double vx = fViewXEntry->GetNumber();
        const double vy = fViewYEntry->GetNumber();
        const double vz = fViewZEntry->GetNumber();
        const double ux = fUpXEntry->GetNumber();
        const double uy = fUpYEntry->GetNumber();
        const double uz = fUpZEntry->GetNumber();

        const double viewNorm2 = vx * vx + vy * vy + vz * vz;
        const double upNorm2 = ux * ux + uy * uy + uz * uz;
        const double cx = vy * uz - vz * uy;
        const double cy = vz * ux - vx * uz;
        const double cz = vx * uy - vy * ux;
        const double crossNorm2 = cx * cx + cy * cy + cz * cz;

        if (viewNorm2 < 1e-12 || upNorm2 < 1e-12 ||
            crossNorm2 / (viewNorm2 * upNorm2) < 1e-8) {
            fSummaryLabel->SetText("Camera vectors are invalid or nearly parallel.");
            return;
        }

        fGLViewer->ReinitializeCurrentCamera(TGLVector3(vx, vy, vz), TGLVector3(ux, uy, uz));
        fCameraInitialized = true;
        UpdateEventSummary();
    }

    void EventViewer::ResetCameraControls() {
        fViewXEntry->SetNumber(-1.0);
        fViewYEntry->SetNumber(0.0);
        fViewZEntry->SetNumber(0.0);
        fUpXEntry->SetNumber(0.0);
        fUpYEntry->SetNumber(1.0);
        fUpZEntry->SetNumber(0.0);
        fCameraInitialized = false;
        ApplyCameraFromUi();
    }

    void EventViewer::UpdateEventSummary() {
        std::ostringstream os;
        os << "Event " << fCurrentEvent << " / " << std::max<Long64_t>(0, fEventCount - 1)
           << "    complete=" << (fComplete ? "yes" : "no")
           << "    tracks=" << (fTracks ? fTracks->GetEntriesFast() : 0)
           << "    steps=" << (fSteps ? fSteps->GetEntriesFast() : 0);
        fSummaryLabel->SetText(os.str().c_str());
    }

    void EventViewer::UpdateObjectLists() {
        std::ostringstream tracks;
        tracks << "#  TrackID Parent PDG Particle Steps  Start(x,y,z,t)  Final(x,y,z,t)  FinalProc\n";
        if (fTracks) {
            const int nTracks = fTracks->GetEntriesFast();
            for (int i = 0; i < nTracks; ++i) {
                const auto *track = static_cast<simobj::Track *>(fTracks->At(i));
                if (track) tracks << FormatTrack(*track, i) << '\n';
            }
        }
        LoadText(fTrackText, tracks.str());

        std::ostringstream steps;
        steps << "#  TrackID  x y z t  Edep NIEdep KE  Volume Copy Envelope Process NDaughters\n";
        if (fSteps) {
            const int nSteps = fSteps->GetEntriesFast();
            for (int i = 0; i < nSteps; ++i) {
                const auto *step = static_cast<simobj::Step *>(fSteps->At(i));
                if (!step) continue;
                const auto found = fStepToTrackId.find(i);
                steps << FormatStep(*step, i, found == fStepToTrackId.end() ? -1 : found->second)
                      << '\n';
            }
        }
        LoadText(fStepText, steps.str());
    }

    void EventViewer::RedrawEvent() {
        if (!fCanvas) return;
        ClearEventPrimitives();
        fCanvas->cd();
        fCanvas->Clear();

        if (fShowGeometry->IsOn()) DrawGeometry();
        if (fShowTracks->IsOn()) DrawTrackLines();
        if (fShowSteps->IsOn()) DrawStepMarkers();

        if (fGLViewer) {
            fGLViewer->SetResetCamerasOnUpdate(false);
            fGLViewer->PadPaint(fCanvas);
            ApplyDefaultCamera();
        }

        fCanvas->Modified();
    }

    void EventViewer::DrawGeometry() {
        if (!gGeoManager || !gGeoManager->GetTopVolume()) return;
        gGeoManager->GetTopVolume()->Draw();
    }

    void EventViewer::DrawTrackLines() {
        if (!fTracks) return;
        const int nTracks = fTracks->GetEntriesFast();
        for (int i = 0; i < nTracks; ++i) {
            const auto *track = static_cast<simobj::Track *>(fTracks->At(i));
            if (!track) continue;

            std::vector<const simobj::Step *> points;
            points.push_back(&track->GetFirstStep());
            if (fSteps) {
                for (size_t j = 0; j < track->GetNStep(); ++j) {
                    auto *step = static_cast<simobj::Step *>(fSteps->At((*track)[j]));
                    if (step) points.push_back(step);
                }
            }
            points.push_back(&track->GetFinalStep());
            if (points.size() < 2) continue;

            auto *line = new TPolyLine3D(points.size());
            for (int p = 0; p < static_cast<int>(points.size()); ++p) {
                line->SetPoint(p, DisplayLength(points[p]->GetX()),
                               DisplayLength(points[p]->GetY()), DisplayLength(points[p]->GetZ()));
            }
            line->SetLineColor(TrackColor(*track));
            line->SetLineWidth(track->GetParentID() == 0 ? 3 : 1);
            line->Draw(fShowGeometry->IsOn() ? "same" : "");
            fEventPrimitives.push_back(line);
        }
    }

    void EventViewer::DrawStepMarkers() {
        if (!fSteps) return;
        auto *markers = new TPolyMarker3D(fSteps->GetEntriesFast());
        for (int i = 0; i < fSteps->GetEntriesFast(); ++i) {
            const auto *step = static_cast<simobj::Step *>(fSteps->At(i));
            if (!step) continue;
            markers->SetPoint(i, DisplayLength(step->GetX()), DisplayLength(step->GetY()),
                              DisplayLength(step->GetZ()));
        }
        markers->SetMarkerStyle(6);
        markers->SetMarkerColor(kOrange + 7);
        markers->Draw((fShowGeometry->IsOn() || fShowTracks->IsOn()) ? "same" : "");
        fEventPrimitives.push_back(markers);
    }

    void EventViewer::ClearEventPrimitives() {
        for (auto *object : fEventPrimitives) delete object;
        fEventPrimitives.clear();
    }

    void EventViewer::ApplyDefaultCamera() {
        if (!fGLViewer || fCameraInitialized) return;
        fGLViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
        fGLViewer->ResetCurrentCamera();
        fCameraInitialized = true;
    }

    int EventViewer::TrackColor(const simobj::Track &track) {
        switch (track.GetPDGCode()) {
        case 2112:
            return kGreen + 2;
        case 22:
            return kOrange + 7;
        case 11:
        case -11:
            return kAzure + 1;
        case 2212:
            return kRed + 1;
        default:
            return track.GetParentID() == 0 ? kMagenta + 2 : kGray + 2;
        }
    }

    std::string EventViewer::FormatTrack(const simobj::Track &track, int idx) {
        const auto &first = track.GetFirstStep();
        const auto &final = track.GetFinalStep();
        std::ostringstream os;
        os << std::fixed << std::setprecision(3) << idx << "  " << track.GetTrackID() << ' '
           << track.GetParentID() << ' ' << track.GetPDGCode() << ' ' << track.GetName() << ' '
           << track.GetNStep() << "  (" << first.GetX() << ", " << first.GetY() << ", "
           << first.GetZ() << ", " << first.GetGlobalTime() << ")  (" << final.GetX() << ", "
           << final.GetY() << ", " << final.GetZ() << ", " << final.GetGlobalTime() << ")  "
           << final.GetProcessName();
        return os.str();
    }

    std::string EventViewer::FormatStep(const simobj::Step &step, int idx, int trackId) {
        std::ostringstream os;
        os << std::fixed << std::setprecision(4) << idx << "  " << trackId << "  " << step.GetX()
           << ' ' << step.GetY() << ' ' << step.GetZ() << ' ' << step.GetGlobalTime() << "  "
           << step.GetDepositedEnergy() << ' ' << step.GetNonIonDepositedEnergy() << ' '
           << step.GetKineticEnergy() << "  " << step.GetVolumeName() << ' ' << step.GetCopyNumber()
           << ' ' << step.GetEnvelopeCopyNumber() << ' ' << step.GetProcessName() << ' '
           << step.GetNDaughters();
        return os.str();
    }
} // namespace eventviewer
