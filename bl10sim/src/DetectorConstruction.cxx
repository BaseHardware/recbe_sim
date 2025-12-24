#include "DetectorConstruction.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

namespace bl10sim {
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

    void DetectorConstruction::ConstructSDandField() {}
} // namespace bl10sim
