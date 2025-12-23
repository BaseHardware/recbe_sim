#include "ActionInitialization.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

using namespace B4;

namespace B4a {
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
} // namespace B4a
