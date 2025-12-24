#include "EventAction.h"
#include "EventTrigger.h"
#include "RootManager.h"

#include "G4Event.hh"

namespace simcore {
    void EventAction::BeginOfEventAction(const G4Event *event) {
        RootManager::GetInstance().Clear();
        EventTrigger::GetInstance().Reset();
    }

    void EventAction::EndOfEventAction(const G4Event *event) {
        RootManager &instance = RootManager::GetInstance();
        instance.StorePrimary(event);
        if (EventTrigger::GetInstance().IsTriggered()) instance.Fill();
    }
} // namespace simcore
