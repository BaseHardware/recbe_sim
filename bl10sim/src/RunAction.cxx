#include "bl10sim/RunAction.h"
#include "bl10sim/PrimaryGeneratorAction.h"

namespace bl10sim {
    RunAction::RunAction(PrimaryGeneratorAction *pg) : fPrimGenAction(pg) {}

    void RunAction::BeginAction(const G4Run *) { fPrimGenAction->InitializeGenerators(); }
} // namespace bl10sim
