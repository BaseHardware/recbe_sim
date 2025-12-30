#include "simcore/TrackingAction.h"
#include "simcore/RootManager.h"

namespace simcore {
    void TrackingAction::PreUserTrackingAction(const G4Track *trk) {
        RootManager &instance = RootManager::GetInstance();

        instance.StartTrack(trk);
    }
    void TrackingAction::PostUserTrackingAction(const G4Track *trk) {}
} // namespace simcore
