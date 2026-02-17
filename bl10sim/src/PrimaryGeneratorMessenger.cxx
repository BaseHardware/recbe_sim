#include "bl10sim/PrimaryGeneratorMessenger.h"
#include "bl10sim/PrimaryGeneratorAction.h"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcommand.hh"

namespace bl10sim {
    PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction *pm)
        : fPrimGenAction(pm) {
        fDirectory = new G4UIdirectory("/prim_gen/");
        fDirectory->SetGuidance("UI commands for the PrimaryGeneratorAction class.");

        fDuctLengthCmd = new G4UIcmdWithADoubleAndUnit("/prim_gen/duct_length", this);
        fDuctLengthCmd->SetGuidance(
            "Set the length of ducts from the neutron moderator to the experimental room.");
        fDuctLengthCmd->SetParameterName("length", false);
        fDuctLengthCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fDuctEnterXSizeCmd = new G4UIcmdWithADoubleAndUnit("/prim_gen/duct_enter_x", this);
        fDuctEnterXSizeCmd->SetGuidance("Set the size of x-axis(width) of the duct enterance.");
        fDuctEnterXSizeCmd->SetParameterName("size", false);
        fDuctEnterXSizeCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fDuctEnterYSizeCmd = new G4UIcmdWithADoubleAndUnit("/prim_gen/duct_enter_y", this);
        fDuctEnterYSizeCmd->SetGuidance("Set the size of y-axis(height) of the duct enterance.");
        fDuctEnterYSizeCmd->SetParameterName("size", false);
        fDuctEnterYSizeCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fFluxFilenameCmd = new G4UIcmdWithAString("/prim_gen/flux_filename", this);
        fFluxFilenameCmd->SetGuidance("Set the filename for the neutron flux.");
        fFluxFilenameCmd->SetParameterName("filename (path)", false);
        fFluxFilenameCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);
    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {
        delete fDirectory;
        delete fDuctLengthCmd;
        delete fDuctEnterXSizeCmd;
        delete fDuctEnterYSizeCmd;
        delete fFluxFilenameCmd;
    }

    void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
        if (command == fDuctLengthCmd) {
            double length = fDuctLengthCmd->GetNewDoubleValue(newValue);
            fPrimGenAction->SetDuctLength(length);
        } else if (command == fDuctEnterXSizeCmd) {
            double length = fDuctEnterXSizeCmd->GetNewDoubleValue(newValue);
            fPrimGenAction->SetDuctEnteranceXSize(length);
        } else if (command == fDuctEnterYSizeCmd) {
            double length = fDuctEnterYSizeCmd->GetNewDoubleValue(newValue);
            fPrimGenAction->SetDuctEnteranceYSize(length);
        } else if (command == fFluxFilenameCmd) {
            fPrimGenAction->SetFluxFilename(newValue);
        }
    }
} // namespace bl10sim
