#include "DetectorConstruction.h"

#include "G4AutoDelete.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"

namespace simcore {
    G4ThreadLocal G4GlobalMagFieldMessenger *DetectorConstruction::fMagFieldMessenger = nullptr;

    G4VPhysicalVolume *DetectorConstruction::Construct() {
        // Define materials
        DefineMaterials();

        // Define volumes
        return DefineVolumes();
    }
    void DetectorConstruction::DefineMaterials() {
        // Material definition

        G4NistManager *nistManager = G4NistManager::Instance();

        // Air defined using NIST Manager
        nistManager->FindOrBuildMaterial("G4_AIR");
        nistManager->FindOrBuildMaterial("G4_Al");
        nistManager->FindOrBuildMaterial("G4_Be");
        nistManager->FindOrBuildMaterial("G4_GRAPHITE");
        nistManager->FindOrBuildMaterial("G4_Galactic");

        G4Material *matPE = nistManager->FindOrBuildMaterial("G4_POLYETHYLENE");
        G4Material *matBO = nistManager->FindOrBuildMaterial("G4_BORON_OXIDE");
        nistManager->FindOrBuildMaterial("G4_CONCRETE");

        G4Material *matMn = nistManager->FindOrBuildMaterial("G4_Mn");
        G4Material *matSi = nistManager->FindOrBuildMaterial("G4_Si");
        G4Material *matCr = nistManager->FindOrBuildMaterial("G4_Cr");
        G4Material *matNi = nistManager->FindOrBuildMaterial("G4_Ni");
        G4Material *matFe = nistManager->FindOrBuildMaterial("G4_Fe");

        G4Material *matSUS = new G4Material("Stainless_Steel", 8.02 * g / cm3, 5);
        matSUS->AddMaterial(matMn, 0.02);
        matSUS->AddMaterial(matSi, 0.01);
        matSUS->AddMaterial(matCr, 0.19);
        matSUS->AddMaterial(matNi, 0.10);
        matSUS->AddMaterial(matFe, 0.68);

        G4double bpe_weight = 1070 * g;
        G4double bpe_volume = 20 * 10 * 5 * cm3;

        G4Material *matBPE = new G4Material("Borated_PE", bpe_weight / bpe_volume, 2);
        matBPE->AddMaterial(matPE, 0.93);
        matBPE->AddMaterial(matBO, 0.07);

        new G4Material("LowDensity_PE", 0.92 * g / cm3, matPE);

        G4Element *elmAl = nistManager->FindOrBuildElement("Al");
        G4Element *elmO  = nistManager->FindOrBuildElement("O");
        G4Element *elmB  = nistManager->FindOrBuildElement("B");
        G4Element *elmN  = nistManager->FindOrBuildElement("N");
        G4Element *elmH  = nistManager->FindOrBuildElement("H");
        G4Element *elmC  = nistManager->FindOrBuildElement("C");

        G4double alumina_density = 3.95 * g / cm3;

        G4Material *matCeramic = new G4Material("Ceramic", alumina_density, 2);
        matCeramic->AddElement(elmAl, 2);
        matCeramic->AddElement(elmO, 3);

        G4Material *matAcrylic = new G4Material("Acrylic", 1.19 * g / cm3, 3);
        matAcrylic->AddElement(elmC, 5);
        matAcrylic->AddElement(elmH, 8);
        matAcrylic->AddElement(elmO, 2);

        G4Material *matBoronNitride = new G4Material("BoronNitride", 3.40 * g / cm3, 2);
        matBoronNitride->AddElement(elmN, 1);
        matBoronNitride->AddElement(elmB, 1);

        G4Material *matCR39 = new G4Material("CR-39", 1.31 * g / cm3, 3);
        matCR39->AddElement(elmC, 12);
        matCR39->AddElement(elmH, 18);
        matCR39->AddElement(elmO, 7);

        // Print materials
        G4cout << *(G4Material::GetMaterialTable()) << G4endl;
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
} // namespace simcore
