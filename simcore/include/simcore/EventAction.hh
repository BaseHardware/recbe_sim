#ifndef simcore_EventAction_h
#define simcore_EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

namespace simcore {
    class EventAction : public G4UserEventAction {
      public:
        EventAction()           = default;
        ~EventAction() override = default;

        void BeginOfEventAction(const G4Event *event) override;
        void EndOfEventAction(const G4Event *event) override;

      private:
    };
} // namespace simcore
#endif
