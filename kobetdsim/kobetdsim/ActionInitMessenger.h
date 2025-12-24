#ifndef __kobetdsim_ActionInitMessenger_h__
#define __kobetdsim_ActionInitMessenger_h__

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcommand;

namespace kobetdsim {
    class ActionInitialization;

    class ActionInitMessenger : public G4UImessenger {
      public:
        ActionInitMessenger(ActionInitialization *);
        ~ActionInitMessenger() override;

        void SetNewValue(G4UIcommand *, G4String) override;

      private:
        ActionInitialization *fActionInitialization = nullptr;

        G4UIdirectory *fDirectory        = nullptr;
        G4UIdirectory *fPrimaryDirectory = nullptr;

        G4UIcmdWithAString *fPrimaryMode  = nullptr;
        G4UIcmdWithAString *fPrimaryFName = nullptr;
    };
} // namespace kobetdsim
#endif
