#ifndef __bl10sim_PrimaryGeneratorAction_h__
#define __bl10sim_PrimaryGeneratorAction_h__

#include "G4VUserPrimaryGeneratorAction.hh"

#include <string>
#include <vector>

class G4ParticleGun;
class G4Event;

namespace bl10sim {
    class LethargyEnergyGenerator {
      public:
        LethargyEnergyGenerator();
        virtual ~LethargyEnergyGenerator();

        void SetInputFilename(const std::string &a) { fFluxFilename = a; }
        std::string GetInputFilename() const { return fFluxFilename; }

        void SetTrimLastones(bool a) { fTrimLastones = a; }
        bool GetTrimLastones() const { return fTrimLastones; }

        double Generate() const;

        bool Initialize();

      private:
        bool fTrimLastones;
        bool fReady;
        std::string fFluxFilename;
        std::vector<double> fCumulative;
        std::vector<double> fBoundaries;
    };

    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
      public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override;

        void GeneratePrimaries(G4Event *event) override;

      private:
        G4ParticleGun *fParticleGun          = nullptr; // G4 particle gun
        LethargyEnergyGenerator *fEGenerator = nullptr;
    };
} // namespace bl10sim
#endif
