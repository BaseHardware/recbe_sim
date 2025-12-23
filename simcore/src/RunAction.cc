#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "globals.hh"

namespace recbesim {
    RunAction::RunAction() {
        // set printing event number per each event
        G4RunManager::GetRunManager()->SetPrintProgress(1);
    }

    void RunAction::BeginOfRunAction(const G4Run * /*run*/) {
        // inform the runManager to save random number seed
        // G4RunManager::GetRunManager()->SetRandomNumberStore(true);
    }

    void RunAction::EndOfRunAction(const G4Run * /*run*/) {}
} // namespace recbesim
