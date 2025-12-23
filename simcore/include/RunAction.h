#ifndef __simcore_RunAction_h__
#define __simcore_RunAction_h__

#include "G4UserRunAction.hh"

class G4Run;

namespace simcore {
    class RunAction : public G4UserRunAction {
      public:
        RunAction(bool m);
        ~RunAction() override = default;

        void BeginOfRunAction(const G4Run *) override;
        void EndOfRunAction(const G4Run *) override;

        bool IsMasterInstance() const { return fMaster; }

      private:
        bool fMaster;
    };
} // namespace simcore
#endif
