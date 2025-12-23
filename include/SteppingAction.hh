#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"

class G4Step;

namespace B4 {
    class DetectorConstruction;
}

namespace B4a {
    class EventAction;

    class SteppingAction : public G4UserSteppingAction {
      public:
        SteppingAction(const B4::DetectorConstruction *detConstruction, EventAction *eventAction);
        ~SteppingAction() override = default;

        void UserSteppingAction(const G4Step *step) override;

      private:
        const B4::DetectorConstruction *fDetConstruction = nullptr;
        EventAction *fEventAction                        = nullptr;
    };

} // namespace B4a
#endif
