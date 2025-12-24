#include <iostream>

#include "Step.h"

namespace simobj {
    Step::Step(const Step &orig)
        : fTrackID(0), fNDaughters(orig.fNDaughters), fEdep(orig.fEdep),
          fProperTime(orig.fProperTime), f4Position(orig.f4Position), f4Momentum(orig.f4Momentum),
          fProcessName(orig.fProcessName), fVolumeName(orig.fVolumeName) {};

    TObject *Step::Clone(const char *) const { return static_cast<Step *>(new Step(*this)); }

    void Step::Print(Option_t *option) const {
        using namespace std;
        cout << "Track ID: " << fTrackID << endl;
        cout << "(x, y, z, global time) = " << "(" << f4Position.X() << ", " << f4Position.Y()
             << ", " << f4Position.Z() << ", " << f4Position.T() << ") [unit: mm, ns]" << endl;
        cout << "(px, py, pz, Ekin) = " << "(" << f4Momentum.Px() << ", " << f4Momentum.Py() << ", "
             << f4Momentum.Pz() << ", " << f4Momentum.E() << ") [unit: MeV]" << endl;
        cout << "Number of daughters: " << fNDaughters << endl;
        cout << "Edep: " << fEdep << endl;
        cout << "Proper time: " << fProperTime << endl;
        cout << "Volume: " << fVolumeName << endl;
        cout << "Process: " << fProcessName << endl;
    }
} // namespace simobj
