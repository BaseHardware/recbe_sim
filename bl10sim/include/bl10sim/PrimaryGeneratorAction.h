#ifndef __bl10sim_PrimaryGeneratorAction_h__
#define __bl10sim_PrimaryGeneratorAction_h__

#include "LethargyEGenerator.h"
#include "NeutronTimeGenerator.h"

#include "G4VUserPrimaryGeneratorAction.hh"

#include <string>

class G4ParticleGun;
class G4Event;

namespace bl10sim {
    class PrimaryGeneratorMessenger;

    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
      public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override;

        void InitializeGenerators();
        void GeneratePrimaries(G4Event *event) override;

        void SetDuctLength(double a) { fDuctLength = a; };
        double GetDuctLength() const { return fDuctLength; };
        void SetDuctEnteranceXSize(double a) { fDuctEnterX = a; };
        double GetDuctEnteranceXSize() const { return fDuctEnterX; };
        void SetDuctEnteranceYSize(double a) { fDuctEnterY = a; };
        double GetDuctEnteranceYSize() const { return fDuctEnterY; };

        void SetFluxFilename(const std::string &a) { fFluxFilename = a; }
        std::string GetFluxFilename() const { return fFluxFilename; }

        bool IsEnabled() const { return fEnabled; }
        void Enable(bool a = true) { fEnabled = a; }

        void SetTimeOffset(double a) { fTGenerator->SetTimeOffset(a); }
        void SetBunchSeparation(double a) { fTGenerator->SetBunchSeparation(a); }
        void SetFirstBunchOffset(double a) { fTGenerator->SetFirstBunchOffset(a); }
        void SetFirstBunchFraction(double a) { fTGenerator->SetFirstBunchFraction(a); }
        void SetFirstBunchFWHM(double a) { fTGenerator->SetFirstBunchFWHM(a); }
        void SetSecondBunchFWHM(double a) { fTGenerator->SetSecondBunchFWHM(a); }

      private:
        G4ParticleGun *fParticleGun           = nullptr;
        LethargyEnergyGenerator *fEGenerator  = nullptr;
        NeutronTimeGenerator *fTGenerator     = nullptr;
        PrimaryGeneratorMessenger *fMessenger = nullptr;

        double fDuctLength;
        double fDuctEnterX;
        double fDuctEnterY;

        std::string fFluxFilename;

        bool fEnabled;
    };
} // namespace bl10sim
#endif
