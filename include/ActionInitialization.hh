#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

namespace recbesim {
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

} // namespace recbesim
#endif
