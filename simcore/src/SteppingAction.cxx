#include "SteppingAction.h"

#include "RootManager.h"

#include "G4Step.hh"

namespace simcore {
    SteppingAction::SteppingAction() {}

    void SteppingAction::UserSteppingAction(const G4Step *step) {
        RootManager::GetInstance().AppendStep(step);
    }
} // namespace simcore
