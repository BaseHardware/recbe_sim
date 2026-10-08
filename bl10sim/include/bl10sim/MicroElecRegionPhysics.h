#ifndef __bl10sim_MicroElecRegionPhysics_h__
#define __bl10sim_MicroElecRegionPhysics_h__
#include "G4VPhysicsConstructor.hh"

namespace bl10sim {
    class MicroElecRegionPhysics : public G4VPhysicsConstructor {
      public:
        explicit MicroElecRegionPhysics(const std::vector<G4String> &regions)
            : G4VPhysicsConstructor("MicroElecRegionPhysics"), fTargetRegions(regions) {
            SetPhysicsType(0);
        }

        void ConstructParticle() override {}

        void ConstructProcess() override;

        size_t GetNRegions() const { return fTargetRegions.size(); }
        G4String GetRegion(size_t i) const { return fTargetRegions[i]; }

        void SetRegions(const std::vector<G4String> &a) { fTargetRegions = a; }
        std::vector<G4String> GetRegions() const { return fTargetRegions; }

      private:
        std::vector<G4String> fTargetRegions;
    };
} // namespace bl10sim

#endif
