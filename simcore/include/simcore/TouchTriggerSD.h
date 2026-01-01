#ifndef __simcore_TouchTriggerSD_h__
#define __simcore_TouchTriggerSD_h__

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;
class G4ParticleDefinition;

namespace simcore {
    class TouchTriggerSD : public G4VSensitiveDetector {
      public:
        TouchTriggerSD(const G4String &name);
        ~TouchTriggerSD() override = default;

        void SetRequireNonzeroEdep(G4bool a = true) { fRequireEdep = a; }
        G4bool GetRequireNonzeroEdep() const { return fRequireEdep; }

        G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;
        void EndOfEvent(G4HCofThisEvent *hitCollection) override;

      private:
        G4bool fRequireEdep;
    };
} // namespace simcore
#endif
