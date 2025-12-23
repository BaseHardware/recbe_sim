#include "RunAction.h"
#include "RootManager.h"

#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "globals.hh"

namespace simcore {
    RunAction::RunAction(bool m) : fMaster(m) {
        // set printing event number per each event
        G4RunManager::GetRunManager()->SetPrintProgress(1000);
    }

    void RunAction::BeginOfRunAction(const G4Run * /*run*/) {
        RootManager &inst = RootManager::GetInstance();

        if (fMaster) {
            inst.StartRunMaster();
        } else {
            inst.StartRunSlave();
        }
    }

    void RunAction::EndOfRunAction(const G4Run * /*run*/) {
        RootManager &inst = RootManager::GetInstance();

        if (fMaster) {
            inst.EndRunMaster();
        } else {
            inst.EndRunSlave();
        }
    }
} // namespace simcore
