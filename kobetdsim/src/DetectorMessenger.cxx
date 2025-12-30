#include "kobetdsim/DetectorMessenger.h"
#include "kobetdsim/DetectorConstruction.h"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

namespace kobetdsim {
    DetectorMessenger::DetectorMessenger(DetectorConstruction *det) : fDetectorConstruction(det) {
        fDirectory = new G4UIdirectory("/kobetdsim/");
        fDirectory->SetGuidance("UI commands specific to this simulation.");

        fDetDirectory = new G4UIdirectory("/kobetdsim/det/");
        fDetDirectory->SetGuidance("Detector construction control");

        fUseBPE = new G4UIcmdWithABool("/kobetdsim/det/useBPE", this);
        fUseBPE->SetGuidance("Select use Borated PE for the shieldings");
        fUseBPE->SetParameterName("choice", false);
        fUseBPE->AvailableForStates(G4State_PreInit);

        fRealisticBP = new G4UIcmdWithABool("/kobetdsim/det/beampipe", this);
        fRealisticBP->SetGuidance("Select use the realistic beampipe geometry");
        fRealisticBP->SetParameterName("choice", false);
        fRealisticBP->AvailableForStates(G4State_PreInit);

        fGeomType = new G4UIcmdWithAString("/kobetdsim/det/geom_type", this);
        fUseBPE->SetGuidance("Select the type of geometries");
        fUseBPE->SetParameterName("type", false);
        fUseBPE->AvailableForStates(G4State_PreInit);
    }

    DetectorMessenger::~DetectorMessenger() {
        delete fUseBPE;
        delete fDetDirectory;
        delete fDirectory;
    }

    void DetectorMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
        if (command == fUseBPE) {
            G4bool boolValue = fUseBPE->GetNewBoolValue(newValue);
            fDetectorConstruction->SetBoratedPE(boolValue);
            return;
        } else if (command == fRealisticBP) {
            G4bool boolValue = fRealisticBP->GetNewBoolValue(newValue);
            fDetectorConstruction->SetRealisticBeampipe(boolValue);
            return;
        } else if (command == fGeomType) {
            fDetectorConstruction->SetGeometryType(newValue);
            return;
        }
    }
} // namespace kobetdsim
