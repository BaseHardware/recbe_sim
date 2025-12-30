#ifndef __bl10sim_DuctDetectorConstruction_h__
#define __bl10sim_DuctDetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;

namespace bl10sim {
    class DuctDetectorConstruction : public simcore::DetectorConstruction {
      public:
        DuctDetectorConstruction()           = default;
        ~DuctDetectorConstruction() override = default;

      public:
        void ConstructSDandField() override;

      private:
        G4VPhysicalVolume *DefineVolumes() override;
    };
} // namespace bl10sim
#endif
