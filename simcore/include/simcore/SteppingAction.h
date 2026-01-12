#ifndef __simcore_SteppingAction_h__
#define __simcore_SteppingAction_h__

#include "G4UserSteppingAction.hh"

class G4Step;

namespace simcore {
    class DetectorConstruction;
    class EventAction;

    class SteppingAction : public G4UserSteppingAction {
      public:
        SteppingAction();
        ~SteppingAction() override = default;

        inline virtual void StepAction(const G4Step *step) {};

      private:
        void UserSteppingAction(const G4Step *step) final;
    };
} // namespace simcore
#endif
