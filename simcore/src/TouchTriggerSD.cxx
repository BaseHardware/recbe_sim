#include "simcore/TouchTriggerSD.h"
#include "simcore/EventTrigger.h"

#include "G4Step.hh"

namespace simcore {
    TouchTriggerSD::TouchTriggerSD(const G4String &name)
        : G4VSensitiveDetector(name), fRequireEdep(true) {}

    G4bool TouchTriggerSD::ProcessHits(G4Step *step, G4TouchableHistory *) {
        if (step->IsFirstStepInVolume()) {
            if (!fRequireEdep) {
                simcore::EventTrigger::GetInstance().Trigger();
            } else if (fRequireEdep && step->GetTotalEnergyDeposit() != 0) {
                simcore::EventTrigger::GetInstance().Trigger();
            }
            return true;
        }
        return false;
    }

    void TouchTriggerSD::EndOfEvent(G4HCofThisEvent *) {}
} // namespace simcore
