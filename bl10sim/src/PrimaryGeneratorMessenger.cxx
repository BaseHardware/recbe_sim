#include "bl10sim/PrimaryGeneratorMessenger.h"
#include "bl10sim/PrimaryGeneratorAction.h"

#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
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

        fEnableCmd = new G4UIcmdWithABool("/prim_gen/enable", this);
        fEnableCmd->SetGuidance("Set the enable for this primary generator. If disable this, the "
                                "fixed vertex will be generated.");
        fEnableCmd->SetParameterName("enable", true, false);
        fEnableCmd->SetDefaultValue(true);
        fFluxFilenameCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fFBFraction = new G4UIcmdWithADouble("/prim_gen/first_fraction", this);
        fFBFraction->SetGuidance("Set the fraction of the first bunch.");
        fFBFraction->SetParameterName("fraction", false);
        fFBFraction->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fTimeOffsetCmd = new G4UIcmdWithADoubleAndUnit("/prim_gen/t_offset", this);
        fTimeOffsetCmd->SetGuidance("Set the time offset for the neutrons.");
        fTimeOffsetCmd->SetParameterName("offset", false);
        fTimeOffsetCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fBSeparation = new G4UIcmdWithADoubleAndUnit("/prim_gen/bunch_separation", this);
        fBSeparation->SetGuidance("Set the time separation for the first and second bunches.");
        fBSeparation->SetParameterName("offset", false);
        fBSeparation->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fFBOffset = new G4UIcmdWithADoubleAndUnit("/prim_gen/first_offset", this);
        fFBOffset->SetGuidance("Set the time offset for the first bunch.");
        fFBOffset->SetParameterName("offset", false);
        fFBOffset->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fFBFWHM = new G4UIcmdWithADoubleAndUnit("/prim_gen/first_fwhm", this);
        fFBFWHM->SetGuidance("Set the full width half maximum for the first bunch.");
        fFBFWHM->SetParameterName("fwhm", false);
        fFBFWHM->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fSBFWHM = new G4UIcmdWithADoubleAndUnit("/prim_gen/second_fwhm", this);
        fSBFWHM->SetGuidance("Set the full width half maximum for the second bunch.");
        fSBFWHM->SetParameterName("fwhm", false);
        fSBFWHM->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);
    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {
        delete fDirectory;
        delete fDuctLengthCmd;
        delete fDuctEnterXSizeCmd;
        delete fDuctEnterYSizeCmd;
        delete fFluxFilenameCmd;

        delete fFBFraction;

        delete fTimeOffsetCmd;
        delete fBSeparation;
        delete fFBOffset;
        delete fFBFWHM;
        delete fSBFWHM;
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
        } else if (command == fEnableCmd) {
            bool enable = fEnableCmd->GetNewBoolValue(newValue);
            fPrimGenAction->Enable(enable);
        } else if (command == fTimeOffsetCmd) {
            double value = fTimeOffsetCmd->GetNewDoubleValue(newValue);
            fPrimGenAction->SetTimeOffset(value / us);
        } else if (command == fFBOffset) {
            double value = fFBOffset->GetNewDoubleValue(newValue);
            fPrimGenAction->SetFirstBunchOffset(value / us);
        } else if (command == fBSeparation) {
            double value = fBSeparation->GetNewDoubleValue(newValue);
            fPrimGenAction->SetBunchSeparation(value / us);
        } else if (command == fFBFraction) {
            double value = fFBFraction->GetNewDoubleValue(newValue);
            fPrimGenAction->SetFirstBunchFraction(value);
        } else if (command == fFBFWHM) {
            double value = fFBFWHM->GetNewDoubleValue(newValue);
            fPrimGenAction->SetFirstBunchFWHM(value / us);
        } else if (command == fSBFWHM) {
            double value = fSBFWHM->GetNewDoubleValue(newValue);
            fPrimGenAction->SetSecondBunchFWHM(value / us);
        }
    }
} // namespace bl10sim
