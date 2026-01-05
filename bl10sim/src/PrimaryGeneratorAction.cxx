#include "bl10sim/PrimaryGeneratorAction.h"

#include <cmath>
#include <fstream>

#include "G4LogicalVolume.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4VSolid.hh"
#include "Randomize.hh"

namespace bl10sim {
    PrimaryGeneratorAction::PrimaryGeneratorAction() {
        G4int nofParticles = 1;
        fParticleGun       = new G4ParticleGun(nofParticles);

        auto particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        fParticleGun->SetParticleDefinition(particleDefinition);

        fEGenerator = new LethargyEnergyGenerator();
        fEGenerator->SetInputFilename("./data/port10_inp.txt");
        fEGenerator->SetTrimLastones(true);
        fEGenerator->Initialize();
    }

    PrimaryGeneratorAction::~PrimaryGeneratorAction() {
        delete fParticleGun;
        delete fEGenerator;
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
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
                G4Exception("PrimaryVertexGeneration", "1", G4ExceptionSeverity::FatalException,
                            "We couldn't find PV named 'BeamWindowPV'. Please use the "
                            "realistic beampipe geometry if you want to generate primaries in the "
                            "deuteron mode. Aborting.");
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

        G4VSolid *beamWindowSolid = beamWindowPV->GetLogicalVolume()->GetSolid();

        // Set gun position
        G4ThreeVector position = beamWindowSolid->GetPointOnSurface();
        fParticleGun->SetParticlePosition(position + totalTranslation);

        fParticleGun->SetParticleEnergy(fEGenerator->Generate());

        G4ThreeVector pDir(0, 0, 1);
        pDir.setTheta(0.065 * G4UniformRand());
        pDir.setPhi(2 * M_PI * G4UniformRand());
        fParticleGun->SetParticleMomentumDirection(pDir);

        fParticleGun->GeneratePrimaryVertex(event);
    }

    LethargyEnergyGenerator::LethargyEnergyGenerator()
        : fTrimLastones(true), fFluxFilename(), fCumulative(), fBoundaries() {}
    LethargyEnergyGenerator::~LethargyEnergyGenerator() {}

    bool LethargyEnergyGenerator::Initialize() {
        using namespace std;

        string linestr;

        double e, w;
        int linecnt = 0;

        fBoundaries.clear();
        fCumulative.clear();

        ifstream fin(fFluxFilename);
        if (fin.is_open()) {
            double prev_e = 0, prev_w = 0;
            double sumw = 0;

            while (true) {
                getline(fin, linestr);
                if (linestr.length() != 0) linecnt++;
                if (fin.eof()) break;
            }

            fin.clear();
            fin.seekg(ios_base::beg);

            while (true) {
                if (!(fin >> e)) {
                    break;
                } else if (e == 0) {
                    G4ExceptionDescription msg;
                    msg << "The energy boundary value is zero, which is not accepted. Please check "
                           "your flux file."
                        << G4endl;
                    G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0002",
                                FatalException, msg);
                    goto cleanup_and_exit;
                } else if (e < prev_e) {
                    G4ExceptionDescription msg;
                    msg << "The energy boundary does not increase monotonically. Please check your "
                           "flux file."
                        << G4endl;
                    G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0003",
                                FatalException, msg);
                    goto cleanup_and_exit;
                }
                e *= MeV;
                prev_e = e;
                fBoundaries.push_back(e);

                if (!(fin >> w)) {
                    break;
                } else if (w < 0) {
                    G4ExceptionDescription msg;
                    msg << "The weight value is negative, which is not accepted. Please check your "
                           "flux file."
                        << G4endl;
                    G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0004",
                                FatalException, msg);
                    goto cleanup_and_exit;
                }
                prev_w = w;
                sumw += w;
                fCumulative.push_back(sumw);
            }

            bool okay;
            if (linecnt != fBoundaries.size()) {
                okay = false;
            } else if (fBoundaries.size() != fCumulative.size() &&
                       (fBoundaries.size() - 1) != fCumulative.size()) {
                okay = false;
            } else {
                okay = true;
            }
            if (okay) {
                if (fBoundaries.size() == fCumulative.size()) {
                    sumw -= prev_w;
                    fCumulative.pop_back();
                }

                if (sumw == 0) {
                    G4ExceptionDescription msg;
                    msg << "The sum of weight is zero, which is not accepted. Please check your "
                           "flux file."
                        << G4endl;
                    G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0005",
                                FatalException, msg);
                    goto cleanup_and_exit;
                }

                for_each(fCumulative.begin(), fCumulative.end(),
                         [sumw](double &a) -> void { a /= sumw; });

                if (fTrimLastones) {
                    size_t last_one_count = 0;
                    for (auto it = fCumulative.rbegin() + 1; it != fCumulative.rend(); ++it) {
                        if (*it != 1) {
                            break;
                        } else {
                            last_one_count++;
                        }
                    }
                    for (size_t i = 0; i < last_one_count; i++) {
                        fCumulative.pop_back();
                        fBoundaries.pop_back();
                    }
                }
            } else {
                G4ExceptionDescription msg;
                msg << "Your flux file has a wrong structue. Please check the file." << G4endl;
                G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0006",
                            FatalException, msg);
                goto cleanup_and_exit;
            }
        } else {
            G4ExceptionDescription msg;
            msg << "Failed to open the flux file." << G4endl;
            G4Exception("LethargyEnergyGenerator::Initialize()", "LethargyE0001", FatalException,
                        msg);
            goto cleanup_and_exit;
        }

        fReady = true;
        return true;
    cleanup_and_exit:
        if (fin.is_open()) fin.close();
        fBoundaries.clear();
        fCumulative.clear();
        fReady = false;
        return false;
    }

    double LethargyEnergyGenerator::Generate() const {
        if (!fReady) {
            G4ExceptionDescription msg;
            msg << "This energy generator is not ready." << G4endl;
            G4Exception("LethargyEnergyGenerator::Generate()", "LethargyE0001", JustWarning, msg);
            return 0;
        }

        double r;

        r = G4UniformRand();
        size_t nowIdx =
            lower_bound(fCumulative.begin(), fCumulative.end(), r) - fCumulative.begin();

        double nowE  = fBoundaries[nowIdx];
        double nextE = fBoundaries[nowIdx + 1];

        r = G4UniformRand();
        return nowE * exp(r * log(nextE / nowE));
    }
} // namespace bl10sim
