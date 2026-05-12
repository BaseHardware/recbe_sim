#include "simcore/SteppingAction.h"
#include "simcore/RootManager.h"

#include "G4Step.hh"

namespace simcore {
    SteppingAction::SteppingAction() {}

    void SteppingAction::UserSteppingAction(const G4Step *step) {
        const G4Track *track = step->GetTrack();

        if (track->GetTrackStatus() != fStopAndKill &&
            track->GetTrackStatus() != fKillTrackAndSecondaries) {
            auto &rmInstance = RootManager::GetInstance();
            if (!rmInstance.AppendStep(step)) {
                G4cerr << "The number of steps exceeds the limit. All the further steps from this "
                          "event will not be recorded."
                       << G4endl;
                rmInstance.DisableStep();
            }
        }

        StepAction(step);
    }
} // namespace simcore
