#ifndef __bl10sim_ActionInitialization_h__
#define __bl10sim_ActionInitialization_h__

#include "simcore/ActionInitialization.h"

namespace bl10sim {
    class DetectorConstruction;

    class ActionInitialization : public simcore::ActionInitialization {
      public:
        ActionInitialization();
        ~ActionInitialization() override = default;

        void BuildForMaster() const override;
        void Build() const override;

      private:
    };

} // namespace bl10sim
#endif
