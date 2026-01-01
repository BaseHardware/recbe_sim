#include "bl10sim/DuctDetectorConstruction.h"

#include "simcore/TouchTriggerSD.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

namespace bl10sim {
    void DuctDetectorConstruction::DefineMaterials() {
        DetectorConstruction::DefineMaterials();

        G4NistManager *instance = G4NistManager::Instance();

        G4Element *gd = instance->FindOrBuildElement("Gd");
        G4Element *o  = instance->FindOrBuildElement("O");

        G4Material *Gd2O3 =
            new G4Material("GADOLINIUM_OXIDE", 7.41 * g / cm3, 2, G4State::kStateSolid,
                           NTP_Temperature, CLHEP::STP_Pressure);
        Gd2O3->AddElement(gd, 2);
        Gd2O3->AddElement(o, 3);
    }

    G4VPhysicalVolume *DuctDetectorConstruction::DefineVolumes() {
        G4double worldSizeXY = 1 * m;
        G4double worldSizeZ  = 40 * m;

        G4double ductThick    = 3 * cm;
        G4double ductLength   = 36 * m;
        G4double ductWindowXY = 10 * cm;

        G4double neutDetectorThick = 1 * cm;

        G4double filterWindowThick = 0.5 * mm;

        // Get materials
        G4Material *airMaterial    = G4Material::GetMaterial("G4_AIR");
        G4Material *ductMaterial   = G4Material::GetMaterial("G4_CONCRETE");
        G4Material *filterMaterial = G4Material::GetMaterial("GADOLINIUM_OXIDE");

        //
        // World
        //
        G4Box *worldS = new G4Box("World",                                           // its name
                                  worldSizeXY / 2, worldSizeXY / 2, worldSizeZ / 2); // its size

        G4LogicalVolume *worldLV = new G4LogicalVolume(worldS,      // its solid
                                                       airMaterial, // its material
                                                       "World");    // its name

        G4VPhysicalVolume *worldPV = new G4PVPlacement(nullptr,         // no rotation
                                                       G4ThreeVector(), // at (0,0,0)
                                                       worldLV,         // its logical volume
                                                       "World",         // its name
                                                       nullptr,         // its mother  volume
                                                       false,           // no boolean operation
                                                       0,               // copy number
                                                       fCheckOverlaps); // checking overlaps

        G4Box *ductOuterBox = new G4Box("DuctOuterBox", ductWindowXY / 2. + ductThick,
                                        ductWindowXY / 2. + ductThick, ductLength / 2.);
        G4Box *ductInnerBox =
            new G4Box("DuctInnerBox", ductWindowXY / 2., ductWindowXY / 2., ductLength / 2.);

        G4LogicalVolume *ductOuterLV =
            new G4LogicalVolume(ductOuterBox, ductMaterial, "DuctOuterLV");
        G4LogicalVolume *ductInnerLV =
            new G4LogicalVolume(ductInnerBox, airMaterial, "DuctInnerLV");

        new G4PVPlacement(nullptr, {0, 0, 0}, ductOuterLV, "DuctOuterPV", worldLV, false, 0,
                          fCheckOverlaps);
        new G4PVPlacement(nullptr, {0, 0, 0}, ductInnerLV, "DuctInnerPV", ductOuterLV, false, 0,
                          fCheckOverlaps);

        G4Box *neutDetectorBox = new G4Box("NeutronDetectorBox", ductWindowXY / 2.,
                                           ductWindowXY / 2., neutDetectorThick / 2.);
        G4LogicalVolume *neutDetectorLV =
            new G4LogicalVolume(neutDetectorBox, airMaterial, "NeutronDetectorLV");
        new G4PVPlacement(nullptr, {0, 0, ductLength / 2. - neutDetectorThick / 2.}, neutDetectorLV,
                          "NeutronDetectorPV", ductInnerLV, false, 0, fCheckOverlaps);

        G4Box *filterBox =
            new G4Box("FilterBox", ductWindowXY / 2., ductWindowXY / 2., filterWindowThick / 2.);
        G4LogicalVolume *filterLV = new G4LogicalVolume(filterBox, filterMaterial, "FilterLV");

        for (int i = 0; i < 6; i++) {
            G4double nowFilterZPos = 1 * m + (i + 1) * 4 * m;

            G4ThreeVector nowFilterTlate{0, 0, -ductLength / 2. + nowFilterZPos};

            new G4PVPlacement(nullptr, nowFilterTlate, filterLV, "FilterPV", ductInnerLV, true, i,
                              fCheckOverlaps);
        }

        return worldPV;
    }

    void DuctDetectorConstruction::ConstructSDandField() {
        G4String neutDetectorSDName = "/NeutronSD";

        simcore::TouchTriggerSD *ndsd = new simcore::TouchTriggerSD(neutDetectorSDName);
        ndsd->SetRequireNonzeroEdep(false);

        G4SDManager::GetSDMpointer()->AddNewDetector(ndsd);
        SetSensitiveDetector("NeutronDetectorLV", ndsd, true);
    }
} // namespace bl10sim
