#ifndef __simcore_RunAction_h__
#define __simcore_RunAction_h__

#include "G4UserRunAction.hh"

class G4Run;

namespace simcore {
    class RunAction : public G4UserRunAction {
      public:
        RunAction();
        ~RunAction() override = default;

        inline virtual void BeginAction(const G4Run *event) {};
        inline virtual void EndAction(const G4Run *event) {};

      private:
        void BeginOfRunAction(const G4Run *) final;
        void EndOfRunAction(const G4Run *) final;
    };
} // namespace simcore
#endif
