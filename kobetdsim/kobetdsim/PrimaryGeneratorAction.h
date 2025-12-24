#ifndef __kobetdsim_PrimaryGeneratorAction_h__
#define __kobetdsim_PrimaryGeneratorAction_h__

#include "G4String.hh"

#include "G4VUserPrimaryGeneratorAction.hh"

#include <map>
#include <random>
#include <vector>

class G4ParticleGun;
class G4ParticleDefinition;
class G4Event;

namespace kobetdsim {
    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
      public:
        PrimaryGeneratorAction(G4String primMode, G4String filename = "");
        ~PrimaryGeneratorAction() override;

        void GeneratePrimaries(G4Event *) override;

        G4ParticleGun *GetParticleGun() { return fParticleGun; }

      private:
        void LoadSpectrum(const std::string &filename);
        G4ParticleGun *fParticleGun = nullptr; // G4 particle gun
        G4String fPrimMode;
        G4String fPrimaryFilename;

        std::map<int, G4ParticleDefinition *> fPDGTable;
        std::vector<int> fPrim_PDG;
        std::vector<double> fPrim_X;
        std::vector<double> fPrim_Y;
        std::vector<double> fPrim_Z;
        std::vector<double> fPrim_E;
        std::vector<double> fPrim_pX;
        std::vector<double> fPrim_pY;
        std::vector<double> fPrim_pZ;

        std::vector<double> energies;
        std::vector<int> weights;
        std::discrete_distribution<> spectrum_dis;
        std::mt19937 rng;
    };
} // namespace kobetdsim
#endif
