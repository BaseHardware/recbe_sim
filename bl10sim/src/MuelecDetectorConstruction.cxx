#include "bl10sim/MuelecDetectorConstruction.h"

#include "simcore/MetadataManager.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4SystemOfUnits.hh"

namespace bl10sim {
    MuelecDetectorConstruction::MuelecDetectorConstruction() {
        fTargetRegion = new G4Region("TargetRegion");

        G4ProductionCuts *cuts = new G4ProductionCuts();

        // G4double defCut = 1 * nanometer;
        // cuts->SetProductionCut(defCut, "gamma");
        // cuts->SetProductionCut(defCut, "e-");
        // cuts->SetProductionCut(defCut, "e+");
        // cuts->SetProductionCut(defCut, "proton");

        fTargetRegion->SetProductionCuts(cuts);
    };

    MuelecDetectorConstruction::~MuelecDetectorConstruction() { delete fTargetRegion; };

    void MuelecDetectorConstruction::DefineMaterials() {
        DetectorConstruction::DefineMaterials();

        G4NistManager *instance = G4NistManager::Instance();

        instance->FindOrBuildMaterial("G4_Si");
        instance->FindOrBuildMaterial("G4_Fe");
    }

    G4VPhysicalVolume *MuelecDetectorConstruction::DefineVolumes() {
        G4double boxSizeXYZ = 200 * um;

        G4double worldSizeXY = 1 * mm;
        G4double worldSizeZ  = 1 * mm;

        // Get materials
        G4Material *vacMaterial = G4Material::GetMaterial("G4_Galactic");
        G4Material *boxMaterial = G4Material::GetMaterial("G4_Si");

        vacMaterial->SetName("Vacuum");

        G4Box *worldS = new G4Box("World", worldSizeXY / 2., worldSizeXY / 2., worldSizeZ / 2);

        G4LogicalVolume *worldLV = new G4LogicalVolume(worldS,      // its solid
                                                       vacMaterial, // its material
                                                       "World");    // its name

        G4VPhysicalVolume *worldPV = new G4PVPlacement(nullptr,         // no rotation
                                                       G4ThreeVector(), // at (0,0,0)
                                                       worldLV,         // its logical volume
                                                       "World",         // its name
                                                       nullptr,         // its mother  volume
                                                       false,           // no boolean operation
                                                       0,               // copy number
                                                       fCheckOverlaps); // checking overlaps
        G4Box *targetBox = new G4Box("Target", boxSizeXYZ / 2., boxSizeXYZ / 2., boxSizeXYZ / 2.);
        G4LogicalVolume *targetLV = new G4LogicalVolume(targetBox, boxMaterial, "TargetLV");

        fTargetRegion->AddRootLogicalVolume(targetLV);

        new G4PVPlacement(nullptr, {}, targetLV, "target", worldLV, false, 0, fCheckOverlaps);

        G4double ductWindowXY    = 1 * mm;
        G4double neutWindowThick = 1 * nm;

        G4Box *nWindowBox = new G4Box("NeutronWindowBox", ductWindowXY / 2., ductWindowXY / 2.,
                                      neutWindowThick / 2.);
        G4LogicalVolume *nWindowLV =
            new G4LogicalVolume(nWindowBox, vacMaterial, "NeutronWindowLV");
        new G4PVPlacement(nullptr, {0, 0, -worldSizeZ / 2. + neutWindowThick / 2.}, nWindowLV,
                          "BeamWindowPV", worldLV, false, 0, fCheckOverlaps);

        simcore::MetadataManager::GetInstance().SetGeometryType("bl10_muelec");

        return worldPV;
    }
} // namespace bl10sim
