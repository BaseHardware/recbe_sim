#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"

class G4Run;

namespace B4 {
    class RunAction : public G4UserRunAction {
      public:
        RunAction();
        ~RunAction() override = default;

        void BeginOfRunAction(const G4Run *) override;
        void EndOfRunAction(const G4Run *) override;
    };

} // namespace B4
#endif
