#ifndef __simcore_RootManagerMessenger_h__
#define __simcore_RootManagerMessenger_h__

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;
class G4UIcommand;

namespace simcore {
    class RootManager;

    class RootManagerMessenger : public G4UImessenger {
      public:
        RootManagerMessenger(RootManager * = nullptr);
        ~RootManagerMessenger() override;

        void SetNewValue(G4UIcommand *, G4String) override;

      private:
        RootManager *fRootManager = nullptr;

        G4UIdirectory *fDirectory = nullptr;

        G4UIcmdWithABool *fRecordStep    = nullptr;
        G4UIcmdWithABool *fRecordPrimary = nullptr;
        G4UIcmdWithAString *fOutFilename = nullptr;

        G4UIcmdWithAnInteger *fMaxTrack = nullptr;
        G4UIcmdWithAnInteger *fMaxStep  = nullptr;
    };
} // namespace simcore
#endif
