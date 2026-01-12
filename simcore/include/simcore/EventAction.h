#ifndef __simcore_EventAction_h__
#define __simcore_EventAction_h__

#include "G4UserEventAction.hh"

class G4Event;

namespace simcore {
    class EventAction : public G4UserEventAction {
      public:
        EventAction()           = default;
        ~EventAction() override = default;

        inline virtual void BeginAction(const G4Event *event) {};
        inline virtual void EndAction(const G4Event *event) {};

      private:
        void BeginOfEventAction(const G4Event *event) final;
        void EndOfEventAction(const G4Event *event) final;
    };
} // namespace simcore
#endif
