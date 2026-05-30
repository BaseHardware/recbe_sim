#include "eventviewer/EventViewer.h"

#include "simobj/Step.h"
#include "simobj/Track.h"

#include <iomanip>
#include <sstream>

namespace eventviewer {
    std::string EventData::FormatTrack(const simobj::Track &track, int idx) {
        const auto &first = track.GetFirstStep();
        const auto &final = track.GetFinalStep();
        std::ostringstream os;
        os << std::fixed << std::setprecision(3) << idx << "  " << track.GetTrackID() << ' '
           << track.GetParentID() << ' ' << track.GetPDGCode() << ' ' << track.GetName() << ' '
           << track.GetNStep() << "  (" << first.GetX() << ", " << first.GetY() << ", "
           << first.GetZ() << ", " << first.GetGlobalTime() << ")  (" << final.GetX() << ", "
           << final.GetY() << ", " << final.GetZ() << ", " << final.GetGlobalTime() << ")  "
           << final.GetProcessName();
        return os.str();
    }

    std::string EventData::FormatStep(const simobj::Step &step, int idx, int trackId) {
        std::ostringstream os;
        os << std::fixed << std::setprecision(4) << idx << "  " << trackId << "  " << step.GetX()
           << ' ' << step.GetY() << ' ' << step.GetZ() << ' ' << step.GetGlobalTime() << "  "
           << step.GetDepositedEnergy() << ' ' << step.GetNonIonDepositedEnergy() << ' '
           << step.GetKineticEnergy() << "  " << step.GetVolumeName() << ' ' << step.GetCopyNumber()
           << ' ' << step.GetEnvelopeCopyNumber() << ' ' << step.GetProcessName() << ' '
           << step.GetNDaughters();
        return os.str();
    }
} // namespace eventviewer
