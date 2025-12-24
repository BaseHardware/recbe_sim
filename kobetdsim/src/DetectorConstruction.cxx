#include <string>

#include "DetectorConstruction.h"

#include "DetectorMessenger.h"

#include "FPGASD.h"

#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4GeometryManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"

constexpr G4double booleanGeomTolerance = 100 * um;
constexpr G4double boltBodyRadiusMargin = 10 * um;

namespace kobetdsim {
    DetectorConstruction::DetectorConstruction()
        : fUseBoratedPE(false), fMessenger(new DetectorMessenger(this)), fGeometryType("default") {}

    DetectorConstruction::~DetectorConstruction() { delete fMessenger; }

    G4VPhysicalVolume *DetectorConstruction::Construct() {
        // Define materials
        DefineMaterials();

        // Define volumes
        return DefineVolumes();
    }

    static G4LogicalVolume *MakeDosimeter() {
        G4double entireBoxWidth     = 1.8 * cm;
        G4double entireBoxHeight    = 2.2 * cm;
        G4double entireBoxThickness = 0.7 * cm;
        G4Material *entireBoxMat    = G4Material::GetMaterial("Acrylic");
        // up 0.3 * cm;
        // down 0.2 * cm;

        G4double blackBoxWidth     = 1.1 * cm;
        G4double blackBoxHeight    = 1.8 * cm;
        G4double blackBoxThickness = 0.2 * cm;
        G4Material *blackBoxMat    = G4Material::GetMaterial("G4_POLYETHYLENE");

        G4double neutDosiBoxWidth     = 1.1 * cm;
        G4double neutDosiBoxHeight    = 1.8 * cm;
        G4double neutDosiBoxThickness = 0.2 * cm;
        G4Material *neutDosiBoxMat    = G4Material::GetMaterial("LowDensity_PE");

        G4double neutDosiBoxWallThickness = 0.02 * cm;
        G4Material *neutDosiIBoxMat       = G4Material::GetMaterial("G4_AIR");

        G4double cr39Height    = 1.6 * cm;
        G4double cr39Width     = 1.0 * cm;
        G4double cr39Thickness = 0.07 * cm;
        G4Material *cr39Mat    = G4Material::GetMaterial("CR-39");

        G4double protonRadHeight    = 1.6 * cm;
        G4double protonRadWidth     = 1.0 * cm;
        G4double protonRadThickness = 0.04 * cm;
        G4Material *protonRadMat    = G4Material::GetMaterial("G4_POLYETHYLENE");

        G4double alphaCaptureWidth     = 1.0 * cm;
        G4double alphaCaptureHeight    = 0.8 * cm;
        G4double alphaCaptureThickness = 0.03 * cm;
        G4Material *alphaCaptureMat    = G4Material::GetMaterial("BoronNitride");

        G4Box *entireBox = new G4Box("Dosimeter_Box", entireBoxHeight / 2., entireBoxWidth / 2.,
                                     entireBoxThickness / 2.);
        G4LogicalVolume *entireLV = new G4LogicalVolume(entireBox, entireBoxMat, "Dosimeter_LV");

        G4Box *blackBox = new G4Box("BlackDosi_Box", blackBoxHeight / 2., blackBoxWidth / 2.,
                                    blackBoxThickness / 2.);
        G4LogicalVolume *blackBoxLV = new G4LogicalVolume(blackBox, blackBoxMat, "BlackDosi_LV");

        G4Box *neutDosiBox = new G4Box("NeutronDosi_Box", neutDosiBoxHeight / 2.,
                                       neutDosiBoxWidth / 2., neutDosiBoxThickness / 2.);
        G4LogicalVolume *neutDosiLV =
            new G4LogicalVolume(neutDosiBox, neutDosiBoxMat, "NeutronDosi_LV");

        G4Box *neutDosiIBox =
            new G4Box("NeutronDosiInner_Box", neutDosiBoxHeight / 2. - neutDosiBoxWallThickness,
                      neutDosiBoxWidth / 2. - neutDosiBoxWallThickness,
                      neutDosiBoxThickness / 2. - neutDosiBoxWallThickness);
        G4LogicalVolume *neutDosiILV =
            new G4LogicalVolume(neutDosiIBox, neutDosiIBoxMat, "NeutronDosiInner_LV");

        G4Box *cr39Box =
            new G4Box("CR-39_Box", cr39Height / 2., cr39Width / 2., cr39Thickness / 2.);
        G4LogicalVolume *cr39LV = new G4LogicalVolume(cr39Box, cr39Mat, "Detector_LV");

        G4Box *protonRadBox = new G4Box("ProtonRad_Box", protonRadHeight / 2., protonRadWidth / 2.,
                                        protonRadThickness / 2.);
        G4LogicalVolume *protonRadLV =
            new G4LogicalVolume(protonRadBox, protonRadMat, "ProtonRad_LV");

        G4Box *alphaCaptureBox = new G4Box("AlphaCapture_Box", alphaCaptureHeight / 2.,
                                           alphaCaptureWidth / 2., alphaCaptureThickness / 2.);
        G4LogicalVolume *alphaCaptureLV =
            new G4LogicalVolume(alphaCaptureBox, alphaCaptureMat, "AlphaCapture_LV");

        G4double neutDosiIBoxThickness = neutDosiBoxThickness - neutDosiBoxWallThickness * 2.;
        G4double neutDosiComplexZOffset =
            neutDosiIBoxThickness - cr39Thickness - protonRadThickness - alphaCaptureThickness;
        neutDosiComplexZOffset /= 2.;

        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0,
                                        -neutDosiIBoxThickness / 2. + neutDosiComplexZOffset +
                                            cr39Thickness / 2.),
                          cr39LV, "CR-39_LV", neutDosiILV, false, 0, true);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(-cr39Height / 2. + alphaCaptureHeight / 2., 0,
                                        -neutDosiIBoxThickness / 2. + neutDosiComplexZOffset +
                                            cr39Thickness + alphaCaptureThickness / 2.),
                          alphaCaptureLV, "AlphaCapture_LV", neutDosiILV, false, 0, true);
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0,
                                        -neutDosiIBoxThickness / 2. + neutDosiComplexZOffset +
                                            cr39Thickness + alphaCaptureThickness +
                                            protonRadThickness / 2.),
                          protonRadLV, "ProtonRad_PV", neutDosiILV, false, 0, true);

        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), neutDosiILV, "NeutronDosiInner_PV",
                          neutDosiLV, false, 0, true);

        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, blackBoxThickness / 2.), blackBoxLV,
                          "BlackDosi_PV", entireLV, false, 0, true);
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -neutDosiBoxThickness / 2.), neutDosiLV,
                          "NeutronDosi_PV", entireLV, false, 0, true);

        return entireLV;
    }

    static G4LogicalVolume *MakeBeampipeGeometry(G4bool detailed) {
        using namespace std;

        G4Material *targetMat = G4Material::GetMaterial("G4_Be");
        G4Material *plungeMat = G4Material::GetMaterial("G4_Al");
        G4Material *vacuumMat = G4Material::GetMaterial("G4_Galactic");
        G4Material *susMat    = G4Material::GetMaterial("Stainless_Steel");
        G4Material *matAir    = G4Material::GetMaterial("G4_AIR");
        G4Material *cermMat   = G4Material::GetMaterial("Ceramic");

        G4double targetRadius = 2.0 * cm;
        G4double targetLength = 1 * mm;

        // Step 0: Making Beryllium target
        G4Tubs *targetTube =
            new G4Tubs("Target_Tub", 0, targetRadius, targetLength / 2., 0, 360 * deg);
        G4LogicalVolume *targetLV = new G4LogicalVolume(targetTube, targetMat, "Target_LV");

        if (detailed) {
            // Detailed geometry
            G4VSolid *nowTempSolid;

            G4double boltHeadRadius     = 13 * mm / 2.;
            G4double boltHeadThickness  = 5.3 * mm;
            G4double boltPositionMargin = 1 * mm;

            G4double boltBodyRadius = 8 * mm / 2.;
            G4double boltBodyLength = 50 * mm;

            G4double plungeDiscRadius    = 80 * mm / 2.;
            G4double plungeDiscThickness = 15 * mm;

            G4double plungeGrooveDepth  = 7.5 * mm;
            G4double plungeGrooveRadius = 40 * mm / 2.;

            G4double targetPipeLength  = 15 * mm;
            G4double targetPipeORadius = 80 * mm / 2.;
            G4double targetPipeIRadius =
                80 * mm / 2. - 20 * mm; // Thickness = 20 * mm (ot match with groove)

            G4double separatorPipeORadius = 30 * mm / 2.;
            G4double separatorPipeIRadius = 30 * mm / 2. - 1 * mm; // thinkness: 1 mm
            G4double separatorPipeLength  = 10 * mm;

            G4double ceramicInsulatorLength  = 50 * mm;
            G4double ceramicInsulatorORadius = 40 * mm / 2.;
            G4double ceramicInsulatorIRadius = 40 * mm / 2. - 2 * mm; // thinkness: 1 mm

            G4double endplateThickness = 1 * mm;

            // Step 1-1: Making plunge with the bolt holes
            G4Tubs *plungeTub = new G4Tubs("PlungeRaw_Tub", 0, plungeDiscRadius,
                                           plungeDiscThickness / 2., 0, 360 * deg);
            G4Tubs *plungeBoltHole =
                new G4Tubs("PlungeBoltHole_Tub", 0, boltBodyRadius + boltBodyRadiusMargin,
                           plungeDiscThickness / 2. + booleanGeomTolerance, 0, 360 * deg);
            G4ThreeVector plungeBoltHoleTrans = {
                plungeDiscRadius - boltHeadRadius - boltPositionMargin, 0, 0};

            nowTempSolid = plungeTub;
            for (int i = 0; i < 6; i++) {
                G4String nowName = "Plunge_BoltPunched_Solid_" + to_string(i);
                nowTempSolid     = new G4SubtractionSolid(nowName, nowTempSolid, plungeBoltHole,
                                                          nullptr, plungeBoltHoleTrans);
                plungeBoltHoleTrans.rotateZ(60 * deg);
            }

            // Step 1-2: Finalizing plunge by graving a groove in the center
            G4Tubs *plungeGraverTub =
                new G4Tubs("PlungeGraver_Tub", 0, plungeGrooveRadius,
                           plungeGrooveDepth / 2. + booleanGeomTolerance, 0, 360 * deg);

            G4ThreeVector plungeGraverTrans(0, 0, -plungeGrooveDepth / 2. - booleanGeomTolerance);
            G4SubtractionSolid *plungeGroovedTub = new G4SubtractionSolid(
                "Plunge_Solid", nowTempSolid, plungeGraverTub, nullptr, plungeGraverTrans);

            G4LogicalVolume *plungeLV =
                new G4LogicalVolume(plungeGroovedTub, plungeMat, "Plunge_LV");

            // Step 2: Making a bolt
            G4Tubs *boltHeadTub =
                new G4Tubs("BoltHead_Tub", 0, boltHeadRadius, boltHeadThickness / 2., 0, 360 * deg);
            G4Tubs *boltBodyTub =
                new G4Tubs("BoltBody_Tub", 0, boltBodyRadius,
                           boltBodyLength / 2. + booleanGeomTolerance, 0, 360 * deg);
            G4ThreeVector boltBodyTrans = {
                0, 0, boltHeadThickness / 2. + boltBodyLength / 2. - booleanGeomTolerance};
            G4UnionSolid *boltSolid =
                new G4UnionSolid("Bolt_Solid", boltHeadTub, boltBodyTub, nullptr, boltBodyTrans);
            G4LogicalVolume *boltLV = new G4LogicalVolume(boltSolid, susMat, "Bolt_LV");

            // Step 3: Making a target and separator pipes with bolt holes
            G4Tubs *targetPipeTub = new G4Tubs("TargetPipe_Tub", 0, targetPipeORadius,
                                               targetPipeLength / 2., 0, 360 * deg);
            G4Tubs *targetPipeBoltHole =
                new G4Tubs("TargetPipeBoltHole_Tub", 0, boltBodyRadius + boltBodyRadiusMargin,
                           targetPipeLength / 2. + booleanGeomTolerance, 0, 360 * deg);
            G4ThreeVector targetPipeBoltHoleTrans = {
                targetPipeORadius - boltHeadRadius - boltPositionMargin, 0, 0};

            nowTempSolid = targetPipeTub;
            for (int i = 0; i < 6; i++) {
                G4String nowName = "TargetPipe_BoltPunched_Solid_" + to_string(i);
                nowTempSolid     = new G4SubtractionSolid(nowName, nowTempSolid, targetPipeBoltHole,
                                                          nullptr, targetPipeBoltHoleTrans);
                targetPipeBoltHoleTrans.rotateZ(60 * deg);
            }
            G4Tubs *separatorPipeTub =
                new G4Tubs("SeparatorPipe_Tub", 0, separatorPipeORadius,
                           separatorPipeLength / 2. + booleanGeomTolerance, 0, 360 * deg);

            G4ThreeVector separatorPipeTrans = {
                0, 0, targetPipeLength / 2. + separatorPipeLength / 2. - booleanGeomTolerance};
            G4VSolid *targetSeparatorPipeSolid =
                new G4UnionSolid("TargetSeparatorPipe_Solid", nowTempSolid, separatorPipeTub,
                                 nullptr, separatorPipeTrans);

            G4LogicalVolume *targetSeparatorPipeLV =
                new G4LogicalVolume(targetSeparatorPipeSolid, susMat, "TargetSeparatorPipe_LV");

            G4Tubs *targetPipeInnerTub =
                new G4Tubs("TargetPipeInner_Tub", 0, targetPipeIRadius,
                           targetPipeLength / 2. - endplateThickness / 2., 0, 360 * deg);
            G4Tubs *separatorPipeInnerTub =
                new G4Tubs("SeparatorPipeInner_Tub", 0, separatorPipeIRadius,
                           separatorPipeLength / 2. + endplateThickness / 2. + booleanGeomTolerance,
                           0, 360 * deg);

            // endplateThickness / 2. cancels out in this calculation
            G4ThreeVector separatorInnerPipeTrans = {
                0, 0, targetPipeLength / 2. + separatorPipeLength / 2. - booleanGeomTolerance};
            G4VSolid *targetSeparatorPipeInnerSolid =
                new G4UnionSolid("TargetSeparatorPipeInner_Solid", targetPipeInnerTub,
                                 separatorPipeInnerTub, nullptr, separatorInnerPipeTrans);

            G4LogicalVolume *targetSeparatorInnerLV = new G4LogicalVolume(
                targetSeparatorPipeInnerSolid, vacuumMat, "TargetSeparatorPipeInner_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -endplateThickness / 2.),
                              targetSeparatorInnerLV, "TargetSeparatorPipeInner_PV",
                              targetSeparatorPipeLV, false, 0, true);

            new G4PVPlacement(
                nullptr,
                G4ThreeVector(0, 0,
                              -targetPipeLength / 2. + endplateThickness / 2. + targetLength / 2.),
                targetLV, "Target_PV", targetSeparatorInnerLV, false, 0, true);

            // Step 4: Making a ceramic insulator
            G4Tubs *ceramicInsulatorTub =
                new G4Tubs("CeramicInsulator_Tub", 0, ceramicInsulatorORadius,
                           ceramicInsulatorLength / 2., 0, 360 * deg);
            G4LogicalVolume *ceramicInsulatorLV =
                new G4LogicalVolume(ceramicInsulatorTub, cermMat, "CeramicInsulator_LV");

            G4Tubs *ceramicInsulatorInnerTub =
                new G4Tubs("CeramicInsulatorInner_Tub", 0, ceramicInsulatorIRadius,
                           ceramicInsulatorLength / 2., 0, 360 * deg);
            G4LogicalVolume *ceramicInsulatorInnerLV = new G4LogicalVolume(
                ceramicInsulatorInnerTub, vacuumMat, "CeramicInsulatorInner_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), ceramicInsulatorInnerLV,
                              "CeramicInsulatorInner_PV", ceramicInsulatorLV, false, 0, true);

            G4Tubs *ceramicInsulatorEndplateTub =
                new G4Tubs("CeramicInsulatorEndplate_Tub", separatorPipeIRadius,
                           ceramicInsulatorIRadius, endplateThickness / 2., 0, 360 * deg);
            G4LogicalVolume *ceramicInsulatorEndplateLV = new G4LogicalVolume(
                ceramicInsulatorEndplateTub, susMat, "CeramicInsulatorEndplate_LV");

            new G4PVPlacement(
                nullptr, G4ThreeVector(0, 0, -ceramicInsulatorLength / 2. + endplateThickness / 2.),
                ceramicInsulatorEndplateLV, "CeramicInsulatorEndplate_PV", ceramicInsulatorInnerLV,
                true, 0, true);
            new G4PVPlacement(
                nullptr, G4ThreeVector(0, 0, ceramicInsulatorLength / 2. - endplateThickness / 2.),
                ceramicInsulatorEndplateLV, "CeramicInsulatorEndplate_PV", ceramicInsulatorInnerLV,
                true, 1, true);

            // Step 5: Build and place entire geometries
            G4double totalRadius = std::max({plungeDiscRadius, targetPipeORadius,
                                             separatorPipeORadius, ceramicInsulatorORadius});
            G4double totalLength = boltHeadThickness + plungeDiscThickness + targetPipeLength +
                                   separatorPipeLength + ceramicInsulatorLength;

            G4Tubs *beamPipeEnvelopeTub =
                new G4Tubs("BeamPipeEnvelope_Tub", 0, totalRadius, totalLength / 2., 0, 360 * deg);
            G4LogicalVolume *beamPipeEnvelopeLV =
                new G4LogicalVolume(beamPipeEnvelopeTub, matAir, "BeamPipeEnvelope_LV");

            beamPipeEnvelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

            G4ThreeVector nowBoltPos = {plungeDiscRadius - boltHeadRadius - boltPositionMargin, 0,
                                        -totalLength / 2. + boltHeadThickness / 2.};
            for (int i = 0; i < 6; i++) {
                new G4PVPlacement(nullptr, nowBoltPos, boltLV, "Bolt_PV", beamPipeEnvelopeLV, true,
                                  i, true);
                nowBoltPos.rotateZ(60 * deg);
            }

            new G4PVPlacement(
                nullptr,
                G4ThreeVector(0, 0,
                              -totalLength / 2. + boltHeadThickness + plungeDiscThickness / 2.),
                plungeLV, "Plunge_PV", beamPipeEnvelopeLV, false, 0, true);

            new G4PVPlacement(nullptr,
                              G4ThreeVector(0, 0,
                                            -totalLength / 2. + boltHeadThickness +
                                                plungeDiscThickness + targetPipeLength / 2.),
                              targetSeparatorPipeLV, "TargetSeparatorPipe_PV", beamPipeEnvelopeLV,
                              false, 0, true);

            new G4PVPlacement(nullptr,
                              G4ThreeVector(0, 0,
                                            -totalLength / 2. + boltHeadThickness +
                                                plungeDiscThickness + targetPipeLength +
                                                separatorPipeLength + ceramicInsulatorLength / 2.),
                              ceramicInsulatorLV, "CeramicInsulator_PV", beamPipeEnvelopeLV, false,
                              0, true);

            return beamPipeEnvelopeLV;
        } else {
            // Simple geometry
            G4double plungeRadius = 2.5 * cm;
            G4double plungeLength = 5.0 * cm;

            G4double plungeRThickness = 0.2 * cm;
            G4double plungeZThickness = 0.2 * cm;

            G4Tubs *plungeTube =
                new G4Tubs("Plunge_Tub", 0, plungeRadius, plungeLength / 2., 0, 360 * deg);
            G4Tubs *plungeInnerTube =
                new G4Tubs("Plunge_InnerTub", 0, plungeRadius - plungeRThickness,
                           plungeLength / 2 - plungeZThickness, 0, 360 * deg);

            G4LogicalVolume *plungeLV = new G4LogicalVolume(plungeTube, plungeMat, "Plunge_LV");
            G4LogicalVolume *plungeInnerLV =
                new G4LogicalVolume(plungeInnerTube, vacuumMat, "PlungeInner_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), plungeInnerLV, "PlungeInner_PV",
                              plungeLV, false, 0, true);
            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), targetLV, "Target_PV", plungeInnerLV,
                              false, 0, true);

            return plungeLV;
        }
    }

    G4VPhysicalVolume *DetectorConstruction::DefineVolumes() {
        G4Material *matAir       = G4Material::GetMaterial("G4_AIR");
        G4Material *matConcrete  = G4Material::GetMaterial("G4_CONCRETE");
        G4Material *matJackPlate = G4Material::GetMaterial("Stainless_Steel");

        // Sizes of the principal geometrical components (solids)

        G4double worldLength       = 15 * m;
        G4double concreteThickness = 4 * m;

        G4double distanceFromWall  = 2 * m;
        G4double distanceFromFloor = 1 * m;

        // World

        G4GeometryManager::GetInstance()->SetWorldMaximumExtent(worldLength);

        G4cout << "Computed tolerance = "
               << G4GeometryTolerance::GetInstance()->GetSurfaceTolerance() / mm << " mm" << G4endl;

        G4Box *worldS            = new G4Box("world", // its name
                                             worldLength / 2, worldLength / 2, worldLength / 2); // its size
        G4LogicalVolume *worldLV = new G4LogicalVolume(worldS,   // its solid
                                                       matAir,   // its material
                                                       "World"); // its name
        worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        //  Must place the World Physical volume unrotated at (0,0,0).
        //
        G4PVPlacement *worldPV = new G4PVPlacement(nullptr,         // no rotation
                                                   G4ThreeVector(), // at (0,0,0)
                                                   worldLV,         // its logical volume
                                                   "World",         // its name
                                                   nullptr,         // its mother  volume
                                                   false,           // no boolean operations
                                                   0,               // copy number
                                                   true);           // checking overlaps

        // Basic parameters for the PE blocks
        G4double peWidth  = 10 * cm;
        G4double peHeight = 5 * cm;
        G4double peLength = 20 * cm;

        // Basic parameters for the placements
        G4double rearFrontZSpacing        = 2.5 * cm;
        G4double beampipeDistance         = 0.4 * cm;
        G4double rearFrontYDiff           = 5 * cm;
        G4double frontBlockXSpacing       = 2.2 * cm;
        G4double fpgaXDistance            = -7.2 * cm;
        G4double rearIncidentTopXBias     = 1.5 * cm;
        G4double cr39DistanceFromBeampipe = 10 * cm;
        G4double rearIncidentBottomXBias  = -(peLength / 2. - peWidth / 2.);

        G4Material *variableBlockMat, *vacuumMat;

        if (fUseBoratedPE) {
            variableBlockMat = G4Material::GetMaterial("Borated_PE");
        } else {
            variableBlockMat = G4Material::GetMaterial("G4_POLYETHYLENE");
        }

        vacuumMat = G4Material::GetMaterial("G4_Galactic");

        // Front PE Blocks
        G4double frontJackPlateThickness = 3 * mm;
        G4double frontJackPlateWidth     = peLength;

        G4double frontHeadXBias = 0 * cm;

        G4Material *frontBodyMat = variableBlockMat;
        G4Material *frontHeadMat = G4Material::GetMaterial("G4_POLYETHYLENE");

        G4double frontStructureXSize = 2 * peWidth + frontBlockXSpacing;
        G4double frontStructureYSize = peHeight * 3 + frontJackPlateThickness;
        G4double frontStructureZSize = peLength;

        G4Box *frontStructureBox = new G4Box("FrontStructure_Box", frontStructureXSize / 2.,
                                             frontStructureYSize / 2., frontStructureZSize / 2.);

        G4LogicalVolume *frontStructureLV =
            new G4LogicalVolume(frontStructureBox, matAir, "FrontStructure_LV");
        frontStructureLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4Box *frontBodyBox =
            new G4Box("FrontBlockBody_Box", peWidth / 2., peHeight / 2. * 2., peLength / 2.);
        G4Box *frontHeadBox =
            new G4Box("FrontBlockHead_Box", peWidth / 2. * 2, peHeight / 2., peLength / 2.);

        G4LogicalVolume *frontBodyLV =
            new G4LogicalVolume(frontBodyBox, frontBodyMat, "FrontBlockBody_LV");
        G4LogicalVolume *frontHeadLV =
            new G4LogicalVolume(frontHeadBox, frontHeadMat, "FrontBlockHead_LV");

        G4double frontBody1XPos = frontBlockXSpacing / 2. + peWidth / 2.;
        G4double frontBody2XPos = -frontBlockXSpacing / 2. - peWidth / 2.;
        G4double frontBodyYPos =
            -frontStructureYSize / 2. + frontJackPlateThickness + peHeight / 2. * 2;

        new G4PVPlacement(nullptr, G4ThreeVector(frontBody1XPos, frontBodyYPos, 0), frontBodyLV,
                          "FrontBlockBody_1_PV", frontStructureLV, false, 0, true);
        new G4PVPlacement(nullptr, G4ThreeVector(frontBody2XPos, frontBodyYPos, 0), frontBodyLV,
                          "FrontBlockBody_2_PV", frontStructureLV, false, 1, true);

        G4double frontHeadXPos = frontHeadXBias;
        G4double frontHeadYPos = frontStructureYSize / 2. - peHeight / 2.;

        new G4PVPlacement(nullptr, G4ThreeVector(frontHeadXPos, frontHeadYPos, 0), frontHeadLV,
                          "FrontBlockHead_PV", frontStructureLV, false, 0, true);

        G4Box *frontJackPlateBox =
            new G4Box("FrontJackPlate_Box", frontJackPlateWidth / 2., frontJackPlateThickness / 2.,
                      frontJackPlateWidth / 2.);

        G4LogicalVolume *frontJackPlateLV =
            new G4LogicalVolume(frontJackPlateBox, matJackPlate, "FrontJackPlate_LV");

        G4double frontJackPlateYPos = -frontStructureYSize / 2. + frontJackPlateThickness / 2.;

        new G4PVPlacement(nullptr, G4ThreeVector(0, frontJackPlateYPos, 0), frontJackPlateLV,
                          "FrontJackPlate_PV", frontStructureLV, false, 0, true);

        // Rear PE Blocks
        G4Material *rearIncidentMat = variableBlockMat;
        G4Material *rearVolumeMat   = G4Material::GetMaterial("G4_POLYETHYLENE");

        G4double rearJackPlateThickness = 3 * mm;
        G4double rearJackPlateWidth     = 25 * cm;

        G4double rearStructureXSize = rearJackPlateWidth;
        G4double rearStructureYSize = 2 * peWidth + 2 * peHeight + rearJackPlateThickness;
        G4double rearStructureZSize = peLength + peWidth;

        G4double rearStructureXPos = 0;
        G4double rearStructureYPos = frontStructureYSize / 2. - rearStructureYSize / 2.;
        G4double rearStructureZPos = -rearFrontZSpacing / 2. - rearStructureZSize / 2.;

        G4Box *rearStructureBox = new G4Box("RearStructure_Box", rearStructureXSize / 2.,
                                            rearStructureYSize / 2., rearStructureZSize / 2.);

        G4Box *rearIncidentBox =
            new G4Box("RearIncident_Box", peLength / 2., peWidth / 2., peHeight / 2.);

        G4LogicalVolume *rearIncidentSurfaceLV =
            new G4LogicalVolume(rearIncidentBox, rearIncidentMat, "RearIncidentSurface_LV");
        G4LogicalVolume *rearIncidentBackLV =
            new G4LogicalVolume(rearIncidentBox, rearVolumeMat, "RearIncidentBack_LV");

        G4LogicalVolume *beamPipeLV = MakeBeampipeGeometry(fRealisticBeampipe);
        G4Tubs *beamPipeEnvelopeTub = static_cast<G4Tubs *>(beamPipeLV->GetSolid());

        // Global placement
        if (fGeometryType == "pe_blocks" || fGeometryType == "pe_blocks_cr39") {
            worldLV->SetMaterial(matConcrete);
            G4Box *expHallBox = new G4Box(
                "ExperimentalHall_Box", worldLength / 2. - concreteThickness,
                worldLength / 2. - concreteThickness, worldLength / 2. - concreteThickness);
            G4LogicalVolume *expHallLV =
                new G4LogicalVolume(expHallBox, matAir, "ExperimentalHall_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), expHallLV, "ExperimentalHall_PV",
                              worldLV, false, 0, true);

            G4ThreeVector globalTranslation = {
                0, -worldLength / 2. + concreteThickness + distanceFromFloor,
                -worldLength / 2. + concreteThickness + distanceFromWall};

            G4double rearIncidentTopSurfaceXPos = rearStructureXPos + rearIncidentTopXBias;
            G4double rearIncidentTopSurfaceYPos =
                rearStructureYSize / 2. - peWidth / 2. + rearStructureYPos;
            G4double rearIncidentTopSurfaceZPos =
                rearStructureZSize / 2. - peHeight / 2. + rearStructureZPos;

            G4double rearIncidentBottomSurfaceXPos = rearStructureXPos + rearIncidentBottomXBias;
            G4double rearIncidentBottomSurfaceYPos =
                rearStructureYSize / 2. - 3 * peWidth / 2. + rearStructureYPos;
            G4double rearIncidentBottomSurfaceZPos =
                rearStructureZSize / 2. - peHeight / 2. + rearStructureZPos;

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(rearIncidentTopSurfaceXPos,
                                                                rearIncidentTopSurfaceYPos,
                                                                rearIncidentTopSurfaceZPos),
                              rearIncidentSurfaceLV, "RearIncidentTopSurface_PV", expHallLV, false,
                              0, true);

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(rearIncidentBottomSurfaceXPos,
                                                                rearIncidentBottomSurfaceYPos,
                                                                rearIncidentBottomSurfaceZPos),
                              rearIncidentSurfaceLV, "RearIncidentBottomSurface_PV", expHallLV,
                              false, 0, true);

            G4double rearIncidentTopBackXPos = rearStructureXPos + rearIncidentTopXBias;
            G4double rearIncidentTopBackYPos =
                rearStructureYSize / 2. - peWidth / 2. + rearStructureYPos;
            G4double rearIncidentTopBackZPos = rearIncidentTopSurfaceZPos - peHeight;

            G4double rearIncidentBottomBackXPos = rearStructureXPos + rearIncidentBottomXBias;
            G4double rearIncidentBottomBackYPos =
                rearStructureYSize / 2. - 3 * peWidth / 2. + rearStructureYPos;
            G4double rearIncidentBottomBackZPos = rearIncidentBottomSurfaceZPos - peHeight;

            new G4PVPlacement(
                nullptr,
                globalTranslation + G4ThreeVector(rearIncidentTopBackXPos, rearIncidentTopBackYPos,
                                                  rearIncidentTopBackZPos),
                rearIncidentBackLV, "RearIncidentTopBack_PV", expHallLV, false, 0, true);

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(rearIncidentBottomBackXPos,
                                                                rearIncidentBottomBackYPos,
                                                                rearIncidentBottomBackZPos),
                              rearIncidentBackLV, "RearIncidentBottomBack_PV", expHallLV, false, 0,
                              true);

            G4Box *rearIncidentBasisBox =
                new G4Box("RearIncidentBasis_Box", peLength / 2., peHeight / 2. * 2, peWidth / 2.);

            G4LogicalVolume *rearIncidentBasisLV =
                new G4LogicalVolume(rearIncidentBasisBox, rearVolumeMat, "RearIncidentBasis_LV");

            G4double rearIncidentBasisXPos = rearStructureXPos + rearIncidentBottomXBias;
            G4double rearIncidentBasisYPos =
                -rearStructureYSize / 2. + rearJackPlateThickness + peHeight + rearStructureYPos;
            G4double rearIncidentBasisZPos =
                rearStructureZSize / 2. - peWidth / 2. + rearStructureZPos;

            new G4PVPlacement(
                nullptr,
                globalTranslation + G4ThreeVector(rearIncidentBasisXPos, rearIncidentBasisYPos,
                                                  rearIncidentBasisZPos),
                rearIncidentBasisLV, "RearIncidentBasis_PV", expHallLV, false, 0, true);

            G4double rearBackSizeX = peWidth;
            G4double rearBackSizeY = peWidth * 2 + peHeight;
            G4double rearBackSizeZ = peLength;

            G4Box *rearBackBox = new G4Box("RearBack_Box", rearBackSizeX / 2., rearBackSizeY / 2.,
                                           rearBackSizeZ / 2.);

            G4LogicalVolume *rearBackLV =
                new G4LogicalVolume(rearBackBox, rearVolumeMat, "RearBack_LV");

            G4double rearBackXPos = rearStructureXPos;
            G4double rearBackYPos = -rearStructureYSize / 2. + rearJackPlateThickness +
                                    rearBackSizeY / 2. + rearStructureYPos;
            G4double rearBackZPos =
                -rearStructureZSize / 2. + rearBackSizeZ / 2. + rearStructureZPos;

            new G4PVPlacement(nullptr,
                              globalTranslation +
                                  G4ThreeVector(rearBackXPos, rearBackYPos, rearBackZPos),
                              rearBackLV, "RearBack_PV", expHallLV, false, 0, true);

            G4Box *rearJackPlateBox =
                new G4Box("RearJackPlate_Box", rearJackPlateWidth / 2., rearJackPlateThickness / 2.,
                          rearJackPlateWidth / 2.);

            G4LogicalVolume *rearJackPlateLV =
                new G4LogicalVolume(rearJackPlateBox, matJackPlate, "RearJackPlate_LV");

            G4double rearJackPlateXPos = rearStructureXPos;
            G4double rearJackPlateYPos =
                -rearStructureYSize / 2. + rearJackPlateThickness / 2. + rearStructureYPos;
            G4double rearJackPlateZPos = rearStructureZPos;

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(rearJackPlateXPos,
                                                                rearJackPlateYPos,
                                                                rearJackPlateZPos),
                              rearJackPlateLV, "RearJackPlate_PV", expHallLV, false, 0, true);

            if (fGeometryType == "pe_blocks") {
                // FPGA
                G4double fpgaWidth     = 1.20 * cm;
                G4double fpgaThickness = 2.00 * mm;

                // G4Material *fpgaMat = G4Material::GetMaterial("G4_Si");
                G4Material *fpgaMat = G4Material::GetMaterial("G4_POLYETHYLENE");

                G4double endpointThickness = 0.5 * cm;
                G4Box *endpointBox         = new G4Box("Terminal_Box", endpointThickness / 2.,
                                                       peHeight * 3 / 2, rearFrontZSpacing / 2.);

                G4LogicalVolume *endpointLV =
                    new G4LogicalVolume(endpointBox, rearVolumeMat, "Terminal_LV");

                G4Box *fpgaBox =
                    new G4Box("FPGA_Box", fpgaWidth / 2., fpgaWidth / 2., fpgaThickness / 2.);

                G4LogicalVolume *fpgaLV = new G4LogicalVolume(fpgaBox, fpgaMat, "Detector_LV");

                new G4PVPlacement(nullptr,
                                  globalTranslation +
                                      G4ThreeVector(fpgaXDistance, 0,
                                                    rearFrontZSpacing / 2. - fpgaThickness / 2.),
                                  fpgaLV, "FPGA_PV", expHallLV, false, 0, true);
            } else if (fGeometryType == "pe_blocks_cr39") {
                G4LogicalVolume *dosimeterLV = MakeDosimeter();
                G4Box *dosimeterBox          = static_cast<G4Box *>(dosimeterLV->GetSolid());

                G4double dosimeterThickness = dosimeterBox->GetZHalfLength() * 2.;

                new G4PVPlacement(nullptr,
                                  globalTranslation + G4ThreeVector(fpgaXDistance, 0,
                                                                    rearFrontZSpacing / 2. -
                                                                        dosimeterThickness / 2.),
                                  dosimeterLV, "Dosimeter_PV", expHallLV, false, 0, true);
            }

            G4double beampipeXPos = 0;
            G4double beampipeYPos = 0;
            G4double beampipeZPos = rearFrontZSpacing / 2. + frontStructureZSize +
                                    beampipeDistance + beamPipeEnvelopeTub->GetZHalfLength();

            new G4PVPlacement(nullptr,
                              globalTranslation +
                                  G4ThreeVector(beampipeXPos, beampipeYPos, beampipeZPos),
                              beamPipeLV, "BeamPipe_PV", expHallLV, false, 0, true);

            G4double frontStructureXPos = 0;
            G4double frontStructureYPos = 0;
            G4double frontStructureZPos = rearFrontZSpacing / 2. + frontStructureZSize / 2.;

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(frontStructureXPos,
                                                                frontStructureYPos,
                                                                frontStructureZPos),
                              frontStructureLV, "FrontStructure_PV", expHallLV, false, 0, true);

            // Visualization attributes
            G4Colour myWhite = G4Colour::White();
            G4Colour myRed   = G4Colour::Red();
            myWhite.SetAlpha(0.8);
            myRed.SetAlpha(0.8);

            G4VisAttributes peBoxVisAtt(myWhite);
            G4VisAttributes bpeBoxVisAtt(myRed);
            G4VisAttributes metalVisAtt(G4Colour::Gray());

            frontBodyLV->SetVisAttributes(bpeBoxVisAtt);
            frontHeadLV->SetVisAttributes(peBoxVisAtt);
            frontJackPlateLV->SetVisAttributes(metalVisAtt);

            rearIncidentSurfaceLV->SetVisAttributes(bpeBoxVisAtt);
            rearIncidentBackLV->SetVisAttributes(peBoxVisAtt);
            rearIncidentBasisLV->SetVisAttributes(peBoxVisAtt);
            rearBackLV->SetVisAttributes(peBoxVisAtt);
            rearJackPlateLV->SetVisAttributes(metalVisAtt);
        } else if (fGeometryType == "sphere_det") {
            G4double detectorDistance  = 20 * cm;
            G4double detectorThickness = 1 * mm;
            G4Sphere *detectorSphere =
                new G4Sphere("Detector_Sphere", detectorDistance,
                             detectorDistance + detectorThickness, 0, 360 * deg, 0, 180 * deg);

            G4LogicalVolume *fpgaLV = new G4LogicalVolume(detectorSphere, vacuumMat, "Detector_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), fpgaLV, "FPGA_PV", worldLV, false, 0,
                              true);

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), beamPipeLV, "BeamPipe_PV", worldLV,
                              false, 0, true);
        } else if (fGeometryType == "hemisphere_det") {
            G4double detectorDistance  = 20 * cm;
            G4double detectorThickness = 1 * mm;
            G4Sphere *detectorSphere =
                new G4Sphere("Detector_Sphere", detectorDistance,
                             detectorDistance + detectorThickness, 0, 180 * deg, 0, 180 * deg);

            G4LogicalVolume *fpgaLV = new G4LogicalVolume(detectorSphere, vacuumMat, "Detector_LV");

            G4RotationMatrix *detectorRotMtx = new G4RotationMatrix();
            detectorRotMtx->rotateX(90 * deg);
            G4AutoDelete::Register(detectorRotMtx);

            new G4PVPlacement(detectorRotMtx, G4ThreeVector(0, 0, 0), fpgaLV, "FPGA_PV", worldLV,
                              false, 0, true);

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), beamPipeLV, "BeamPipe_PV", worldLV,
                              false, 0, true);
        } else if (fGeometryType == "cr39") {
            worldLV->SetMaterial(matConcrete);
            G4Box *expHallBox = new G4Box(
                "ExperimentalHall_Box", worldLength / 2. - concreteThickness,
                worldLength / 2. - concreteThickness, worldLength / 2. - concreteThickness);
            G4LogicalVolume *expHallLV =
                new G4LogicalVolume(expHallBox, matAir, "ExperimentalHall_LV");

            new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), expHallLV, "ExperimentalHall_PV",
                              worldLV, false, 0, true);

            G4ThreeVector globalTranslation = {
                0, -worldLength / 2. + concreteThickness + distanceFromFloor,
                -worldLength / 2. + concreteThickness + distanceFromWall};

            G4LogicalVolume *dosimeterLV = MakeDosimeter();
            G4Box *dosimeterBox          = static_cast<G4Box *>(dosimeterLV->GetSolid());

            G4double dosimeterThickness = dosimeterBox->GetZHalfLength() * 2.;

            new G4PVPlacement(nullptr,
                              globalTranslation + G4ThreeVector(0, 0, -dosimeterThickness / 2.),
                              dosimeterLV, "Dosimeter_PV", expHallLV, false, 0, true);

            G4double beampipeXPos = 0;
            G4double beampipeYPos = 0;
            G4double beampipeZPos =
                cr39DistanceFromBeampipe + beamPipeEnvelopeTub->GetZHalfLength();

            new G4PVPlacement(nullptr,
                              globalTranslation +
                                  G4ThreeVector(beampipeXPos, beampipeYPos, beampipeZPos),
                              beamPipeLV, "BeamPipe_PV", expHallLV, false, 0, true);
        } else {
            G4Exception("DetectorConstruction", "1", G4ExceptionSeverity::FatalException,
                        "Not supported geometry type.");
        }

        return worldPV;
    }

    void DetectorConstruction::ConstructSDandField() {
        G4String fpgaChamberSDname = "/FPGASD";

        FPGASD *fpgaSD = new FPGASD(fpgaChamberSDname);

        G4SDManager::GetSDMpointer()->AddNewDetector(fpgaSD);
        SetSensitiveDetector("Target_LV", fpgaSD, true);
    }
} // namespace kobetdsim
