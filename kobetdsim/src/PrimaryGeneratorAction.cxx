#include <cstdint>
#include <fstream>

#include "PrimaryGeneratorAction.h"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "Randomize.hh"

using namespace std;

static G4double SampleThetaFromLinearPDF() {
    const G4double theta_min_deg = 0.0;
    const G4double theta_max_deg = 90.0;
    const G4double slope         = 0.007; // from f(θ) = 1 - 0.007θ

    while (true) {
        G4double theta_deg = G4UniformRand() * (theta_max_deg - theta_min_deg);
        G4double f         = 1.0 - slope * theta_deg;
        G4double u         = G4UniformRand();     // uniform random number in [0,1)
        if (u < f) return theta_deg * CLHEP::deg; // convert to radians
    }
}

namespace kobetdsim {
    PrimaryGeneratorAction::PrimaryGeneratorAction(G4String pMode, G4String filename)
        : fPrimMode(pMode), fPrimaryFilename(filename) {
        G4int nofParticles = 1;
        fParticleGun       = new G4ParticleGun(nofParticles);

        // default particle kinematic

        G4ParticleDefinition *particleDefinition;
        if (fPrimMode == "deuteron") {
            particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("deuteron");
            fParticleGun->SetParticleDefinition(particleDefinition);
        } else if (fPrimMode == "neutron") {
            particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
            fParticleGun->SetParticleDefinition(particleDefinition);

            // Load energy spectrum
            LoadSpectrum("data/spectrum.txt");
            rng.seed(std::random_device{}());
            spectrum_dis = std::discrete_distribution<>(weights.begin(), weights.end());
        } else if (fPrimMode == "external_neutron_sampling" || fPrimMode == "external_sequential") {
            ifstream input_file(filename);

            if (!input_file.is_open()) {
                G4Exception("PrimaryGeneratorAction", "FileNotFound", FatalException,
                            ("Cannot open primary file: " + filename).c_str());
            }

            uint64_t entries;
            input_file >> entries;

            fPrim_PDG.resize(entries);
            fPrim_X.resize(entries);
            fPrim_Y.resize(entries);
            fPrim_Z.resize(entries);
            fPrim_E.resize(entries);
            fPrim_pX.resize(entries);
            fPrim_pY.resize(entries);
            fPrim_pZ.resize(entries);

            size_t int_bytes    = sizeof(int) * entries;
            size_t double_bytes = sizeof(double) * entries;

            input_file.read((char *)fPrim_PDG.data(), int_bytes);
            input_file.read((char *)fPrim_X.data(), double_bytes);
            input_file.read((char *)fPrim_Y.data(), double_bytes);
            input_file.read((char *)fPrim_Z.data(), double_bytes);
            input_file.read((char *)fPrim_E.data(), double_bytes);
            input_file.read((char *)fPrim_pX.data(), double_bytes);
            input_file.read((char *)fPrim_pY.data(), double_bytes);
            input_file.read((char *)fPrim_pZ.data(), double_bytes);

            for (auto nowpdg : fPrim_PDG) {
                if (fPDGTable.find(nowpdg) == fPDGTable.end()) {
                    fPDGTable[nowpdg] = G4ParticleTable::GetParticleTable()->FindParticle(nowpdg);
                }
            }

            input_file.close();
        } else {
            G4Exception("PrimaryGeneratorAction", "WrongMode", FatalException,
                        "Wrong primary generation mode");
        }
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
        // This function is called at the begining of event

        // In order to avoid dependence of PrimaryGeneratorAction
        // on DetectorConstruction class we get world volume
        // from G4LogicalVolumeStore.

        G4ThreadLocal static G4bool first = true;

        G4ThreadLocal static G4ThreeVector totalTranslation = {0, 0, 0};

        G4ThreadLocal static G4VPhysicalVolume *targetPV;

        if (first) {
            G4LogicalVolume *worldLV = G4LogicalVolumeStore::GetInstance()->GetVolume("World");

            G4PhysicalVolumeStore *store = G4PhysicalVolumeStore::GetInstance();

            if (fPrimMode == "deuteron") {
                targetPV = G4PhysicalVolumeStore::GetInstance()->GetVolume("CeramicInsulator_PV");
                if (targetPV == nullptr) {
                    G4Exception(
                        "PrimaryVertexGeneration", "1", G4ExceptionSeverity::FatalException,
                        "We couldn't find PV named 'CeramicInsulator_PV'. Please use the "
                        "realistic beampipe geometry if you want to generate primaries in the "
                        "deuteron mode. Aborting.");
                }
            } else {
                targetPV = G4PhysicalVolumeStore::GetInstance()->GetVolume("Target_PV");
            }
            totalTranslation = targetPV->GetTranslation();

            G4LogicalVolume *nowMotherLV = targetPV->GetMotherLogical();

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
            first = false;
        }

        G4ThreeVector nowPos;

        if (fPrimMode == "deuteron") {
            G4double x = G4RandGauss::shoot(0, 0.1 * cm);
            G4double y = G4RandGauss::shoot(0, 0.1 * cm);
            G4double z = G4RandGauss::shoot(0, 0.15 * cm);

            nowPos = {x, y, z};

            fParticleGun->SetParticlePosition(nowPos + totalTranslation);

            G4double theta = G4RandGauss::shoot(90 * deg, 2 * deg);
            G4double phi   = G4UniformRand() * M_PI * 2;

            G4ThreeVector direction = {1, 0, 0};
            direction.rotateY(theta);
            direction.rotateZ(phi);

            fParticleGun->SetParticleMomentumDirection(direction);

            fParticleGun->SetParticleEnergy(3 * MeV);
        } else if (fPrimMode == "neutron") {
            G4Tubs *targetTube = static_cast<G4Tubs *>(targetPV->GetLogicalVolume()->GetSolid());
            while (true) {
                nowPos = targetTube->GetPointOnSurface();
                if (abs(nowPos.getZ()) == targetTube->GetZHalfLength()) {
                    nowPos.setZ(-abs(nowPos.getZ()));
                    break;
                }
            }
            fParticleGun->SetParticlePosition(nowPos + totalTranslation);

            G4double theta = SampleThetaFromLinearPDF();
            G4double phi   = G4UniformRand() * M_PI * 2;

            G4ThreeVector direction;
            direction.setRThetaPhi(-1.0, theta, phi); // unit vector

            fParticleGun->SetParticleMomentumDirection(direction);

            G4bool discrete_energy = true;

            int index = spectrum_dis(rng);

            double sampled_energy;

            if (discrete_energy) {
                sampled_energy = energies[index] * MeV;
            } else {
                int a, b;
                if (index == energies.size() - 1) {
                    a = index - 1;
                    b = index;
                } else {
                    a = index;
                    b = index + 1;
                }

                sampled_energy = std::uniform_real_distribution<>(energies[a], energies[b])(rng);
            }

            fParticleGun->SetParticleEnergy(sampled_energy);
        } else if (G4StrUtil::contains(fPrimMode, "external")) {
            uint64_t idx;
            if (fPrimMode == "external_neutron_sampling") {
                do {
                    idx = G4UniformRand() * fPrim_PDG.size();
                } while (abs(fPrim_PDG[idx]) != 2112);
            } else if (fPrimMode == "external_sequential") {
                idx = event->GetEventID();
            } else {
                idx = 0;
            }

            if (idx >= fPrim_X.size()) {
                fParticleGun->SetNumberOfParticles(0);
            } else {
                fParticleGun->SetNumberOfParticles(1);

                G4ThreeVector position = {fPrim_X[idx], fPrim_Y[idx], fPrim_Z[idx]};
                G4ThreeVector momentum = {fPrim_pX[idx], fPrim_pY[idx], fPrim_pZ[idx]};

                G4double energy = fPrim_E[idx];
                fParticleGun->SetParticlePosition(position + totalTranslation);
                fParticleGun->SetParticleMomentumDirection(momentum);
                fParticleGun->SetParticleEnergy(energy);
                fParticleGun->SetParticleDefinition(fPDGTable[fPrim_PDG[idx]]);
            }
        }

        fParticleGun->GeneratePrimaryVertex(event);
    }

    void PrimaryGeneratorAction::LoadSpectrum(const std::string &filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            G4Exception("PrimaryGeneratorAction::LoadSpectrum", "FileNotFound", FatalException,
                        ("Cannot open spectrum file: " + filename).c_str());
        }

        std::string line;
        std::getline(infile, line); // skip header

        double en;
        int count;
        while (infile >> en >> count) {
            energies.push_back(en);
            weights.push_back(count);
        }

        if (energies.empty()) {
            G4Exception("PrimaryGeneratorAction::LoadSpectrum", "EmptySpectrum", FatalException,
                        "Spectrum file contains no data.");
        }
    }
} // namespace kobetdsim
