#ifndef __bl10sim_ActionInitialization_h__
#define __bl10sim_ActionInitialization_h__

#include "G4VUserActionInitialization.hh"

namespace bl10sim {
    class DetectorConstruction;

    class ActionInitialization : public G4VUserActionInitialization {
      public:
        ActionInitialization();
        ~ActionInitialization() override = default;

        void BuildForMaster() const override;
        void Build() const override;

      private:
    };

} // namespace bl10sim
#endif
