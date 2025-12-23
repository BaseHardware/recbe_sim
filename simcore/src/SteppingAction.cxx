#include "SteppingAction.h"

#include "DetectorConstruction.h"
#include "EventAction.h"

#include "G4Step.hh"
#include "globals.hh"

namespace simcore {
    SteppingAction::SteppingAction() {}

    void SteppingAction::UserSteppingAction(const G4Step *step) {}
} // namespace simcore
