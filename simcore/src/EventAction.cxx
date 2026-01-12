#include "simcore/EventAction.h"
#include "simcore/EventTrigger.h"
#include "simcore/RootManager.h"
#include "simcore/SafeTermination.h"

#include "G4Event.hh"
#include "G4RunManager.hh"

namespace simcore {
    void EventAction::BeginOfEventAction(const G4Event *event) {
        RootManager::GetInstance().Clear();
        EventTrigger::GetInstance().Reset();
    }

    void EventAction::EndOfEventAction(const G4Event *event) {
        RootManager &instance = RootManager::GetInstance();
        instance.StorePrimary(event);
        if (EventTrigger::GetInstance().IsTriggered()) instance.Fill();

        if (SafeTermination::GetInstance().IsTerminated()) {
            G4RunManager::GetRunManager()->AbortRun(true);
        }
    }
} // namespace simcore
