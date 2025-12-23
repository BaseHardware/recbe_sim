#include "Step.h"

namespace simobj {
    Step::Step(const Step &orig)
        : fNDaughters(orig.fNDaughters), fEdep(orig.fEdep), fProperTime(orig.fProperTime),
          f4Position(orig.f4Position), f4Momentum(orig.f4Momentum), fProcessName(orig.fProcessName),
          fVolumeName(orig.fVolumeName) {};

    TObject *Step::Clone(const char *) const { return static_cast<Step *>(new Step(*this)); }
} // namespace simobj
