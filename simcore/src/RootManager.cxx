#include "RootManager.h"

#include "TClonesArray.h"
#include "TTree.h"

#include "G4AutoLock.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include "simobj/Step.h"
#include "simobj/Track.h"

static void G4Step2SimStep(const G4Step *src, simobj::Step *dest) {
    G4ThreeVector pos = src->GetPostStepPoint()->GetPosition();
    G4double glob_t   = src->GetPostStepPoint()->GetGlobalTime();

    G4ThreeVector mom = src->GetPostStepPoint()->GetMomentum();
    G4double energy   = src->GetPostStepPoint()->GetKineticEnergy();

    G4double prop_t = src->GetPostStepPoint()->GetProperTime();
    G4double edep   = src->GetTotalEnergyDeposit();

    G4int nDaug = src->GetNumberOfSecondariesInCurrentStep();

    std::string procName, volName;

    const G4VProcess *nowProcess = src->GetPostStepPoint()->GetProcessDefinedStep();
    if (nowProcess == nullptr) {
        procName = "FirstStep";
    } else {
        procName = nowProcess->GetProcessName();
    }

    const G4VPhysicalVolume *nowVolume = src->GetPostStepPoint()->GetPhysicalVolume();
    if (nowVolume == nullptr) {
        volName = "OutOfWorld";
    } else {
        volName = nowVolume->GetName();
    }

    dest->SetNDaughters(nDaug);
    dest->SetDepositedEnergy(edep / MeV);
    dest->SetProperTime(prop_t / ns);
    dest->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    dest->SetPxPyPzE(mom.x() / mm, mom.y() / mm, mom.z() / mm, energy / MeV);
    dest->SetProcessName(procName.c_str());
    dest->SetVolumeName(volName.c_str());
}

static void G4Track2SimStep(const G4Track *src, simobj::Step *dest) {
    G4ThreeVector pos = src->GetPosition();
    G4double glob_t   = src->GetGlobalTime();

    G4ThreeVector mom = src->GetMomentum();
    G4double energy   = src->GetKineticEnergy();

    G4double prop_t = src->GetProperTime();
    G4double edep   = 0;

    G4int nDaug = 0;

    std::string procName, volName;

    procName = "FirstStep";

    volName = src->GetVolume()->GetName();

    dest->SetNDaughters(nDaug);
    dest->SetDepositedEnergy(edep / MeV);
    dest->SetProperTime(prop_t / ns);
    dest->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    dest->SetPxPyPzE(mom.x() / mm, mom.y() / mm, mom.z() / mm, energy / MeV);
    dest->SetProcessName(procName.c_str());
    dest->SetVolumeName(volName.c_str());
}

namespace simcore {
    RootManager RootManager::fgInstance;
    G4ThreadLocal TLSContainer *RootManager::fgTLS = nullptr;

    void RootManager::Fill() const { fgTLS->fTree->Fill(); }

    void RootManager::Clear() const {
        fgTLS->fTCAStep->Clear("C");
        fgTLS->fTCATrack->Clear("C");

        fgTLS->fNTrack = 0;
        fgTLS->fNStep  = 0;

        fgTLS->fID2IdxTable.clear();
    }

    bool RootManager::StartTrack(const G4Track *track) const {
        using namespace simobj;

        if (fgcMaxTrackNum <= fgTLS->fNTrack) {
            G4cerr << "WARNING: The number of tracks exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        fgTLS->fID2IdxTable[track->GetTrackID()] = fgTLS->fNTrack;

        Track *newTrack = new ((*fgTLS->fTCATrack)[fgTLS->fNTrack++]) Track(
            track->GetDefinition()->GetPDGEncoding(), track->GetDefinition()->GetParticleName(),
            track->GetTrackID(), track->GetParentID());
        newTrack->AppendStepIdx(fgTLS->fNStep);

        Step *newStep = (new ((*fgTLS->fTCAStep)[fgTLS->fNStep++]) Step());

        G4Track2SimStep(track, newStep);
        return true;
    }

    bool RootManager::AppendStep(const G4Step *step) const {
        using namespace simobj;

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This step will not be added." << G4endl;
            return false;
        }

        const G4Track *track = step->GetTrack();
        Track *tcaTrack =
            static_cast<Track *>((*fgTLS->fTCATrack)[fgTLS->fID2IdxTable[track->GetTrackID()]]);
        tcaTrack->AppendStepIdx(fgTLS->fNStep);

        Step *newStep = (new ((*fgTLS->fTCAStep)[fgTLS->fNStep++]) Step());

        G4Step2SimStep(step, newStep);
        return true;
    }

    bool RootManager::StartRunMaster() {
        G4AutoLock lock(&fgcMasterMutex);
        if (fStarted) return false;

        fStarted = true;
        fMerger  = new ROOT::TBufferMerger(fFilename.c_str());

        return true;
    }
    bool RootManager::EndRunMaster() {
        G4AutoLock lock(&fgcMasterMutex);

        if (!fStarted) return false;
        delete fMerger;

        fStarted = false;
        fMerger  = nullptr;
        return true;
    }

    bool RootManager::StartRunSlave() {
        G4AutoLock lock(&fgcSlaveMutex);
        if (!fStarted || fgTLS) return false;

        auto file = fMerger->GetFile();

        TTree *tree = new TTree(fTreename.c_str(), "The output TTree instance");
        if (tree == nullptr) return false;
        tree->ResetBit(kMustCleanup);
        tree->SetDirectory(file.get());

        TClonesArray *tca_step  = new TClonesArray("simobj::Step", fgcMaxStepNum);
        TClonesArray *tca_track = new TClonesArray("simobj::Track", fgcMaxTrackNum);

        tree->Branch("Step", &tca_step);
        tree->Branch("Track", &tca_track);

        fgTLS = new TLSContainer{file, tree, 0, {}, tca_track, 0, tca_step};

        return true;
    }

    bool RootManager::EndRunSlave() {
        G4AutoLock lock(&fgcSlaveMutex);
        if (!fStarted || !fgTLS) return false;

        fgTLS->fFile->Write();

        delete fgTLS->fTree;
        delete fgTLS->fTCAStep;
        delete fgTLS->fTCATrack;
        delete fgTLS;
        fgTLS = nullptr;

        return true;
    }
} // namespace simcore
