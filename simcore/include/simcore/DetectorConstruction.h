#ifndef __simcore_DetectorConstruction_h__
#define __simcore_DetectorConstruction_h__

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;

namespace simcore {
    class DetectorConstruction : public G4VUserDetectorConstruction {
      public:
        DetectorConstruction()           = default;
        ~DetectorConstruction() override = default;

        G4VPhysicalVolume *Construct() override;
        void ConstructSDandField() override;

      protected:
        virtual void DefineMaterials();

      private:
        virtual G4VPhysicalVolume *DefineVolumes() = 0;

        static G4ThreadLocal G4GlobalMagFieldMessenger *fMagFieldMessenger;
        // magnetic field messenger

      protected:
        G4bool fCheckOverlaps = true; // option to activate checking of volumes overlaps
    };
} // namespace simcore
#endif
