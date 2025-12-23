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

        // get methods
        //
        const G4VPhysicalVolume *GetAbsorberPV() const;
        const G4VPhysicalVolume *GetGapPV() const;

      private:
        // methods
        //
        void DefineMaterials();
        G4VPhysicalVolume *DefineVolumes();

        // data members
        //
        static G4ThreadLocal G4GlobalMagFieldMessenger *fMagFieldMessenger;
        // magnetic field messenger

        G4VPhysicalVolume *fAbsorberPV = nullptr; // the absorber physical volume
        G4VPhysicalVolume *fGapPV      = nullptr; // the gap physical volume

        G4bool fCheckOverlaps = true; // option to activate checking of volumes overlaps
    };

    // inline functions

    inline const G4VPhysicalVolume *DetectorConstruction::GetAbsorberPV() const {
        return fAbsorberPV;
    }

    inline const G4VPhysicalVolume *DetectorConstruction::GetGapPV() const { return fGapPV; }
} // namespace recbesim
#endif
