#ifndef __simobj_Primary_h__
#define __simobj_Primary_h__

#include "TObject.h"

#include "Math/LorentzVector.h"
#include "Math/Vector4Dfwd.h"

namespace simobj {
    class Primary : public TObject {
      public:
        Primary()
            : fPDGCode(0), fVertexIdx(0), fParticleIdx(0), f4Position({0, 0, 0, 0}),
              f4Momentum({0, 0, 0, 0}) {};
        Primary(const Primary &orig);

        TObject *Clone(const char *) const override;

        void SetXYZT(double x, double y, double z, double t) { f4Position.SetXYZT(x, y, z, t); }
        void SetPxPyPzE(double px, double py, double pz, double e) {
            f4Momentum.SetPxPyPzE(px, py, pz, e);
        }

        void SetPDGCode(int a) { fPDGCode = a; }
        int GetPDGCode() const { return fPDGCode; }

        void SetVertexIdx(int a) { fVertexIdx = a; }
        int GetVertexIdx() const { return fVertexIdx; }

        void SetParticleIdx(int a) { fParticleIdx = a; }
        int GetParticleIdx() const { return fParticleIdx; }

        void SetX(double a) { f4Position.SetPx(a); }
        void SetY(double a) { f4Position.SetPy(a); }
        void SetZ(double a) { f4Position.SetPz(a); }
        void SetT(double a) { f4Position.SetE(a); }
        double GetX() const { return f4Position.X(); }
        double GetY() const { return f4Position.Y(); }
        double GetZ() const { return f4Position.Z(); }
        double GetT() const { return f4Position.T(); }

        void SetPx(double a) { f4Momentum.SetPx(a); }
        void SetPy(double a) { f4Momentum.SetPy(a); }
        void SetPz(double a) { f4Momentum.SetPz(a); }
        void SetKineticEnergy(double a) { f4Momentum.SetE(a); }
        double GetPx() const { return f4Momentum.Px(); }
        double GetPy() const { return f4Momentum.Py(); }
        double GetPz() const { return f4Momentum.Pz(); }
        double GetKineticEnergy() const { return f4Momentum.E(); }

        void Print(Option_t *option = "") const override;

      private:
        int fPDGCode, fVertexIdx, fParticleIdx;
        ROOT::Math::XYZTVector f4Position;
        ROOT::Math::PxPyPzEVector f4Momentum;

        ClassDefOverride(simobj::Primary, 1)
    };
}; // namespace simobj
#endif
