#ifndef __kobetdsim_aDetectorMessenger_h__
#define __kobetdsim_aDetectorMessenger_h__

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcommand;

namespace kobetdsim {
    class DetectorConstruction;

    class DetectorMessenger : public G4UImessenger {
      public:
        DetectorMessenger(DetectorConstruction *);
        ~DetectorMessenger() override;

        void SetNewValue(G4UIcommand *, G4String) override;

      private:
        DetectorConstruction *fDetectorConstruction = nullptr;

        G4UIdirectory *fDirectory    = nullptr;
        G4UIdirectory *fDetDirectory = nullptr;

        G4UIcmdWithABool *fUseBPE      = nullptr;
        G4UIcmdWithABool *fRealisticBP = nullptr;

        G4UIcmdWithAString *fGeomType = nullptr;
    };
} // namespace kobetdsim
#endif
