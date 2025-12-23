#ifndef __simcore_TrackingAction_h__
#define __simcore_TrackingAction_h__

#include "G4UserTrackingAction.hh"

class G4Track;

namespace simcore {

    class TrackingAction : public G4UserTrackingAction {
      public:
        TrackingAction() {};
        ~TrackingAction() override = default;

        void PreUserTrackingAction(const G4Track *trk) override;
        void PostUserTrackingAction(const G4Track *trk) override;
    };
}; // namespace simcore

#endif
