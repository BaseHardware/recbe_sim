#include "DetectorConstruction.hh"

#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

namespace recbesim {
    G4ThreadLocal G4GlobalMagFieldMessenger *DetectorConstruction::fMagFieldMessenger = nullptr;

    G4VPhysicalVolume *DetectorConstruction::Construct() {
        // Define materials
        DefineMaterials();

        // Define volumes
        return DefineVolumes();
    }

    void DetectorConstruction::DefineMaterials() {
        // Lead material defined using NIST Manager
        auto nistManager = G4NistManager::Instance();
        nistManager->FindOrBuildMaterial("G4_Pb");

        // Liquid argon material
        G4double a; // mass of a mole;
        G4double z; // z=mean number of protons;
        G4double density;
        new G4Material("liquidArgon", z = 18., a = 39.95 * g / mole, density = 1.390 * g / cm3);
        // The argon by NIST Manager is a gas with a different density

        // Vacuum
        new G4Material("Galactic", z = 1., a = 1.01 * g / mole, density = universe_mean_density,
                       kStateGas, 2.73 * kelvin, 3.e-18 * pascal);

        // Print materials
        G4cout << *(G4Material::GetMaterialTable()) << G4endl;
    }

    G4VPhysicalVolume *DetectorConstruction::DefineVolumes() {
        // Geometry parameters
        G4int nofLayers        = 10;
        G4double absoThickness = 10. * mm;
        G4double gapThickness  = 5. * mm;
        G4double calorSizeXY   = 10. * cm;

        auto layerThickness = absoThickness + gapThickness;
        auto calorThickness = nofLayers * layerThickness;
        auto worldSizeXY    = 1.2 * calorSizeXY;
        auto worldSizeZ     = 1.2 * calorThickness;

        // Get materials
        auto defaultMaterial  = G4Material::GetMaterial("Galactic");
        auto absorberMaterial = G4Material::GetMaterial("G4_Pb");
        auto gapMaterial      = G4Material::GetMaterial("liquidArgon");

        if (!defaultMaterial || !absorberMaterial || !gapMaterial) {
            G4ExceptionDescription msg;
            msg << "Cannot retrieve materials already defined.";
            G4Exception("DetectorConstruction::DefineVolumes()", "MyCode0001", FatalException, msg);
        }

        //
        // World
        //
        auto worldS = new G4Box("World",                                           // its name
                                worldSizeXY / 2, worldSizeXY / 2, worldSizeZ / 2); // its size

        auto worldLV = new G4LogicalVolume(worldS,          // its solid
                                           defaultMaterial, // its material
                                           "World");        // its name

        auto worldPV = new G4PVPlacement(nullptr,         // no rotation
                                         G4ThreeVector(), // at (0,0,0)
                                         worldLV,         // its logical volume
                                         "World",         // its name
                                         nullptr,         // its mother  volume
                                         false,           // no boolean operation
                                         0,               // copy number
                                         fCheckOverlaps); // checking overlaps

        //
        // Calorimeter
        //
        auto calorimeterS =
            new G4Box("Calorimeter",                                         // its name
                      calorSizeXY / 2, calorSizeXY / 2, calorThickness / 2); // its size

        auto calorLV = new G4LogicalVolume(calorimeterS,    // its solid
                                           defaultMaterial, // its material
                                           "Calorimeter");  // its name

        new G4PVPlacement(nullptr,         // no rotation
                          G4ThreeVector(), // at (0,0,0)
                          calorLV,         // its logical volume
                          "Calorimeter",   // its name
                          worldLV,         // its mother  volume
                          false,           // no boolean operation
                          0,               // copy number
                          fCheckOverlaps); // checking overlaps

        //
        // Layer
        //
        auto layerS = new G4Box("Layer",                                               // its name
                                calorSizeXY / 2, calorSizeXY / 2, layerThickness / 2); // its size

        auto layerLV = new G4LogicalVolume(layerS,          // its solid
                                           defaultMaterial, // its material
                                           "Layer");        // its name

        new G4PVReplica("Layer",         // its name
                        layerLV,         // its logical volume
                        calorLV,         // its mother
                        kZAxis,          // axis of replication
                        nofLayers,       // number of replica
                        layerThickness); // witdth of replica

        //
        // Absorber
        //
        auto absorberS = new G4Box("Abso",                                               // its name
                                   calorSizeXY / 2, calorSizeXY / 2, absoThickness / 2); // its size

        auto absorberLV = new G4LogicalVolume(absorberS,        // its solid
                                              absorberMaterial, // its material
                                              "Abso");          // its name

        fAbsorberPV = new G4PVPlacement(nullptr,                                  // no rotation
                                        G4ThreeVector(0., 0., -gapThickness / 2), // its position
                                        absorberLV,      // its logical volume
                                        "Abso",          // its name
                                        layerLV,         // its mother  volume
                                        false,           // no boolean operation
                                        0,               // copy number
                                        fCheckOverlaps); // checking overlaps

        //
        // Gap
        //
        auto gapS = new G4Box("Gap",                                               // its name
                              calorSizeXY / 2, calorSizeXY / 2, gapThickness / 2); // its size

        auto gapLV = new G4LogicalVolume(gapS,        // its solid
                                         gapMaterial, // its material
                                         "Gap");      // its name

        fGapPV = new G4PVPlacement(nullptr,                                  // no rotation
                                   G4ThreeVector(0., 0., absoThickness / 2), // its position
                                   gapLV,                                    // its logical volume
                                   "Gap",                                    // its name
                                   layerLV,                                  // its mother  volume
                                   false,                                    // no boolean operation
                                   0,                                        // copy number
                                   fCheckOverlaps);                          // checking overlaps

        //
        // print parameters
        //
        G4cout << G4endl << "------------------------------------------------------------" << G4endl
               << "---> The calorimeter is " << nofLayers << " layers of: [ " << absoThickness / mm
               << "mm of " << absorberMaterial->GetName() << " + " << gapThickness / mm << "mm of "
               << gapMaterial->GetName() << " ] " << G4endl
               << "------------------------------------------------------------" << G4endl;

        //
        // Visualization attributes
        //
        worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());
        calorLV->SetVisAttributes(G4VisAttributes(G4Colour::White()));

        //
        // Always return the physical World
        //
        return worldPV;
    }

    void DetectorConstruction::ConstructSDandField() {
        // Create global magnetic field messenger.
        // Uniform magnetic field is then created automatically if
        // the field value is not zero.
        G4ThreeVector fieldValue;
        fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
        fMagFieldMessenger->SetVerboseLevel(1);

        // Register the field messenger for deleting
        G4AutoDelete::Register(fMagFieldMessenger);
    }
} // namespace recbesim
