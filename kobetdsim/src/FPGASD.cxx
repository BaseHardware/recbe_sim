#include "FPGASD.h"

#include "simcore/EventTrigger.h"

#include "G4Step.hh"

namespace kobetdsim {
    FPGASD::FPGASD(const G4String &name) : G4VSensitiveDetector(name) {}

    G4bool FPGASD::ProcessHits(G4Step *step, G4TouchableHistory *) {
        if (step->GetTotalEnergyDeposit() != 0) {
            simcore::EventTrigger::GetInstance().Trigger();
        }

        return true;
    }

    void FPGASD::EndOfEvent(G4HCofThisEvent *) {}
} // namespace kobetdsim
