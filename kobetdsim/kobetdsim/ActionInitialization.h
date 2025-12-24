#ifndef __kobetdsim_ActionInitialization_h__
#define __kobetdsim_ActionInitialization_h__

#include "ActionInitMessenger.h"

#include "G4String.hh"
#include "G4VUserActionInitialization.hh"

namespace kobetdsim {
    class ActionInitialization : public G4VUserActionInitialization {
      public:
        ActionInitialization()
            : fPrimaryMode("neutron"), fPrimaryFilename("primary.dat"),
              fOutputFilename("test.root") {
            fMessenger = new ActionInitMessenger(this);
        };
        ~ActionInitialization() override { delete fMessenger; };

        void SetOutputFilename(G4String a) { fOutputFilename = a; }
        G4String GetOutputFilename() const { return fOutputFilename; }

        void SetPrimaryMode(G4String a) { fPrimaryMode = a; }
        G4String GetPrimaryMode() const { return fPrimaryMode; }

        void SetPrimaryFilename(G4String a) { fPrimaryFilename = a; }
        G4String GetPrimaryFilename() const { return fPrimaryFilename; }

        void BuildForMaster() const override;
        void Build() const override;

      private:
        ActionInitMessenger *fMessenger;

        G4String fPrimaryMode;
        G4String fPrimaryFilename;
        G4String fOutputFilename;
    };
} // namespace kobetdsim
#endif
