#ifndef __bl10sim_BL10DetectorConstruction_h__
#define __bl10sim_BL10DetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;

namespace bl10sim {
    class BL10DetectorConstruction : public simcore::DetectorConstruction {
      public:
        BL10DetectorConstruction()           = default;
        ~BL10DetectorConstruction() override = default;

      public:
        void ConstructSDandField() override;

      protected:
        virtual void DefineMaterials() override;

      private:
        G4VPhysicalVolume *DefineVolumes() override;
    };
} // namespace bl10sim
#endif
