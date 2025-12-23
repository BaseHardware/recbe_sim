#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;
class G4Event;

namespace B4 {
    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
      public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override;

        void GeneratePrimaries(G4Event *event) override;

      private:
        G4ParticleGun *fParticleGun = nullptr; // G4 particle gun
    };

} // namespace B4
#endif
