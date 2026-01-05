#include "simcore/MetadataManager.h"
#include "simcore/RootManager.h"

#include "recbesim/GitInformation.h"

#include "simobj/Metadata.h"

#include "G4RunManager.hh"
#include "Randomize.hh"

namespace simcore {
    void MetadataManager::FillMetadata(simobj::Metadata *target) const {
        using namespace recbesim;
        GitInformation &gitInfo = GitInformation::GetInstance();
        RootManager &rootman    = RootManager::GetInstance();

        target->SetGeometryType(fGeomTypename);
        target->SetSimulationName(fSimName);
        target->SetGitHash(gitInfo.GetHash());

        target->SetMaxTrackNum(rootman.GetMaxTrackNum());
        target->SetMaxStepNum(rootman.GetMaxStepNum());
        target->SetAllStepRecorded(rootman.DoesRecordStep());
        target->SetPrimaryRecorded(rootman.DoesRecordPrimary());
        target->SetOutputTreename(rootman.GetTreename());

        const long *seeds;
        if (fAutoRandomSeed) {
            seeds = G4Random::getTheSeeds();
        } else {
            seeds = fRandomSeeds;
        }
        target->SetRandomSeed(seeds[0], false);
        target->SetRandomSeed(seeds[1], true);

        target->SetThreadNum(G4RunManager::GetRunManager()->GetNumberOfThreads());
    }

    void MetadataManager::SetRandomSeed(long a, bool aux) {
        if (aux)
            fRandomSeeds[1] = a;
        else
            fRandomSeeds[0] = a;
    }

    long MetadataManager::GetRandomSeed(bool aux) const {
        if (aux)
            return fRandomSeeds[1];
        else
            return fRandomSeeds[0];
    }
} // namespace simcore
