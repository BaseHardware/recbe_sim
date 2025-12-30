#include <iostream>

#include "TClonesArray.h"

#include "simobj/Primary.h"

namespace simobj {
    Vertex::Vertex(const Vertex &orig)
        : fParticleNumber(orig.fParticleNumber), fWeight(orig.fWeight),
          f4Position(orig.f4Position) {};

    TObject *Vertex::Clone(const char *) const { return static_cast<Vertex *>(new Vertex(*this)); }

    void Vertex::Print(Option_t *option) const {
        using namespace std;
        cout << "Weight: " << fWeight << " | N_part: " << fParticleNumber << " | ";
        cout << "(x0, y0, z0, t0) = " << "(" << f4Position.X() << ", " << f4Position.Y() << ", "
             << f4Position.Z() << ", " << f4Position.T() << ") [unit: mm, ns]";
    }

    PrimaryParticle::PrimaryParticle(const PrimaryParticle &orig)
        : fVertexIdx(orig.fVertexIdx), fPDGCode(orig.fPDGCode), fWeight(orig.fWeight),
          fPolarization(orig.fPolarization), f4Momentum(orig.f4Momentum) {};

    TObject *PrimaryParticle::Clone(const char *) const {
        return static_cast<PrimaryParticle *>(new PrimaryParticle(*this));
    }

    void PrimaryParticle::Print(Option_t *option) const {
        using namespace std;
        cout << "PDG Code: " << fPDGCode << " | Weight: " << fWeight << " | ";
        cout << "(px, py, pz, Ekin) = " << "(" << f4Momentum.Px() << ", " << f4Momentum.Py() << ", "
             << f4Momentum.Pz() << ", " << f4Momentum.E() << ") [unit: MeV] | ";
        cout << "(pol_x, pol_y, pol_z) = " << "(" << fPolarization.X() << ", " << fPolarization.Y()
             << ", " << fPolarization.Z() << ")";
    }

    Primary::Primary(int vertMaxNum, int partMaxNum) : fTCAVertex(nullptr), fTCAPrimPart(nullptr) {
        fTCAVertex   = new TClonesArray("simobj::Vertex", vertMaxNum);
        fTCAPrimPart = new TClonesArray("simobj::PrimaryParticle", partMaxNum);
    };
    Primary::Primary(const Primary &orig) {
        fTCAVertex   = static_cast<TClonesArray *>(orig.fTCAVertex->Clone());
        fTCAPrimPart = static_cast<TClonesArray *>(orig.fTCAPrimPart->Clone());
    }

    TObject *Primary::Clone(const char *) const {
        return static_cast<Primary *>(new Primary(*this));
    }

    void Primary::Print(Option_t *option) const {
        using namespace std;
        int prevVIdx = -1;
        int ppartNum = fTCAPrimPart->GetEntries();

        for (int idxPP = 0; idxPP < ppartNum; idxPP++) {
            PrimaryParticle *nowPP = static_cast<PrimaryParticle *>(fTCAPrimPart->At(idxPP));

            int vertIdx = nowPP->GetVertexIdx();

            if (prevVIdx != vertIdx) {
                fTCAVertex->At(vertIdx)->Print();
                cout << endl;
            }

            cout << "    ";
            nowPP->Print();
        }
    }

    TObject *Primary::GetVertexObjPtr(int idx) const { return (*fTCAVertex)[idx]; }
    TObject *Primary::GetPrimaryParticleObjPtr(int idx) const { return (*fTCAPrimPart)[idx]; }
    int Primary::GetVertexSize() const { return fTCAVertex->GetSize(); }
    int Primary::GetPrimaryParticleSize() const { return fTCAVertex->GetSize(); }

    void Primary::Clear() const {
        fTCAVertex->Clear("C");
        fTCAPrimPart->Clear("C");
    }
} // namespace simobj
