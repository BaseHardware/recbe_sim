#include "EventAction.h"
#include "EventTrigger.h"
#include "RootManager.h"

#include "G4Event.hh"

namespace simcore {
    void EventAction::BeginOfEventAction(const G4Event *event) {}

    void EventAction::EndOfEventAction(const G4Event *event) {
        RootManager &instance = RootManager::GetInstance();
        EventTrigger::GetInstance().Trigger();
        instance.Fill();
    }
} // namespace simcore
