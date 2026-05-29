#include "bl10sim/PrimTouchTriggerSD.h"
#include "simcore/EventTrigger.h"

#include "G4Step.hh"

namespace bl10sim {
    PrimTouchTriggerSD::PrimTouchTriggerSD(const G4String &name) : TouchTriggerSD(name) {}

    G4bool PrimTouchTriggerSD::ProcessHits(G4Step *step, G4TouchableHistory *h) {
        if (step->GetTrack()->GetTrackID() == 1) {
            return TouchTriggerSD::ProcessHits(step, h);
        }
        return false;
    }
} // namespace bl10sim
