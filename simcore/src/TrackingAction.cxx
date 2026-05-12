#include "simcore/TrackingAction.h"
#include "simcore/RootManager.h"

namespace simcore {
    void TrackingAction::PreUserTrackingAction(const G4Track *trk) {
        auto &rmInstance = RootManager::GetInstance();
        if (!rmInstance.CheckTrack(trk, true)) {
            G4cerr << "The number of tracks exceeds the limit. All the further tracks from this "
                      "event will not be recorded."
                   << G4endl;
            rmInstance.DisableTrack();
        }
        PreAction(trk);
    }

    void TrackingAction::PostUserTrackingAction(const G4Track *trk) {
        RootManager::GetInstance().CheckTrack(trk, false);
        PostAction(trk);
    }
} // namespace simcore
