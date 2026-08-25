#ifndef __bl10sim_MuelecDetectorConstruction_h__
#define __bl10sim_MuelecDetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4Region;
class G4GlobalMagFieldMessenger;

namespace bl10sim {
    class MuelecDetectorConstruction : public simcore::DetectorConstruction {
      public:
        MuelecDetectorConstruction();
        ~MuelecDetectorConstruction() override;

      protected:
        void DefineMaterials() override;

      private:
        G4Region *fTargetRegion;
        G4VPhysicalVolume *DefineVolumes() override;
    };
} // namespace bl10sim
#endif
