#ifndef __bl10sim_PrimaryGeneratorMessenger_h__
#define __bl10sim_PrimaryGeneratorMessenger_h__

#include "G4UImessenger.hh"

class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4UIcommand;

namespace bl10sim {
    class PrimaryGeneratorAction;
    class PrimaryGeneratorMessenger : public G4UImessenger {
      public:
        PrimaryGeneratorMessenger(PrimaryGeneratorAction *pa);
        ~PrimaryGeneratorMessenger() override;

        void SetNewValue(G4UIcommand *, G4String) override;

      private:
        PrimaryGeneratorAction *fPrimGenAction = nullptr;

        G4UIdirectory *fDirectory = nullptr;

        G4UIcmdWithADoubleAndUnit *fDuctLengthCmd     = nullptr;
        G4UIcmdWithADoubleAndUnit *fDuctEnterXSizeCmd = nullptr;
        G4UIcmdWithADoubleAndUnit *fDuctEnterYSizeCmd = nullptr;
        G4UIcmdWithAString *fFluxFilenameCmd          = nullptr;
    };
}; // namespace bl10sim
#endif
