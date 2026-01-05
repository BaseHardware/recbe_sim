#include "simcore/TrackingAction.h"
#include "simcore/RootManager.h"

namespace simcore {
    void TrackingAction::PreUserTrackingAction(const G4Track *trk) {
        RootManager::GetInstance().CheckTrack(trk, true);
    }
    void TrackingAction::PostUserTrackingAction(const G4Track *trk) {
        RootManager::GetInstance().CheckTrack(trk, false);
    }
} // namespace simcore
