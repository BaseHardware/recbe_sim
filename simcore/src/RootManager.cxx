#include "RootManager.h"

#include "TClonesArray.h"
#include "TTree.h"

#include "G4AutoLock.hh"

#include "simobj/Step.h"
#include "simobj/Track.h"

namespace simcore {
    RootManager RootManager::fgInstance;
    G4ThreadLocal TLSContainer *RootManager::fgTLS = nullptr;

    void RootManager::Fill() const { fgTLS->fTree->Fill(); }

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

        fgTLS = new TLSContainer{file, tree, tca_step, tca_track};

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
