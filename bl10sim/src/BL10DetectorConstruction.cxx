#include "bl10sim/BL10DetectorConstruction.h"

#include "simcore/MetadataManager.h"

#include "G4Box.hh"
#include "G4DisplacedSolid.hh"
#include "G4ExtrudedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"

#include <vector>

static const G4double booleanSolidTolerance = 200 * um;

namespace bl10sim {
    BL10DetectorConstruction::BL10DetectorConstruction() : fSimpleGeometry(false) {
        SetGeometryParameters();
        CalculateGeometrySubparameters();
    }

    void BL10DetectorConstruction::DefineMaterials() {
        DetectorConstruction::DefineMaterials();
        G4NistManager::Instance()->FindOrBuildMaterial("G4_Fe");
        G4NistManager::Instance()->FindOrBuildMaterial("G4_BORON_CARBIDE");

        // B4C material definition from the PHITS code for BL10.
        // Currently, commented out as we cannot believe this value
        // G4Element *atomB10 = new G4Element("Boron10", "B", 5, 10);
        // G4Element *atomB11 = new G4Element("Boron11", "B", 5, 11);
        // G4Element *atomC12 = new G4Element("Carbon", "C", 6, 12);

        // G4Material *b4c_normal =
        //     new G4Material("B4C_NORMAL", 2.5 * g / cm3, 3, G4State::kStateSolid);
        // b4c_normal->AddElementByMassFraction(atomB10, 4.336e-3);
        // b4c_normal->AddElementByMassFraction(atomB11, 1.745e-2);
        // b4c_normal->AddElementByMassFraction(atomC12, 5.447e-3);

        // G4Material *b4c_10b_97p =
        //     new G4Material("B4C_10B_91%", 2.5 * g / cm3, 3, G4State::kStateSolid);
        // b4c_10b_97p->AddElementByMassFraction(atomB10, 2.113e-2);
        // b4c_10b_97p->AddElementByMassFraction(atomB11, 6.536e-4);
        // b4c_10b_97p->AddElementByMassFraction(atomC12, 5.447e-3);
    }

    void BL10DetectorConstruction::SetGeometryParameters() {
        fBeamXDistanceFromWall = 75 * cm;

        fBoronResinThickness = 20 * cm;
        fIronThickness       = 10 * cm;
        fFloorThickness      = 10 * cm;

        fLabHeight        = 3.0 * m;
        fLabZLength       = 3.5 * m;
        fLabWidthBeamside = 1.9 * m;
        fLabWidthDumpside = 3.1 * m;
        fLabFloorSpacing  = 25 * cm;

        fExitwallDistance  = 60 * cm;
        fExitwallThickness = 50 * cm;
        fExitwallWidth     = 1 * m;
        fExitwallBRDepth   = 10 * cm;

        fExitpathWidth = 60 * cm;

        fWorkbenchPlateWidth     = 1000 * mm;
        fWorkbenchPlateLength    = 2000 * mm;
        fWorkbenchPlateThickness = 30 * mm;

        fWorkbenchPlateOuterMargin = 60 * mm;
        fWorkbenchPlateInnerMargin = 25 * mm;

        fWorkbenchSupportWidth     = 100 * mm;
        fWorkbenchSupportHeight    = 100 * mm;
        fWorkbenchSupportThickness = 10 * mm;
        fWorkbenchXSupportLength   = 850 * mm;
        fWorkbenchZSupportLength   = 2050 * mm;

        fWBPipeEndplateThickness = 20 * mm;
        fWBPipeEndplate1Width    = 220 * mm;
        fWBPipeEndplate2Width    = 220 * mm;
        fWBPipeEndplate3Width    = 115 * mm;
        fWBPipeEndplate1Length   = 220 * mm;
        fWBPipeEndplate2Length   = 320 * mm;
        fWBPipeEndplate3Length   = 155 * mm;

        fWBVertPipeLength     = 900 * mm;
        fWBZDiagPipeLength    = 950 * mm;
        fWBXDiagPipeLength    = 630 * mm;
        fWBVertPipeRadius     = 115 * mm / 2.;
        fWBZDiagPipeRadius    = 65 * mm / 2.;
        fWBXDiagPipeRadius    = 65 * mm / 2.;
        fWBVertPipeThickness  = 4.5 * mm;
        fWBZDiagPipeThickness = 3 * mm;
        fWBXDiagPipeThickness = 3 * mm;

        fWBZDiagPipeAngle     = 45 * deg;
        fWBXDiagPipeAngle     = 30 * deg;
        fWBZDiagPipeYDistance = 55 * mm;
        fWBXDiagPipeYDistance = 190 * mm;

        fWBLevelingBoltSize      = 60 * mm;
        fWBLevelingBoltThickness = 30 * mm;

        fWBZDistanceFromWall = 10 * cm;

        fSampleZPosFromWBCenter = 300 * mm;
    }

