#include <iostream>

#include "bl10sim/PrimaryGeneratorAction.h"
#include "bl10sim/PrimaryGeneratorMessenger.h"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcommand.hh"

using namespace std;

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
    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {
        delete fDirectory;
        delete fDuctLengthCmd;
        delete fDuctEnterXSizeCmd;
        delete fDuctEnterYSizeCmd;
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
        }
    }
} // namespace bl10sim
