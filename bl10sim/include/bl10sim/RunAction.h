#ifndef __bl10sim_RunAction_h__
#define __bl10sim_RunAction_h__

#include "simcore/RunAction.h"

namespace bl10sim {
    class PrimaryGeneratorAction;

    class RunAction : public simcore::RunAction {
      public:
        RunAction(PrimaryGeneratorAction *pg);
        ~RunAction() override = default;

        virtual void BeginAction(const G4Run *event) override;

      private:
        PrimaryGeneratorAction* fPrimGenAction;
    };
} // namespace bl10sim

#endif
