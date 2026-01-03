#include "bl10sim/BL10DetectorConstruction.h"

#include "G4Box.hh"
#include "G4DisplacedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"

static const G4double booleanSolidTolerance = 100 * um;

namespace bl10sim {
    BL10DetectorConstruction::BL10DetectorConstruction() : fSimpleGeometry(false) {
        SetGeometryParameters();
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
        fBoronResinThickness = 20 * cm;
        fIronThickness       = 10 * cm;
        fFloorThickness      = 10 * cm;

        fLabHeight        = 3.0 * m;
        fLabZLength       = 3.5 * m;
        fLabWidthBeamside = 1.9 * m;
        fLabWidthDumpside = 3.1 * m;

        fExitwallDistance  = 60 * cm;
        fExitwallThickness = 50 * cm;
        fExitwallWidth     = 1 * m;
        fExitwallBRDepth   = 10 * cm;

        fExitpathWidth = 60 * cm;

        ftLabWidthSlope = (fLabWidthDumpside - fLabWidthBeamside) / fLabZLength;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildIroncase(G4bool floor) const {
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

        if (floor) {
            G4Material *matConcrete = G4Material::GetMaterial("G4_CONCRETE");
            G4Trd *floorTrd =
                new G4Trd("FloorTrd", worldWidthBeamside / 2., worldWidthDumpside / 2.,
                          fFloorThickness / 2., fFloorThickness / 2., worldZLength / 2.);
            G4LogicalVolume *floorLV = new G4LogicalVolume(floorTrd, matConcrete, "FloorLV");

            G4ThreeVector floorTlate = {0, -worldHeight / 2. + fFloorThickness / 2., 0};
            new G4PVPlacement(nullptr, floorTlate, floorLV, "FloorPV", worldLV, false, 0,
                              fCheckOverlaps);
        }

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

        G4DisplacedSolid *displacedBRTrd = new G4DisplacedSolid(
            "BoronResinCaseDisplacedTrd", boronResinTrd, nullptr, {0, 0, fIronThickness / 2.});

        if (simple) return boronResinTrd;

        G4double carverBoxWidth =
            fExitwallWidth - fExitwallBRDepth + fBoronResinThickness + booleanSolidTolerance * 2;
        G4double carverBoxHeight    = fLabHeight + booleanSolidTolerance;
        G4double carverBoxThickness = fExitwallThickness - 2 * fExitwallBRDepth;

        G4Box *exitwallCarverBox = new G4Box("BoronResinCaseExitwallCarverBox", carverBoxWidth / 2.,
                                             carverBoxHeight / 2., carverBoxThickness / 2.);

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
        ewCarverTlate +=
            {-boronResinWidthDumpside / 2. +
                 ftLabWidthSlope * (fExitwallDistance + fExitwallBRDepth + fBoronResinThickness) /
                     2.,
             0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {carverBoxWidth / 2. - booleanSolidTolerance, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2. + fExitwallBRDepth};
        // Make the carver cling to the bottom
        ewCarverTlate += {0, -fBoronResinThickness / 2. - booleanSolidTolerance / 2., 0};

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

        G4SubtractionSolid *brCaseWithExit = new G4SubtractionSolid(
            "BoronResinCaseSolid", carvedBRCase, exitpathCarver, nullptr,
            {fExitpathWidth / 2. + fBoronResinThickness + booleanSolidTolerance / 2., 0,
             fIronThickness / 2. + boronResinZLength / 2. - exitpathCarverZLength / 2. +
                 booleanSolidTolerance / 2.});

        return brCaseWithExit;
    }

    G4VSolid *BL10DetectorConstruction::BuildLabSolid(G4bool simple) const {
        G4double labTrdZLength = fLabZLength + fBoronResinThickness + fIronThickness;
        G4double labTrdWidthDumpside =
            fLabWidthDumpside + ftLabWidthSlope * (fBoronResinThickness + fIronThickness);
        G4Trd *labTrd = new G4Trd("LabTrd", fLabWidthBeamside / 2., labTrdWidthDumpside / 2.,
                                  fLabHeight / 2., fLabHeight / 2., labTrdZLength / 2.);

        G4DisplacedSolid *labDisplacedTrd =
            new G4DisplacedSolid("LabDisplacedTrd", labTrd, nullptr,
                                 {0, 0, (fBoronResinThickness + fIronThickness) / 2.});

        if (simple) return labTrd;

        G4Box *exitwallCarverBox =
            new G4Box("LabExitwallCarverBox", fExitwallWidth / 2. + booleanSolidTolerance,
                      fLabHeight / 2. + booleanSolidTolerance, fExitwallThickness / 2.);

        // Tlanslation for the carver
        G4ThreeVector ewCarverTlate = {0, 0, 0};
        // Move the carver center to the +z end
        ewCarverTlate += {0, 0, fLabZLength / 2.};
        // Move the center to the +z edge of exitwall Boron-resin shield
        ewCarverTlate += {0, 0, -fExitwallDistance};
        // Move the center to the x-border of exitwall
        ewCarverTlate += {-fLabWidthDumpside / 2. + ftLabWidthSlope * fExitwallDistance / 2., 0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {fExitwallWidth / 2. - booleanSolidTolerance, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2.};

        G4SubtractionSolid *carvedLab = new G4SubtractionSolid(
            "LabWExitwallSSolid", labDisplacedTrd, exitwallCarverBox, nullptr, ewCarverTlate);

        G4double exitpathCarverHeight = fLabHeight + 2 * booleanSolidTolerance;
        G4double exitpathCarverZLength =
            fBoronResinThickness + fIronThickness + booleanSolidTolerance;
        G4double exitpathCarverMZWidth = fLabWidthDumpside - fExitpathWidth + booleanSolidTolerance;
        G4double exitpathCarverPZWidth = fLabWidthDumpside - fExitpathWidth +
                                         ftLabWidthSlope * exitpathCarverZLength +
                                         booleanSolidTolerance;

        G4Trd *exitpathCarver = new G4Trd("LabExitCarverTrd", exitpathCarverMZWidth / 2.,
                                          exitpathCarverPZWidth / 2., exitpathCarverHeight / 2.,
                                          exitpathCarverHeight / 2., exitpathCarverZLength / 2.);

        G4SubtractionSolid *labWithExit = new G4SubtractionSolid(
            "labSolid", carvedLab, exitpathCarver, nullptr,
            {fExitpathWidth / 2. + booleanSolidTolerance / 2., 0,
             (fBoronResinThickness + fIronThickness) / 2. + labTrdZLength / 2. -
                 exitpathCarverZLength / 2. + booleanSolidTolerance / 2.});

        return labWithExit;
    }

    void BL10DetectorConstruction::FillIroncase(G4LogicalVolume *lv) const {
        G4Material *matB4C            = G4Material::GetMaterial("G4_BORON_CARBIDE");
        G4Material *matAir            = G4Material::GetMaterial("G4_AIR");
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
    }

    G4VPhysicalVolume *BL10DetectorConstruction::DefineVolumes() {
        G4LogicalVolume *ironcaseLV   = BuildIroncase(true);
        G4VPhysicalVolume *ironcasePV = new G4PVPlacement(nullptr, {}, ironcaseLV, "IroncasePV",
                                                          nullptr, false, 0, fCheckOverlaps);
        FillIroncase(ironcaseLV);

        return ironcasePV;
    }

    void BL10DetectorConstruction::ConstructSDandField() {}
} // namespace bl10sim
