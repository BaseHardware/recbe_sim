#include "bl10sim/PrimaryGeneratorAction.h"
#include "bl10sim/PrimaryGeneratorMessenger.h"

#include <cmath>

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4VSolid.hh"
#include "Randomize.hh"

// Copied from G4Box source code. It uses G4QuickRand(), which is not suitable for the MT
// environment
G4ThreeVector GetPointOnSurface(const G4Box *solid) {
    G4double fDx = solid->GetXHalfLength();
    G4double fDy = solid->GetYHalfLength();
    G4double fDz = solid->GetZHalfLength();
    G4double sxy = fDx * fDy, sxz = fDx * fDz, syz = fDy * fDz;
    G4double select = (sxy + sxz + syz) * G4UniformRand();
    G4double u      = 2. * G4UniformRand() - 1.;
    G4double v      = 2. * G4UniformRand() - 1.;

    if (select < sxy)
        return {u * fDx, v * fDy, ((select < 0.5 * sxy) ? -fDz : fDz)};
    else if (select < sxy + sxz)
        return {u * fDx, ((select < sxy + 0.5 * sxz) ? -fDy : fDy), v * fDz};
    else
        return {((select < sxy + sxz + 0.5 * syz) ? -fDx : fDx), u * fDy, v * fDz};
}

namespace bl10sim {
    PrimaryGeneratorAction::PrimaryGeneratorAction() : fEnabled(true) {
        G4int nofParticles = 1;
        fParticleGun       = new G4ParticleGun(nofParticles);

        auto particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        fParticleGun->SetParticleDefinition(particleDefinition);

        fMessenger = new PrimaryGeneratorMessenger(this);

        fDuctLength = 5.5 * m;
        fDuctEnterX = 105 * mm;
        fDuctEnterY = 105 * mm;

        fEGenerator   = new LethargyEnergyGenerator();
        fTGenerator   = new NeutronTimeGenerator();
        fFluxFilename = "./data/at_ductexit.txt";
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() {
        delete fParticleGun;
        delete fEGenerator;
        delete fMessenger;
    }

    void PrimaryGeneratorAction::InitializeEGenerator() {
        fEGenerator->SetInputFilename(fFluxFilename);
        fEGenerator->SetTrimLastones(true);
        fEGenerator->Initialize();
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
        if (!fEnabled) fParticleGun->GeneratePrimaryVertex(event);

        G4ThreadLocal static G4VPhysicalVolume *worldPV     = nullptr;
        G4ThreadLocal static G4ThreeVector totalTranslation = {0, 0, 0};

        if (worldPV == nullptr) {
            G4PhysicalVolumeStore *store = G4PhysicalVolumeStore::GetInstance();

            worldPV = G4TransportationManager::GetTransportationManager()
                          ->GetNavigatorForTracking()
                          ->GetWorldVolume();

            G4VPhysicalVolume *targetPV;
            targetPV = store->GetVolume("BeamWindowPV");
            if (targetPV == nullptr) {
                G4Exception(
                    "PrimaryVertexGeneration", "1", G4ExceptionSeverity::FatalException,
                    "We couldn't find PV named 'BeamWindowPV'. Please use the realistic beampipe "
                    "geometry if you want to generate primaries in the deuteron mode. Aborting.");
            }

            totalTranslation = targetPV->GetTranslation();

            G4LogicalVolume *nowMotherLV = targetPV->GetMotherLogical();

            G4LogicalVolume *worldLV = worldPV->GetLogicalVolume();

            while (nowMotherLV != worldLV) {
                G4VPhysicalVolume *findres = nullptr;
                for (auto *nowPV : *store) {
                    if (nowPV->GetLogicalVolume() == nowMotherLV) {
                        findres = nowPV;
                        break;
                    }
                }

                if (findres == nullptr) {
                    G4Exception("PrimaryVertexGeneration", "1", G4ExceptionSeverity::FatalException,
                                "We couldn't find PV from LV. Aborting.");
                } else {
                    totalTranslation += findres->GetTranslation();
                    nowMotherLV = findres->GetMotherLogical();
                }
            }
        }

        auto beamWindowPV = G4PhysicalVolumeStore::GetInstance()->GetVolume("BeamWindowPV");

        G4Box *beamWindowSolid = static_cast<G4Box *>(beamWindowPV->GetLogicalVolume()->GetSolid());

        G4ThreeVector bwSurfPosition = GetPointOnSurface(beamWindowSolid);
        fParticleGun->SetParticlePosition(bwSurfPosition + totalTranslation);

        G4double particleEnergy = fEGenerator->Generate();
        fParticleGun->SetParticleEnergy(particleEnergy);

        G4ThreeVector exitPosition, enterPosition;
        enterPosition.setX((G4UniformRand() - 0.5) * fDuctEnterX);
        enterPosition.setY((G4UniformRand() - 0.5) * fDuctEnterY);
        enterPosition.setZ(0);

        exitPosition = bwSurfPosition;
        exitPosition.setZ(fDuctLength);

        G4ThreeVector pDir      = exitPosition - enterPosition;
        G4double flightDistance = pDir.mag();
        pDir *= 1. / flightDistance;
        fParticleGun->SetParticleMomentumDirection(pDir);

        G4double particleTime = fTGenerator->Generate(particleEnergy / eV, flightDistance / m);
        fParticleGun->SetParticleTime(particleTime * us);
        fParticleGun->GeneratePrimaryVertex(event);
    }

} // namespace bl10sim
