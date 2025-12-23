#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

#include "G4Threading.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;

namespace recbesim {
    class DetectorConstruction : public G4VUserDetectorConstruction {
      public:
        DetectorConstruction()           = default;
        ~DetectorConstruction() override = default;

      public:
        G4VPhysicalVolume *Construct() override;
        void ConstructSDandField() override;

      private:
        // methods
        //
        void DefineMaterials();
        G4VPhysicalVolume *DefineVolumes();

        // data members
        //
        static G4ThreadLocal G4GlobalMagFieldMessenger *fMagFieldMessenger;
        // magnetic field messenger

        G4bool fCheckOverlaps = true; // option to activate checking of volumes overlaps
    };
} // namespace recbesim
#endif
