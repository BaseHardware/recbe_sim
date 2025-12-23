#ifndef __simcore_RunAction_h__
#define __simcore_RunAction_h__

#include "G4UserRunAction.hh"

class G4Run;

namespace simcore {
    class RunAction : public G4UserRunAction {
      public:
        RunAction();
        ~RunAction() override = default;

        void BeginOfRunAction(const G4Run *) override;
        void EndOfRunAction(const G4Run *) override;
    };
} // namespace simcore
#endif