    void BL10DetectorConstruction::CalculateGeometrySubparameters() {
        ftLabWidthSlope = (fLabWidthDumpside - fLabWidthBeamside) / fLabZLength;

        ftWBEnvelopeWidth =
            fWorkbenchXSupportLength + fWorkbenchSupportWidth * 2 + fWorkbenchPlateOuterMargin * 2;
        ftWBEnvelopeHeight = fWBPipeEndplateThickness + fWBVertPipeLength +
                             fWBPipeEndplateThickness + fWorkbenchSupportHeight +
                             fWorkbenchPlateThickness;
        ftWBEnvelopeZLength = fWorkbenchPlateOuterMargin * 2 + fWorkbenchZSupportLength;

        G4TwoVector nowHBeamPoint;

        G4double hbXSize     = fWorkbenchSupportWidth;
        G4double hbYSize     = fWorkbenchSupportHeight;
        G4double hbThickness = fWorkbenchSupportThickness;

        fHBeamPoints.clear();

        nowHBeamPoint = {-hbThickness / 2., -hbYSize / 2. + hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbThickness / 2. - hbXSize / 2., 0};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, -hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbXSize, 0};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbXSize / 2. + hbThickness / 2., 0};
        fHBeamPoints.push_back(nowHBeamPoint);

        nowHBeamPoint = {hbThickness / 2., hbYSize / 2. - hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbThickness / 2. + hbXSize / 2., 0};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbXSize, 0};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, -hbThickness};
        fHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbXSize / 2. - hbThickness / 2., 0};
        fHBeamPoints.push_back(nowHBeamPoint);

        fLevelingBoltPoints.clear();

        G4TwoVector nowLBPoint = {0, fWBLevelingBoltSize / 2.};
        fLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        fLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        fLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        fLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        fLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        fLevelingBoltPoints.push_back(nowLBPoint);
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildIroncase() const {
        // Making a logical volume for the world (Iron case)
        G4Material *matIron = G4Material::GetMaterial("G4_Fe");

        G4double worldZLength = fLabZLength + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldHeight = fLabHeight + fBoronResinThickness + fIronThickness + fFloorThickness;

        // Calculating the width of the iron case at the beam/dump side
        // (assuming beamside < dumpside)
        G4double worldWidthAtLabBeamBoundary =
            fLabWidthBeamside + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldWidthAtLabDumpBoundary =
            fLabWidthDumpside + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldWidthBeamside =
            worldWidthAtLabBeamBoundary - ftLabWidthSlope * (fBoronResinThickness + fIronThickness);
        G4double worldWidthDumpside =
            worldWidthAtLabDumpBoundary + ftLabWidthSlope * (fBoronResinThickness + fIronThickness);

        G4Trd *worldTrd = new G4Trd("IroncaseTrd", worldWidthBeamside / 2., worldWidthDumpside / 2.,
                                    worldHeight / 2., worldHeight / 2., worldZLength / 2.);
        G4LogicalVolume *worldLV = new G4LogicalVolume(worldTrd, matIron, "IroncaseLV");

        G4Material *matConcrete = G4Material::GetMaterial("G4_CONCRETE");
        G4Trd *floorTrd = new G4Trd("FloorTrd", worldWidthBeamside / 2., worldWidthDumpside / 2.,
                                    fFloorThickness / 2., fFloorThickness / 2., worldZLength / 2.);
        G4LogicalVolume *floorLV = new G4LogicalVolume(floorTrd, matConcrete, "FloorLV");

        G4ThreeVector floorTlate = {0, -worldHeight / 2. + fFloorThickness / 2., 0};
        new G4PVPlacement(nullptr, floorTlate, floorLV, "FloorPV", worldLV, false, 0,
                          fCheckOverlaps);

        return worldLV;
    }

    G4VSolid *BL10DetectorConstruction::BuildBoronResincaseSolid(G4bool simple) const {
        G4double boronResinZLength = fLabZLength + 2 * fBoronResinThickness + fIronThickness;
        G4double boronResinHeight  = fLabHeight + fBoronResinThickness;

        // Calculating the width of the boron-resin case at the beam/dump side
        // (assuming beamside < dumpside)
        G4double boronResinWidthAtLabBeamBoundary = fLabWidthBeamside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtLabDumpBoundary = fLabWidthDumpside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtOrigBeamBoundary =
            boronResinWidthAtLabBeamBoundary - ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthAtOrigDumpBoundary =
            boronResinWidthAtLabDumpBoundary + ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthBeamside = boronResinWidthAtOrigBeamBoundary;
        G4double boronResinWidthDumpside =
            boronResinWidthAtOrigDumpBoundary + ftLabWidthSlope * fIronThickness;

        // Build the Boron-Resin case (simple version)
        G4Trd *boronResinTrd = new G4Trd("BoronResinCaseTrd", boronResinWidthBeamside / 2.,
                                         boronResinWidthDumpside / 2., boronResinHeight / 2.,
                                         boronResinHeight / 2., boronResinZLength / 2.);

        if (simple) return boronResinTrd;

        G4double ewCarverBoxWidth =
            fExitwallWidth - fExitwallBRDepth + fBoronResinThickness + booleanSolidTolerance * 2;
        G4double ewCarverBoxHeight    = fLabHeight + booleanSolidTolerance;
        G4double ewCarverBoxThickness = fExitwallThickness - 2 * fExitwallBRDepth;

        G4Box *exitwallCarverBox =
            new G4Box("BoronResinCaseExitwallCarverBox", ewCarverBoxWidth / 2.,
                      ewCarverBoxHeight / 2., ewCarverBoxThickness / 2.);

        // Tlanslation for the carver
        G4ThreeVector ewCarverTlate = {0, 0, 0};
        // Apply a bias of the center of G4Trd
        // -z length: fLabZLength/2+fBoronResinThickness
        // +z length: fLabZLength/2+fBoronResinThickness + fIronThickness
        ewCarverTlate += {0, 0, fIronThickness / 2.};
        // Move the carver center to the +z end
        ewCarverTlate += {0, 0, boronResinZLength / 2.};
        // Compenstae the thickness of the iron
        ewCarverTlate += {0, 0, -fIronThickness};
        // Move the center to the +z edge of exitwall iron shield (inside the boron-resin shield)
        ewCarverTlate += {0, 0, -fExitwallDistance - fExitwallBRDepth - fBoronResinThickness};
        // Move the center to the x-border of exitwall
        ewCarverTlate += {-boronResinWidthDumpside / 2. +
                              ftLabWidthSlope / 2. *
                                  (fExitwallDistance + fExitwallBRDepth + fBoronResinThickness),
                          0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {ewCarverBoxWidth / 2. - booleanSolidTolerance, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2. + fExitwallBRDepth};
        // Make the carver cling to the bottom
        ewCarverTlate += {0, -fBoronResinThickness / 2. - booleanSolidTolerance / 2., 0};

        G4DisplacedSolid *displacedBRTrd = new G4DisplacedSolid(
            "BoronResinCaseDisplacedTrd", boronResinTrd, nullptr, {0, 0, fIronThickness / 2.});

        G4SubtractionSolid *carvedBRCase =
            new G4SubtractionSolid("BoronResinCaseWExitwallSSolid", displacedBRTrd,
                                   exitwallCarverBox, nullptr, ewCarverTlate);

        G4double exitpathCarverHeight  = boronResinHeight + 2 * booleanSolidTolerance;
        G4double exitpathCarverZLength = fIronThickness + booleanSolidTolerance;
        G4double exitpathCarverMZWidth = boronResinWidthAtOrigDumpBoundary - fExitpathWidth -
                                         2 * fBoronResinThickness + booleanSolidTolerance;
        G4double exitpathCarverPZWidth =
            boronResinWidthDumpside - fExitpathWidth - 2 * fBoronResinThickness +
            ftLabWidthSlope * booleanSolidTolerance + booleanSolidTolerance;

        G4Trd *exitpathCarver = new G4Trd("BRCaseExitCarverTrd", exitpathCarverMZWidth / 2.,
                                          exitpathCarverPZWidth / 2., exitpathCarverHeight / 2.,
                                          exitpathCarverHeight / 2., exitpathCarverZLength / 2.);

        G4ThreeVector eCarverTlate = {0, 0, 0};
        // Compensate the displacement of G4DisplacedSolid
        eCarverTlate += {0, 0, fIronThickness / 2.};
        // Move +z to the +z-end of the Boron-resin solid with consideration of boolean tolerance
        eCarverTlate += {0, 0, boronResinZLength / 2. + booleanSolidTolerance / 2.};
        // Insert carver to -z to make the boron-resin+iron shield region
        eCarverTlate += {0, 0, -exitpathCarverZLength / 2.};
        // Move +x to make the exit path with consideration of boolean tolerance
        eCarverTlate += {fExitpathWidth / 2. + booleanSolidTolerance / 2., 0, 0};
        // Adding addition +x for the thickness of boron-resin layer
        eCarverTlate += {fBoronResinThickness, 0, 0};
        // Move +y to make the spacing of lab at the floor with consideration of boolean tolerance
        eCarverTlate += {0, fLabFloorSpacing + booleanSolidTolerance, 0};

        G4SubtractionSolid *brCaseWithExit = new G4SubtractionSolid(
            "BoronResinCaseSolid", carvedBRCase, exitpathCarver, nullptr, eCarverTlate);

        return brCaseWithExit;
    }

    G4VSolid *BL10DetectorConstruction::BuildLabSolid(G4bool simple) const {
        G4double labTrdZLength       = fLabZLength + fBoronResinThickness + fIronThickness;
        G4double labTrdHeight        = fLabHeight - booleanSolidTolerance;
        G4double labTrdWidthBeamside = fLabWidthBeamside;
        G4double labTrdWidthDumpside =
            fLabWidthDumpside + ftLabWidthSlope * (fBoronResinThickness + fIronThickness);

        G4Trd *labTrd = new G4Trd("LabTrd", labTrdWidthBeamside / 2., labTrdWidthDumpside / 2.,
                                  labTrdHeight / 2., labTrdHeight / 2., labTrdZLength / 2.);

        // Copied from BuildBoronResincaseSolid()
        // (assuming beamside < dumpside)
        G4double boronResinZLength = fLabZLength + 2 * fBoronResinThickness + fIronThickness;

        G4double boronResinWidthAtLabBeamBoundary = fLabWidthBeamside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtLabDumpBoundary = fLabWidthDumpside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtOrigBeamBoundary =
            boronResinWidthAtLabBeamBoundary - ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthAtOrigDumpBoundary =
            boronResinWidthAtLabDumpBoundary + ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthBeamside = boronResinWidthAtOrigBeamBoundary;
        G4double boronResinWidthDumpside =
            boronResinWidthAtOrigDumpBoundary + ftLabWidthSlope * fIronThickness;

        G4DisplacedSolid *displacedLabTrd = new G4DisplacedSolid(
            "DisplacedLabTrd", labTrd, nullptr, {0, booleanSolidTolerance / 2., 0});

        G4Trd *labFloorTrd =
            new G4Trd("LabFloorTrd", boronResinWidthBeamside / 2., boronResinWidthDumpside / 2.,
                      fLabFloorSpacing / 2., fLabFloorSpacing / 2., boronResinZLength / 2.);

        G4ThreeVector floorSpacingTlate = {0, 0, 0};
        // Moving the center of floor spaing to the bottom of lab
        floorSpacingTlate += {0, -fLabHeight / 2., 0};
        // Insert the floor spacing to the lab
        floorSpacingTlate += {0, fLabFloorSpacing / 2., 0};

        G4UnionSolid *labTrdWithFloor;
        if (simple) {
            labTrdWithFloor = new G4UnionSolid("LabFloorSolid", displacedLabTrd, labFloorTrd,
                                               nullptr, floorSpacingTlate);
            return labTrdWithFloor;
        } else {
            // Compensating a tlanslation for the the iron case
            floorSpacingTlate += {0, 0, labTrdZLength / 2. - boronResinZLength / 2.};

            labTrdWithFloor =
                new G4UnionSolid("LabFloorSolid", labTrd, labFloorTrd, nullptr, floorSpacingTlate);
        }

        G4double ewCarverBoxWidth =
            fExitwallWidth + fBoronResinThickness + booleanSolidTolerance * 2;
        G4double ewCarverBoxHeight  = fLabHeight + booleanSolidTolerance * 2;
        G4double ewCarverBoxZLength = fExitwallThickness;

        G4Box *exitwallCarverBox = new G4Box("LabExitwallCarverBox", ewCarverBoxWidth / 2.,
                                             ewCarverBoxHeight / 2., ewCarverBoxZLength / 2.);

        // Tlanslation for the carver
        G4ThreeVector ewCarverTlate = {0, 0, 0};
        // Move the carver center to the +z end
        ewCarverTlate += {0, 0, fLabZLength / 2.};
        // Move the center to the +z edge of exitwall Boron-resin shield
        ewCarverTlate += {0, 0, -fExitwallDistance};
        // Move the center to the x-border of exitwall
        ewCarverTlate += {-fLabWidthDumpside / 2. + ftLabWidthSlope * fExitwallDistance / 2., 0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {-ewCarverBoxWidth / 2., 0, 0};
        ewCarverTlate += {fExitwallWidth, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2.};

        G4DisplacedSolid *displacedLTWF =
            new G4DisplacedSolid("DisplacedLabFloorSolid", labTrdWithFloor, nullptr,
                                 {0, 0, (fBoronResinThickness + fIronThickness) / 2.});

        G4SubtractionSolid *carvedLab = new G4SubtractionSolid(
            "LabWExitwallSSolid", displacedLTWF, exitwallCarverBox, nullptr, ewCarverTlate);

        G4double exitpathCarverHeight = fLabHeight + 2 * booleanSolidTolerance;
        G4double exitpathCarverZLength =
            fBoronResinThickness + fIronThickness + booleanSolidTolerance;
        G4double exitpathCarverMZWidth =
            boronResinWidthDumpside - fExitpathWidth + booleanSolidTolerance;
        G4double exitpathCarverPZWidth = boronResinWidthDumpside - fExitpathWidth +
                                         ftLabWidthSlope * exitpathCarverZLength +
                                         booleanSolidTolerance;

        G4Trd *exitpathCarver = new G4Trd("LabExitCarverTrd", exitpathCarverMZWidth / 2.,
                                          exitpathCarverPZWidth / 2., exitpathCarverHeight / 2.,
                                          exitpathCarverHeight / 2., exitpathCarverZLength / 2.);

        G4ThreeVector eCarverTlate = {0, 0, 0};
        // Compensate the displacement of the previous instances of G4DisplacedSolid
        eCarverTlate += {0, 0, (fBoronResinThickness + fIronThickness) / 2.};
        // Move +z to the +z-end of the labTrd solid with consideration of boolean tolerance
        eCarverTlate += {0, 0, labTrdZLength / 2. + booleanSolidTolerance / 2.};
        // Insert carver to -z to make the boron-resin+iron shield region
        eCarverTlate += {0, 0, -exitpathCarverZLength / 2.};
        // Move +x to make the exit path with consideration of boolean tolerance
        eCarverTlate += {fExitpathWidth / 2. + booleanSolidTolerance / 2., 0, 0};
        // Adding addition +x for the thickness of boron-resin layer
        eCarverTlate += {fBoronResinThickness, 0, 0};
        // Move +y to make the spacing of lab at the floor with consideration of boolean tolerance
        eCarverTlate += {0, fLabFloorSpacing + booleanSolidTolerance, 0};

        G4SubtractionSolid *labWithExit = new G4SubtractionSolid(
            "LabWExitSolid", carvedLab, exitpathCarver, nullptr, eCarverTlate);

        return labWithExit;
    }

    G4LogicalVolume *BL10DetectorConstruction::FillIroncase(G4LogicalVolume *lv) const {
        G4Material *matB4C = G4Material::GetMaterial("G4_BORON_CARBIDE");
        G4Material *matAir = G4Material::GetMaterial("G4_AIR");

        G4LogicalVolume *boronResinLV = new G4LogicalVolume(
            BuildBoronResincaseSolid(fSimpleGeometry), matB4C, "BoronResinCaseLV");
        G4ThreeVector boronResinTlate = {
            0, -(fFloorThickness + fIronThickness) / 2. + fFloorThickness, 0};
        new G4PVPlacement(nullptr, boronResinTlate, boronResinLV, "BoronResinCasePV", lv, false, 0,
                          fCheckOverlaps);

        G4LogicalVolume *labLV =
            new G4LogicalVolume(BuildLabSolid(fSimpleGeometry), matAir, "LabLV");
        G4ThreeVector labTlate = {0, -fBoronResinThickness / 2., 0};
        new G4PVPlacement(nullptr, labTlate, labLV, "LabPV", boronResinLV, false, 0,
                          fCheckOverlaps);

        return labLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildWorkbench() const {
        G4Material *matAir = G4Material::GetMaterial("G4_AIR");
        G4Material *matSS  = G4Material::GetMaterial("Stainless_Steel");
        G4Material *matFe  = G4Material::GetMaterial("G4_Fe");

        G4Box *envelopeBox = new G4Box("WorkbenchEnvelopeBox", ftWBEnvelopeWidth / 2.,
                                       ftWBEnvelopeHeight / 2., ftWBEnvelopeZLength / 2.);
        G4LogicalVolume *envelopeLV =
            new G4LogicalVolume(envelopeBox, matAir, "WorkbenchEnvelopeLV");

        G4Box *plateBox          = new G4Box("WBPlateBox", fWorkbenchPlateWidth / 2.,
                                             fWorkbenchPlateThickness / 2., fWorkbenchPlateLength / 2.);
        G4LogicalVolume *plateLV = new G4LogicalVolume(plateBox, matSS, "WBPlateLV");

        G4ThreeVector plateTlate = {0, 0, 0};
        // Move the plate to the +y-end of the envelope
        plateTlate += {0, ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness / 2., 0};
        // Move the plate in the -z-end of the envelope
        plateTlate += {0, 0, -ftWBEnvelopeZLength / 2. + fWorkbenchPlateLength / 2.};
        // Add +z margin for the plate
        plateTlate += {0, 0, fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin};
        // Move the plate in the -x-end of the envelope
        plateTlate += {-ftWBEnvelopeWidth / 2. + fWorkbenchPlateWidth / 2., 0, 0};
        // Add +x margin for the plate
        plateTlate += {fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin, 0, 0};
        new G4PVPlacement(nullptr, plateTlate, plateLV, "WBPlatePV", envelopeLV, false, 0,
                          fCheckOverlaps);

        G4ExtrudedSolid *xSupportHBeamSolid = new G4ExtrudedSolid(
            "WBXSupportHBeamSolid", fHBeamPoints, fWorkbenchXSupportLength / 2.);
        G4ExtrudedSolid *zSupportHBeamSolid = new G4ExtrudedSolid(
            "WBZSupportHBeamSolid", fHBeamPoints, fWorkbenchZSupportLength / 2.);

        G4LogicalVolume *xSupportHBeamLV =
            new G4LogicalVolume(xSupportHBeamSolid, matFe, "WBXSupportHBeamLV");
        G4LogicalVolume *zSupportHBeamLV =
            new G4LogicalVolume(zSupportHBeamSolid, matFe, "WBZSupportHBeamLV");

        G4ThreeVector beamsideYHBeamTlate = {
            0, ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness - fWorkbenchSupportHeight / 2.,
            -ftWBEnvelopeZLength / 2. + fWorkbenchSupportWidth / 2. + fWorkbenchPlateOuterMargin};
        G4ThreeVector dumpsideYHBeamTlate = beamsideYHBeamTlate;
        dumpsideYHBeamTlate[G4ThreeVector::Z] *= -1;

        G4ThreeVector xPlussideZHBeamTlate = {
            fWorkbenchXSupportLength / 2. + fWorkbenchSupportWidth / 2.,
            ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness - fWorkbenchSupportHeight / 2.,
            -ftWBEnvelopeZLength / 2. + fWorkbenchZSupportLength / 2. + fWorkbenchPlateOuterMargin};
        G4ThreeVector xMinussideZHBeamTlate = xPlussideZHBeamTlate;
        xMinussideZHBeamTlate[G4ThreeVector::X] *= -1;

        G4RotationMatrix *xSupportHBeamRotMtx = new G4RotationMatrix();
        xSupportHBeamRotMtx->rotateY(90 * deg);

        new G4PVPlacement(xSupportHBeamRotMtx, beamsideYHBeamTlate, xSupportHBeamLV,
                          "WBXSupportHBeamPV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(xSupportHBeamRotMtx, dumpsideYHBeamTlate, xSupportHBeamLV,
                          "WBXSupportHBeamPV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(nullptr, xPlussideZHBeamTlate, zSupportHBeamLV, "WBZSuppoerHBeamPV",
                          envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, xMinussideZHBeamTlate, zSupportHBeamLV, "WBZSuppoerHBeamPV",
                          envelopeLV, true, 1, fCheckOverlaps);

        G4Box *wbPipeEndplate1Box =
            new G4Box("WBPipeEndplate1Box", fWBPipeEndplate1Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate1Length / 2.);
        G4Box *wbPipeEndplate2Box =
            new G4Box("WBPipeEndplate2Box", fWBPipeEndplate2Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate2Length / 2.);
        G4Box *wbPipeEndplate3Box =
            new G4Box("WBPipeEndplate3Box", fWBPipeEndplate3Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate3Length / 2.);

        G4LogicalVolume *wbPipeEndplate1LV =
            new G4LogicalVolume(wbPipeEndplate1Box, matFe, "WBEndplate1LV");
        G4LogicalVolume *wbPipeEndplate2LV =
            new G4LogicalVolume(wbPipeEndplate2Box, matFe, "WBEndplate2LV");
        G4LogicalVolume *wbPipeEndplate3LV =
            new G4LogicalVolume(wbPipeEndplate3Box, matFe, "WBEndplate3LV");

        G4ThreeVector vpTopEndplateXTlate = {-ftWBEnvelopeWidth / 2. + fWBPipeEndplate2Width / 2.,
                                             0, 0};
        G4ThreeVector vpTopEndplateYTlate = {0,
                                             ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness -
                                                 fWorkbenchSupportHeight -
                                                 fWBPipeEndplateThickness / 2.,
                                             0};
        G4ThreeVector vpTopEndplateBeamsideTlate = {
            0, 0, -ftWBEnvelopeZLength / 2. + fWBPipeEndplate2Length / 2.};
        G4ThreeVector vpTopEndplateDumpsideTlate = {
            0, 0, ftWBEnvelopeZLength / 2. - fWBPipeEndplate1Length / 2.};

        new G4PVPlacement(
            nullptr, vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateBeamsideTlate,
            wbPipeEndplate2LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateBeamsideTlate,
            wbPipeEndplate2LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);

        new G4PVPlacement(
            nullptr, vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4Tubs *wbVertPipeTub =
            new G4Tubs("WBVertPipeTub", fWBVertPipeRadius - fWBVertPipeThickness, fWBVertPipeRadius,
                       fWBVertPipeLength / 2., 0, 360 * deg);
        G4LogicalVolume *wbVertPipeLV = new G4LogicalVolume(wbVertPipeTub, matFe, "WBVertPipeLV");

        G4ThreeVector vpXTlate = vpTopEndplateXTlate;
        G4ThreeVector vpYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplateThickness + fWBVertPipeLength / 2., 0};
        G4ThreeVector vpBeamsideTlate = {
            0, 0, -ftWBEnvelopeZLength / 2. + fWBPipeEndplate2Length - fWBPipeEndplate1Length / 2.};
        G4ThreeVector vpDumpsideTlate = {0, 0,
                                         ftWBEnvelopeZLength / 2. - fWBPipeEndplate1Length / 2.};

        G4RotationMatrix *wbVertPipeRotMtx = new G4RotationMatrix();
        wbVertPipeRotMtx->rotateX(90 * deg);

        new G4PVPlacement(wbVertPipeRotMtx, vpXTlate + vpYTlate + vpBeamsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(wbVertPipeRotMtx, -vpXTlate + vpYTlate + vpBeamsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(wbVertPipeRotMtx, vpXTlate + vpYTlate + vpDumpsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(wbVertPipeRotMtx, -vpXTlate + vpYTlate + vpDumpsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector bottomEndplateXTlate = vpTopEndplateXTlate;
        G4ThreeVector bottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplateThickness / 2., 0};
        G4ThreeVector bottomEndplateBeamsideTlate = vpBeamsideTlate;
        G4ThreeVector bottomEndplateDumpsideTlate = vpDumpsideTlate;

        new G4PVPlacement(
            nullptr, bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateBeamsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateBeamsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4Tubs *wbZDiagPipeTub =
            new G4Tubs("WBZDiagPipeTub", fWBZDiagPipeRadius - fWBZDiagPipeThickness,
                       fWBZDiagPipeRadius, fWBZDiagPipeLength / 2., 0, 360 * deg);
        G4Tubs *wbXDiagPipeTub =
            new G4Tubs("WBXDiagPipeTub", fWBXDiagPipeRadius - fWBXDiagPipeThickness,
                       fWBXDiagPipeRadius, fWBXDiagPipeLength / 2., 0, 360 * deg);

        G4LogicalVolume *wbZDiagPipeLV =
            new G4LogicalVolume(wbZDiagPipeTub, matFe, "WBZDiagPipeLV");
        G4LogicalVolume *wbXDiagPipeLV =
            new G4LogicalVolume(wbXDiagPipeTub, matFe, "WBXDiagPipeLV");

        G4ThreeVector zdpBottomEndplateXTlate = vpTopEndplateXTlate;
        G4ThreeVector zdpBottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplate3Width / 2. + fWBZDiagPipeYDistance, 0};
        G4ThreeVector zdpBottomEndplateBeamsideTlate = vpBeamsideTlate;
        zdpBottomEndplateBeamsideTlate += {0, 0, fWBVertPipeRadius + fWBPipeEndplateThickness / 2.};
        G4ThreeVector zdpBottomEndplateDumpsideTlate = vpDumpsideTlate;
        zdpBottomEndplateDumpsideTlate +=
            {0, 0, -fWBVertPipeRadius - fWBPipeEndplateThickness / 2.};

        G4RotationMatrix *zdpBottomRotMtx = new G4RotationMatrix();
        zdpBottomRotMtx->rotateX(90 * deg);

        new G4PVPlacement(
            zdpBottomRotMtx,
            zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            zdpBottomRotMtx,
            -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            zdpBottomRotMtx,
            zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            zdpBottomRotMtx,
            -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector zdpBeamsideTlate = {0, fWBZDiagPipeLength / 2. + fWBZDiagPipeRadius * 1.5, 0};
        G4ThreeVector zdpDumpsideTlate = {0, fWBZDiagPipeLength / 2. + fWBZDiagPipeRadius * 1.5, 0};
        zdpBeamsideTlate.rotateX(fWBZDiagPipeAngle);
        zdpDumpsideTlate.rotateX(-fWBZDiagPipeAngle);

        G4RotationMatrix *zdpBeamsideRotMtx = new G4RotationMatrix();
        zdpBeamsideRotMtx->rotateX(fWBZDiagPipeAngle);
        G4RotationMatrix *zdpDumpsideRotMtx = new G4RotationMatrix();
        zdpDumpsideRotMtx->rotateX(-fWBZDiagPipeAngle);

        new G4PVPlacement(zdpBeamsideRotMtx,
                          zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateBeamsideTlate + zdpBeamsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(zdpBeamsideRotMtx,
                          -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateBeamsideTlate + zdpBeamsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(zdpDumpsideRotMtx,
                          zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateDumpsideTlate + zdpDumpsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(zdpDumpsideRotMtx,
                          -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateDumpsideTlate + zdpDumpsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector zdpTopEndplateXTlate = zdpBottomEndplateXTlate;
        G4ThreeVector zdpTopEndplateYTlate = {0,
                                              ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness -
                                                  fWorkbenchSupportWidth -
                                                  fWBPipeEndplateThickness / 2.,
                                              0};

        G4double zdpBotEPDistFromVertTop =
            fWBVertPipeLength -
            (fWBZDiagPipeYDistance - fWBPipeEndplateThickness + fWBPipeEndplate3Width / 2.);

        G4ThreeVector zdpTopEndplateBeamsideTlate = vpBeamsideTlate;
        zdpTopEndplateBeamsideTlate += {0, 0, zdpBotEPDistFromVertTop * tan(fWBZDiagPipeAngle)};
        G4ThreeVector zdpTopEndplateDumpsideTlate = vpDumpsideTlate;
        zdpTopEndplateDumpsideTlate += {0, 0, -zdpBotEPDistFromVertTop * tan(fWBZDiagPipeAngle)};

        new G4PVPlacement(
            nullptr, zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector xdpBottomEndplateXTlate = vpTopEndplateXTlate;
        xdpBottomEndplateXTlate += {fWBVertPipeRadius + fWBPipeEndplateThickness / 2., 0, 0};
        G4ThreeVector xdpBottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplate3Width / 2. + fWBXDiagPipeYDistance, 0};
        G4ThreeVector xdpBottomEndplateBeamsideTlate = vpBeamsideTlate;
        G4ThreeVector xdpBottomEndplateDumpsideTlate = vpDumpsideTlate;

        G4RotationMatrix *xdpBottomRotMtx = new G4RotationMatrix();
        xdpBottomRotMtx->rotateZ(90 * deg);
        xdpBottomRotMtx->rotateY(90 * deg);

        new G4PVPlacement(
            xdpBottomRotMtx,
            xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector xdpXPlussideTlate = {0, fWBXDiagPipeLength / 2. + fWBXDiagPipeRadius * 2.5,
                                           0};
        xdpXPlussideTlate.rotateZ(-fWBXDiagPipeAngle);
        G4ThreeVector xdpXMinussideTlate = {0, fWBXDiagPipeLength / 2. + fWBXDiagPipeRadius * 2.5,
                                            0};
        xdpXMinussideTlate.rotateZ(fWBXDiagPipeAngle);

        G4RotationMatrix *xdpXPlussideRotMtx = new G4RotationMatrix();
        xdpXPlussideRotMtx->rotateX(90 * deg);
        xdpXPlussideRotMtx->rotateY(-fWBXDiagPipeAngle);
        G4RotationMatrix *xdpXMinussideRotMtx = new G4RotationMatrix();
        xdpXMinussideRotMtx->rotateX(90 * deg);
        xdpXMinussideRotMtx->rotateY(fWBXDiagPipeAngle);

        new G4PVPlacement(xdpXPlussideRotMtx,
                          xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateBeamsideTlate + xdpXPlussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(xdpXMinussideRotMtx,
                          -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateBeamsideTlate + xdpXMinussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(xdpXPlussideRotMtx,
                          xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateDumpsideTlate + xdpXPlussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(xdpXMinussideRotMtx,
                          -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateDumpsideTlate + xdpXMinussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4double xdpBotEPDistFromVertTop =
            fWBVertPipeLength -
            (fWBXDiagPipeYDistance - fWBPipeEndplateThickness + fWBPipeEndplate3Width / 2.);

        G4ThreeVector xdpTopEndplateXTlate = vpXTlate;
        xdpTopEndplateXTlate += {xdpBotEPDistFromVertTop * tan(fWBXDiagPipeAngle), 0, 0};
        G4ThreeVector xdpTopEndplateYTlate = zdpTopEndplateYTlate;

        G4ThreeVector xdpTopEndplateBeamsideTlate = xdpBottomEndplateBeamsideTlate;
        G4ThreeVector xdpTopEndplateDumpsideTlate = xdpBottomEndplateDumpsideTlate;

        new G4PVPlacement(
            nullptr, xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        return envelopeLV;
    }

    G4ThreeVector BL10DetectorConstruction::PlaceWorkbench(G4LogicalVolume *labLV,
                                                           G4LogicalVolume *wbLV) const {
        G4Material *matFe = G4Material::GetMaterial("G4_Fe");

        G4ThreeVector wbTlate = {0, 0, 0};

        // Move the workbench to the center of beamline
        wbTlate += {+fLabWidthBeamside / 2. - fBeamXDistanceFromWall, 0, 0};
        // Move the workbench to the earth
        wbTlate += {0, -fLabHeight / 2. + ftWBEnvelopeHeight / 2., 0};
        // Move the workbench to the beamside wall of lab
        wbTlate += {0, 0, -fLabZLength / 2. + ftWBEnvelopeZLength / 2};
        // Move the workbench to match the distance from wall
        wbTlate += {0, 0, fWBZDistanceFromWall};

        // Move wb
        new G4PVPlacement(nullptr, wbTlate, wbLV, "WorkbenchPV", labLV, false, 0, fCheckOverlaps);

        G4ExtrudedSolid *wbBoltSolid =
            new G4ExtrudedSolid("WBBoltSolid", fLevelingBoltPoints, fWBLevelingBoltThickness / 2.);
        G4LogicalVolume *wbBoltLV = new G4LogicalVolume(wbBoltSolid, matFe, "WBBoltLV");

        G4RotationMatrix *wbBoltRotMtx = new G4RotationMatrix();
        wbBoltRotMtx->rotateX(90 * deg);

        G4ThreeVector wbBoltXTlate = {-ftWBEnvelopeWidth / 2. + fWBLevelingBoltSize / 2. +
                                          fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin,
                                      0, 0};
        G4ThreeVector wbBoltYTlate = {0, ftWBEnvelopeHeight / 2. + fWBLevelingBoltThickness / 2.,
                                      0};
        G4ThreeVector wbBoltZTlate = {0, 0,
                                      -ftWBEnvelopeZLength / 2. + fWBLevelingBoltSize / 2. +
                                          fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin};

        new G4PVPlacement(wbBoltRotMtx, wbTlate + wbBoltXTlate + wbBoltYTlate + wbBoltZTlate,
                          wbBoltLV, "WBBoltPV", labLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(wbBoltRotMtx, wbTlate - wbBoltXTlate + wbBoltYTlate + wbBoltZTlate,
                          wbBoltLV, "WBBoltPV", labLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(wbBoltRotMtx, wbTlate + wbBoltXTlate + wbBoltYTlate - wbBoltZTlate,
                          wbBoltLV, "WBBoltPV", labLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(wbBoltRotMtx, wbTlate - wbBoltXTlate + wbBoltYTlate - wbBoltZTlate,
                          wbBoltLV, "WBBoltPV", labLV, true, 3, fCheckOverlaps);

        G4ThreeVector samplePosition{0, ftWBEnvelopeHeight / 2., fSampleZPosFromWBCenter};
        samplePosition += wbTlate;
        return samplePosition;
    }

    void BL10DetectorConstruction::PlaceSamples(G4LogicalVolume *labLV,
                                                const G4ThreeVector sampleTlate) const {
        G4Material *matPb = G4Material::GetMaterial("G4_Be");

        G4Box *exampleBox          = new G4Box("ExampleBox", 10 * cm, 10 * cm, 10 * cm);
        G4LogicalVolume *exampleLV = new G4LogicalVolume(exampleBox, matPb, "ExampleLV");

        G4ThreeVector exampleTlate = sampleTlate;
        exampleTlate += {0, 10 * cm, 0};
        new G4PVPlacement(nullptr, exampleTlate, exampleLV, "ExamplePV", labLV, false, 0,
                          fCheckOverlaps);
    }

    G4VPhysicalVolume *BL10DetectorConstruction::DefineVolumes() {
        std::string geomType = "bl10_";
        if (fSimpleGeometry) {
            geomType += "simple";
        } else {
            geomType += "detailed";
        }
        simcore::MetadataManager::GetInstance().SetGeometryType(geomType);

        G4LogicalVolume *ironcaseLV   = BuildIroncase();
        G4VPhysicalVolume *ironcasePV = new G4PVPlacement(nullptr, {}, ironcaseLV, "IroncasePV",
                                                          nullptr, false, 0, fCheckOverlaps);

        G4LogicalVolume *labLV       = FillIroncase(ironcaseLV);
        G4LogicalVolume *wbLV        = BuildWorkbench();
        G4ThreeVector samplePosition = PlaceWorkbench(labLV, wbLV);

        PlaceSamples(labLV, samplePosition);

        return ironcasePV;
    }

    void BL10DetectorConstruction::ConstructSDandField() {}
} // namespace bl10sim
