#include "DetectorConstruction.hh"

#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

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
        G4NistManager *nistManager = G4NistManager::Instance();
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
        G4double worldSizeXY = 4 * m;
        G4double worldSizeZ  = 4 * m;

        // Get materials
        G4Material *defaultMaterial = G4Material::GetMaterial("Galactic");

        //
        // World
        //
        G4Box *worldS = new G4Box("World",                                           // its name
                                  worldSizeXY / 2, worldSizeXY / 2, worldSizeZ / 2); // its size

        G4LogicalVolume *worldLV = new G4LogicalVolume(worldS,          // its solid
                                                       defaultMaterial, // its material
                                                       "World");        // its name

        G4VPhysicalVolume *worldPV = new G4PVPlacement(nullptr,         // no rotation
                                                       G4ThreeVector(), // at (0,0,0)
                                                       worldLV,         // its logical volume
                                                       "World",         // its name
                                                       nullptr,         // its mother  volume
                                                       false,           // no boolean operation
                                                       0,               // copy number
                                                       fCheckOverlaps); // checking overlaps

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
