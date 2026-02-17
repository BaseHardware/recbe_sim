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
        G4ThreeVector PlaceWorkbench(G4LogicalVolume *labLV, G4LogicalVolume *wbLV) const;

        void PlaceBeamWindow(G4LogicalVolume *labLV) const;

        void PlaceSamples(G4LogicalVolume *labLV, const G4ThreeVector sampleTlate) const;

        G4VPhysicalVolume *DefineVolumes() override;

      private:
        G4bool fSimpleGeometry;

        G4double ftLabWidthSlope;
        G4double ftWBEnvelopeWidth;
        G4double ftWBEnvelopeHeight;
        G4double ftWBEnvelopeZLength;
        std::vector<G4TwoVector> ftHBeamPoints;
        std::vector<G4TwoVector> ftLevelingBoltPoints;

        G4double fBeamXDistanceFromWall;
        G4double fBeamYDistanceFromFloor;

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
        G4double fWBZDiagPipeLength;
        G4double fWBXDiagPipeLength;
        G4double fWBVertPipeRadius;
        G4double fWBZDiagPipeRadius;
        G4double fWBXDiagPipeRadius;
        G4double fWBVertPipeThickness;
        G4double fWBZDiagPipeThickness;
        G4double fWBXDiagPipeThickness;

        G4double fWBZDiagPipeAngle;
        G4double fWBXDiagPipeAngle;
        G4double fWBZDiagPipeYDistance;
        G4double fWBXDiagPipeYDistance;

        G4double fWBLevelingBoltSize;
        G4double fWBLevelingBoltThickness;

        G4double fWBZDistanceFromWall;

        G4double fSampleZPosFromWBCenter;

        G4double fBeamWindowWidth;
        G4double fBeamWindowHeight;

        G4double fWindowThickness;
    };
} // namespace bl10sim
#endif
