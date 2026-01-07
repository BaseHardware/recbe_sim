#include "simcore/RunAction.h"
#include "simcore/MetadataManager.h"
#include "simcore/RootManager.h"

#include "G4Run.hh"
#include "G4RunManager.hh"

namespace simcore {
    RunAction::RunAction(bool m) : fMaster(m) {
        // set printing event number per each event
        G4RunManager::GetRunManager()->SetPrintProgress(1000);
    }

    void RunAction::BeginOfRunAction(const G4Run *run) {
        RootManager &rmInst     = RootManager::GetInstance();
        MetadataManager &mmInst = MetadataManager::GetInstance();

        mmInst.SetNumberOfRequestedEvents(run->GetNumberOfEventToBeProcessed());

        if (fMaster) {
            rmInst.StartRunMaster();
        } else {
            rmInst.StartRunSlave();
        }
    }

    void RunAction::EndOfRunAction(const G4Run *run) {
        RootManager &rmInst     = RootManager::GetInstance();
        MetadataManager &mmInst = MetadataManager::GetInstance();

        mmInst.SetNumberOfProcessedEvents(run->GetNumberOfEvent());

        if (fMaster) {
            rmInst.EndRunMaster();
        } else {
            rmInst.EndRunSlave();
        }
    }
} // namespace simcore
