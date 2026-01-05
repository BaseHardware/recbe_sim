#include "simcore/MetadataManager.h"
#include "simcore/RootManager.h"

#include "recbesim/GitInformation.h"

#include "simobj/Metadata.h"

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
    }
} // namespace simcore
