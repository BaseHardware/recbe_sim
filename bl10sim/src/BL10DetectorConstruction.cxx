#include "bl10sim/BL10DetectorConstruction.h"

#include "G4Box.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

namespace bl10sim {
    void BL10DetectorConstruction::DefineMaterials() {
        DetectorConstruction::DefineMaterials();

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
    G4VPhysicalVolume *BL10DetectorConstruction::DefineVolumes() {
        G4double worldSizeXY = 4 * m;
        G4double worldSizeZ  = 4 * m;

        // Get materials
        G4Material *defaultMaterial = G4Material::GetMaterial("G4_AIR");

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

    void BL10DetectorConstruction::ConstructSDandField() {}
} // namespace bl10sim
