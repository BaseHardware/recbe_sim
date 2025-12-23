#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "globals.hh"

namespace recbesim {
    SteppingAction::SteppingAction(const DetectorConstruction *detConstruction,
                                   EventAction *eventAction)
        : fDetConstruction(detConstruction), fEventAction(eventAction) {}

    void SteppingAction::UserSteppingAction(const G4Step *step) {}
} // namespace recbesim
