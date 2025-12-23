#ifndef __simcore_SteppingAction_h__
#define __simcore_SteppingAction_h__

#include "G4UserSteppingAction.hh"

class G4Step;

namespace simcore {
    class DetectorConstruction;
    class EventAction;

    class SteppingAction : public G4UserSteppingAction {
      public:
        SteppingAction(const DetectorConstruction *detConstruction, EventAction *eventAction);
        ~SteppingAction() override = default;

        void UserSteppingAction(const G4Step *step) override;

      private:
        const DetectorConstruction *fDetConstruction = nullptr;
        EventAction *fEventAction                    = nullptr;
    };
} // namespace simcore
#endif
