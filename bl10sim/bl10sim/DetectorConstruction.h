#ifndef __bl10sim_DetectorConstruction_h__
#define __bl10sim_DetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4VUserDetectorConstruction.hh"

#include "G4Threading.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;

namespace bl10sim {
    class DetectorConstruction : public simcore::DetectorConstruction {
      public:
        DetectorConstruction()           = default;
        ~DetectorConstruction() override = default;

      public:
        void ConstructSDandField() override;

      private:
        G4VPhysicalVolume *DefineVolumes() override;
    };
} // namespace bl10sim
#endif
