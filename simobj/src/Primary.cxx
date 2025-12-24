#include <iostream>

#include "Primary.h"

namespace simobj {
    Primary::Primary(const Primary &orig)
        : fPDGCode(orig.fPDGCode), fVertexIdx(orig.fVertexIdx), fParticleIdx(orig.fParticleIdx),
          f4Position(orig.f4Position), f4Momentum(orig.f4Momentum) {};

    TObject *Primary::Clone(const char *) const {
        return static_cast<Primary *>(new Primary(*this));
    }

    void Primary::Print(Option_t *option) const {
        using namespace std;
        cout << "PDG code: " << fPDGCode << endl;
        cout << "Vertex-Particle index: " << fVertexIdx << "-" << fParticleIdx << endl;
        cout << "(x, y, z, global time) = " << "(" << f4Position.X() << ", " << f4Position.Y()
             << ", " << f4Position.Z() << ", " << f4Position.T() << ") [unit: mm, ns]" << endl;
        cout << "(px, py, pz, Ekin) = " << "(" << f4Momentum.Px() << ", " << f4Momentum.Py() << ", "
             << f4Momentum.Pz() << ", " << f4Momentum.E() << ") [unit: MeV]" << endl;
    }
} // namespace simobj
