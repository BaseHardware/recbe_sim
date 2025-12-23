#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "globals.hh"

namespace recbesim {
    SteppingAction::SteppingAction(const DetectorConstruction *detConstruction,
                                   EventAction *eventAction)
        : fDetConstruction(detConstruction), fEventAction(eventAction) {}

    void SteppingAction::UserSteppingAction(const G4Step *step) {
        // Collect energy and track length step by step

        // get volume of the current step
        auto volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();

        // energy deposit
        auto edep = step->GetTotalEnergyDeposit();

        // step length
        G4double stepLength = 0.;
        if (step->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) {
            stepLength = step->GetStepLength();
        }

        if (volume == fDetConstruction->GetAbsorberPV()) {
            fEventAction->AddAbs(edep, stepLength);
        }

        if (volume == fDetConstruction->GetGapPV()) {
            fEventAction->AddGap(edep, stepLength);
        }
    }
} // namespace recbesim
