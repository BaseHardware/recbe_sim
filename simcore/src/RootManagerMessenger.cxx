#include "simcore/RootManagerMessenger.h"
#include "simcore/RootManager.h"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"

namespace simcore {
    RootManagerMessenger::RootManagerMessenger(RootManager *rm) {
        if (rm == nullptr) {
            fRootManager = &RootManager::GetInstance();
        } else {
            fRootManager = rm;
        }

        fDirectory = new G4UIdirectory("/root/");
        fDirectory->SetGuidance("UI commands for the root file output.");

        fRecordPrimary = new G4UIcmdWithABool("/root/primary", this);
        fRecordPrimary->SetGuidance(
            "Select recording the primary information to the output ROOT file.");
        fRecordPrimary->SetParameterName("choice", false);
        fRecordPrimary->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fRecordStep = new G4UIcmdWithABool("/root/step", this);
        fRecordStep->SetGuidance("Select recording the step information to the output ROOT file.");
        fRecordStep->SetParameterName("choice", false);
        fRecordStep->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fOutFilename = new G4UIcmdWithAString("/root/filename", this);
        fOutFilename->SetGuidance("Select the filename for the output ROOT files.");
        fOutFilename->SetParameterName("filename", false);
        fOutFilename->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fMaxTrack = new G4UIcmdWithAnInteger("/root/maxTrack", this);
        fMaxTrack->SetGuidance("Set the maximum number of tracks in the output ROOT file.");
        fMaxTrack->SetParameterName("max", false);
        fMaxTrack->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);

        fMaxStep = new G4UIcmdWithAnInteger("/root/maxStep", this);
        fMaxStep->SetGuidance("Set the maximum number of steps in the output ROOT file.");
        fMaxStep->SetParameterName("max", false);
        fMaxStep->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);
    }

    RootManagerMessenger::~RootManagerMessenger() {
        delete fRecordPrimary;
        delete fDirectory;
    }

    void RootManagerMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
        if (command == fRecordPrimary) {
            G4bool boolValue = fRecordPrimary->GetNewBoolValue(newValue);
            fRootManager->RecordPrimary(boolValue);
            return;
        } else if (command == fRecordStep) {
            G4bool boolValue = fRecordPrimary->GetNewBoolValue(newValue);
            fRootManager->RecordPrimary(boolValue);
            return;
        } else if (command == fMaxTrack) {
            G4int intValue = fMaxTrack->GetNewIntValue(newValue);
            fRootManager->SetMaxTrackNum(intValue);
        } else if (command == fMaxStep) {
            G4int intValue = fMaxStep->GetNewIntValue(newValue);
            fRootManager->SetMaxStepNum(intValue);
        } else if (command == fOutFilename) {
            if (newValue.find(".root") == newValue.npos) {
                newValue = newValue + ".root";
            }
            fRootManager->SetFilename(newValue);
            return;
        }
    }
} // namespace simcore
