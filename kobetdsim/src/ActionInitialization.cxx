#include "kobetdsim/ActionInitialization.h"
#include "kobetdsim/PrimaryGeneratorAction.h"

#include "simcore/EventAction.h"
#include "simcore/RunAction.h"
#include "simcore/TrackingAction.h"

namespace kobetdsim {
    void ActionInitialization::BuildForMaster() const {
        SetUserAction(new simcore::RunAction(true));
    }

    void ActionInitialization::Build() const {
        SetUserAction(new PrimaryGeneratorAction(fPrimaryMode, fPrimaryFilename));
        SetUserAction(new simcore::RunAction(false));
        SetUserAction(new simcore::TrackingAction());
        SetUserAction(new simcore::EventAction());
    }
} // namespace kobetdsim
