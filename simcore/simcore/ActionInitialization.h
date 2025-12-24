#ifndef __simcore_ActionInitialization_h__
#define __simcore_ActionInitialization_h__

#include "RootManager.h"

#include "G4VUserActionInitialization.hh"

namespace simcore {
    class ActionInitialization : public G4VUserActionInitialization {
      public:
        ActionInitialization() {
            // Force initialization of RootManager
            RootManager::GetInstance();
        };
        ~ActionInitialization() override = default;
    };
} // namespace simcore

#endif
