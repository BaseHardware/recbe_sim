#ifndef __bl10sim_BL10DetectorConstruction_h__
#define __bl10sim_BL10DetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4ThreeVector.hh"
#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;
class G4VSolid;

namespace bl10sim {
    class BL10DetectorConstruction : public simcore::DetectorConstruction {
      public:
        BL10DetectorConstruction();
        ~BL10DetectorConstruction() override = default;

      public:
        void ConstructSDandField() override;

      protected:
        virtual void DefineMaterials() override;

        virtual void SetGeometryParameters();

      private:
        G4LogicalVolume *BuildIroncase(G4bool floor) const;
        void FillIroncase(G4LogicalVolume *) const;

        G4VSolid* BuildBoronResincaseSolid(G4bool) const;
        G4VSolid* BuildLabSolid(G4bool) const;

        G4VPhysicalVolume *DefineVolumes() override;

        G4bool fSimpleGeometry;

        G4double fBoronResinThickness;
        G4double fIronThickness;
        G4double fFloorThickness;

        G4double fLabHeight;
        G4double fLabZLength;
        G4double fLabWidthBeamside;
        G4double fLabWidthDumpside;

        G4double fExitwallDistance;
        G4double fExitwallThickness;
        G4double fExitwallWidth;
        G4double fExitwallBRDepth;

        G4ThreeVector fGlobalTlate;
    };
} // namespace bl10sim
#endif
