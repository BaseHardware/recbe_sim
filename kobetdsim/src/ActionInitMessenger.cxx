#include "ActionInitMessenger.h"
#include "ActionInitialization.h"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

namespace kobetdsim {
    ActionInitMessenger::ActionInitMessenger(ActionInitialization *det)
        : fActionInitialization(det) {
        fDirectory = new G4UIdirectory("/kobetdsim/");
        fDirectory->SetGuidance("UI commands specific to this simulation.");

        fPrimaryDirectory = new G4UIdirectory("/kobetdsim/primary/");
        fPrimaryDirectory->SetGuidance("UI commands for the primary vertex generation.");

        fPrimaryMode = new G4UIcmdWithAString("/kobetdsim/primary/mode", this);
        fPrimaryMode->SetGuidance("Select the mode for the primary generation.");
        fPrimaryMode->SetParameterName("mode", false);
        fPrimaryMode->AvailableForStates(G4State_PreInit);

        fPrimaryFName = new G4UIcmdWithAString("/kobetdsim/primary/filename", this);
        fPrimaryFName->SetGuidance("Select the filename for the primary input.");
        fPrimaryFName->SetParameterName("filename", false);
        fPrimaryFName->AvailableForStates(G4State_PreInit);
    }

    ActionInitMessenger::~ActionInitMessenger() { delete fDirectory; }

    void ActionInitMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
        if (command == fPrimaryMode) {
            fActionInitialization->SetPrimaryMode(newValue);
            return;
        } else if (command == fPrimaryFName) {
            fActionInitialization->SetPrimaryFilename(newValue);
            return;
        }
    }
} // namespace kobetdsim
