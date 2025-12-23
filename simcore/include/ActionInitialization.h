#ifndef __simcore_ActionInitialization_h__
#define __simcore_ActionInitialization_h__

#include "G4VUserActionInitialization.hh"

namespace simcore {
    class DetectorConstruction;

    /// Action initialization class.

    class ActionInitialization : public G4VUserActionInitialization {
      public:
        ActionInitialization(DetectorConstruction *);
        ~ActionInitialization() override = default;

        void BuildForMaster() const override;
        void Build() const override;

      private:
        DetectorConstruction *fDetConstruction = nullptr;
    };

} // namespace simcore
#endif
