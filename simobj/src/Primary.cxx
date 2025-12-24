#include <iostream>

#include "Primary.h"

namespace simobj {
    Primary::Primary(const Primary &orig)
        : fPDGCode(0), fPosition(orig.fPosition), f4Momentum(orig.f4Momentum) {};

    TObject *Primary::Clone(const char *) const {
        return static_cast<Primary *>(new Primary(*this));
    }

    void Primary::Print(Option_t *option) const {
        using namespace std;
        cout << "PDG code: " << fPDGCode << endl;
        cout << "(x, y, z, global time) = " << "(" << fPosition.X() << ", " << fPosition.Y() << ", "
             << fPosition.Z() << ") [unit: mm]" << endl;
        cout << "(px, py, pz, Ekin) = " << "(" << f4Momentum.Px() << ", " << f4Momentum.Py() << ", "
             << f4Momentum.Pz() << ", " << f4Momentum.E() << ") [unit: MeV]" << endl;
    }
} // namespace simobj
