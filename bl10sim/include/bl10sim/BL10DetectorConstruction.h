#ifndef __bl10sim_BL10DetectorConstruction_h__
#define __bl10sim_BL10DetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4ThreeVector.hh"
#include "G4TwoVector.hh"
#include "G4VUserDetectorConstruction.hh"

#include <vector>

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
        virtual void CalculateGeometrySubparameters();

        G4VSolid *BuildBoronResincaseSolid(G4bool) const;
        G4VSolid *BuildLabSolid(G4bool) const;

        G4LogicalVolume *BuildIroncase() const;
        G4LogicalVolume *FillIroncase(G4LogicalVolume *) const;

        G4LogicalVolume *BuildWorkbench() const;
        void PlaceWorkbench(G4LogicalVolume *lv) const;

        G4VPhysicalVolume *DefineVolumes() override;

      private:
        G4bool fSimpleGeometry;

        G4double ftLabWidthSlope;
        std::vector<G4TwoVector> fHBeamPoints;

        G4double fBeamYDistanceFromWall;

        G4double fBoronResinThickness;
        G4double fIronThickness;
        G4double fFloorThickness;

        G4double fLabHeight;
        G4double fLabZLength;
        G4double fLabWidthBeamside;
        G4double fLabWidthDumpside;
        G4double fLabFloorSpacing;

        G4double fExitwallDistance;
        G4double fExitwallThickness;
        G4double fExitwallWidth;
        G4double fExitwallBRDepth;

        G4double fExitpathWidth;

        G4double fWorkbenchPlateWidth;
        G4double fWorkbenchPlateLength;
        G4double fWorkbenchPlateThickness;

        G4double fWorkbenchPlateOuterMargin;
        G4double fWorkbenchPlateInnerMargin;

        G4double fWorkbenchSupportWidth;
        G4double fWorkbenchSupportHeight;
        G4double fWorkbenchSupportThickness;
        G4double fWorkbenchXSupportLength;
        G4double fWorkbenchZSupportLength;

        G4double fWBPipeEndplateThickness;
        G4double fWBPipeEndplate1Width;
        G4double fWBPipeEndplate2Width;
        G4double fWBPipeEndplate3Width;
        G4double fWBPipeEndplate1Length;
        G4double fWBPipeEndplate2Length;
        G4double fWBPipeEndplate3Length;

        G4double fWBVertPipeLength;
        G4double fWBDiagPipe1Length;
        G4double fWBDiagPipe2Length;
        G4double fWBVertPipeRadius;
        G4double fWBDiagPipe1Radius;
        G4double fWBDiagPipe2Radius;
        G4double fWBVertPipeThickness;
        G4double fWBDiagPipe1Thickness;
        G4double fWBDiagPipe2Thickness;

        G4double fWBLevelingBoltSize;

        G4double fWBZDistanceFromWall;

        G4ThreeVector fGlobalTlate;
    };
} // namespace bl10sim
#endif
