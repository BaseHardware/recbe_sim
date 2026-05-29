#ifndef __bl10sim_PrimTouchTriggerSD_h__
#define __bl10sim_PrimTouchTriggerSD_h__

#include "simcore/TouchTriggerSD.h"

class G4Step;
class G4HCofThisEvent;
class G4ParticleDefinition;

namespace bl10sim {
    class PrimTouchTriggerSD : public simcore::TouchTriggerSD {
      public:
        PrimTouchTriggerSD(const G4String &name);
        ~PrimTouchTriggerSD() override = default;

        G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;

      private:
        G4bool fRequireEdep;
    };
} // namespace bl10sim
#endif
