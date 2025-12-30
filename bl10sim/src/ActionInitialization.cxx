#include "bl10sim/ActionInitialization.h"
#include "bl10sim/PrimaryGeneratorAction.h"

#include "simcore/EventAction.h"
#include "simcore/RunAction.h"
#include "simcore/SteppingAction.h"
#include "simcore/TrackingAction.h"


namespace bl10sim {
    ActionInitialization::ActionInitialization() {}

    void ActionInitialization::BuildForMaster() const {
        SetUserAction(new simcore::RunAction(true));
    }

    void ActionInitialization::Build() const {
        SetUserAction(new PrimaryGeneratorAction);
        SetUserAction(new simcore::RunAction(false));
        SetUserAction(new simcore::EventAction);
        SetUserAction(new simcore::TrackingAction);
        SetUserAction(new simcore::SteppingAction);
    }
} // namespace bl10sim
