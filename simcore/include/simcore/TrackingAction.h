#ifndef __simcore_TrackingAction_h__
#define __simcore_TrackingAction_h__

#include "G4UserTrackingAction.hh"

class G4Track;

namespace simcore {

    class TrackingAction : public G4UserTrackingAction {
      public:
        TrackingAction() {};
        ~TrackingAction() override = default;

        inline virtual void PreAction(const G4Track *) {};
        inline virtual void PostAction(const G4Track *) {};

      private:
        void PreUserTrackingAction(const G4Track *trk) final;
        void PostUserTrackingAction(const G4Track *trk) final;
    };
}; // namespace simcore

#endif
