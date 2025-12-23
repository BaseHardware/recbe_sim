#include "ActionInitialization.h"

#include "EventAction.h"
#include "PrimaryGeneratorAction.h"
#include "RunAction.h"
#include "SteppingAction.h"

namespace simcore {
    ActionInitialization::ActionInitialization() {}

    void ActionInitialization::BuildForMaster() const { SetUserAction(new RunAction(true)); }

    void ActionInitialization::Build() const {
        SetUserAction(new PrimaryGeneratorAction);
        SetUserAction(new RunAction(false));
        SetUserAction(new EventAction);
        SetUserAction(new SteppingAction);
    }
} // namespace simcore
