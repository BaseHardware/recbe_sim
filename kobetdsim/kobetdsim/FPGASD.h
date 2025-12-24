#ifndef __kobetdsim_FPGASD_h__
#define __kobetdsim_FPGASD_h__

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;
class G4ParticleDefinition;

namespace kobetdsim {
    class FPGASD : public G4VSensitiveDetector {
      public:
        FPGASD(const G4String &name);
        ~FPGASD() override = default;

        G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;
        void EndOfEvent(G4HCofThisEvent *hitCollection) override;
    };
} // namespace kobetdsim
#endif
