#include "ActionInitialization.h"

#include "EventAction.h"
#include "PrimaryGeneratorAction.h"
#include "RunAction.h"
#include "SteppingAction.h"

namespace simcore {
    ActionInitialization::ActionInitialization(DetectorConstruction *detConstruction)
        : fDetConstruction(detConstruction) {}

    void ActionInitialization::BuildForMaster() const { SetUserAction(new RunAction); }

    void ActionInitialization::Build() const {
        SetUserAction(new PrimaryGeneratorAction);
        SetUserAction(new RunAction);
        auto eventAction = new EventAction;
        SetUserAction(eventAction);
        SetUserAction(new SteppingAction(fDetConstruction, eventAction));
    }
} // namespace simcore
