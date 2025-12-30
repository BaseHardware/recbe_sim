#ifndef __simobj_Primary_h__
#define __simobj_Primary_h__

#include "TObject.h"

#include "Math/LorentzVector.h"
#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"

class TClonesArray;

namespace simobj {
    class Vertex : public TObject {
      public:
        Vertex() : fParticleNumber(0), fWeight(1), f4Position({0, 0, 0, 0}) {};
        Vertex(const Vertex &orig);
        TObject *Clone(const char *) const override;

        void SetXYZT(double x, double y, double z, double t) { f4Position.SetXYZT(x, y, z, t); }

        void SetX(double a) { f4Position.SetPx(a); }
        void SetY(double a) { f4Position.SetPy(a); }
        void SetZ(double a) { f4Position.SetPz(a); }
        void SetT(double a) { f4Position.SetE(a); }
        double GetX() const { return f4Position.X(); }
        double GetY() const { return f4Position.Y(); }
        double GetZ() const { return f4Position.Z(); }
        double GetT() const { return f4Position.T(); }

        void SetNParticle(double a) { fParticleNumber = a; }
        double GetNParticle() const { return fParticleNumber; }

        void SetWeight(double a) { fWeight = a; }
        double GetWeight() const { return fWeight; }

        void Print(Option_t *option = "") const override;

      private:
        int fParticleNumber;
        double fWeight;
        ROOT::Math::XYZTVector f4Position;

        ClassDefOverride(simobj::Vertex, 1)
    };

    class PrimaryParticle : public TObject {
      public:
        PrimaryParticle()
            : fPDGCode(0), fWeight(1), fPolarization({0, 0, 0}), f4Momentum({0, 0, 0, 0}) {};
        PrimaryParticle(const PrimaryParticle &orig);
        TObject *Clone(const char *) const override;

        void SetVertexIdx(int a) { fVertexIdx = a; }
        int GetVertexIdx() const { return fVertexIdx; }

        void SetPDGCode(int a) { fPDGCode = a; }
        int GetPDGCode() const { return fPDGCode; }

        void SetWeight(double a) { fWeight = a; }
        double GetWeight() const { return fWeight; }

        void SetPxPyPzE(double px, double py, double pz, double e) {
            f4Momentum.SetPxPyPzE(px, py, pz, e);
        }

        void SetPx(double a) { f4Momentum.SetPx(a); }
        void SetPy(double a) { f4Momentum.SetPy(a); }
        void SetPz(double a) { f4Momentum.SetPz(a); }
        void SetKineticEnergy(double a) { f4Momentum.SetE(a); }
        double GetPx() const { return f4Momentum.Px(); }
        double GetPy() const { return f4Momentum.Py(); }
        double GetPz() const { return f4Momentum.Pz(); }
        double GetKineticEnergy() const { return f4Momentum.E(); }

        void SetPolX(double a) { fPolarization.SetX(a); }
        void SetPolY(double a) { fPolarization.SetY(a); }
        void SetPolZ(double a) { fPolarization.SetZ(a); }
        double GetPolX() const { return fPolarization.X(); }
        double GetPolY() const { return fPolarization.Y(); }
        double GetPolZ() const { return fPolarization.Z(); }

        void Print(Option_t *option = "") const override;

      private:
        int fVertexIdx, fPDGCode;
        double fWeight;
        ROOT::Math::XYZVector fPolarization;
        ROOT::Math::PxPyPzEVector f4Momentum;

        ClassDefOverride(simobj::PrimaryParticle, 1)
    };

    class Primary : public TObject {
      public:
        Primary(int vertMaxNum = 100, int partMaxNum = 200);
        Primary(const Primary &orig);

        TObject *Clone(const char *) const override;

        void Print(Option_t *option = "") const override;

        TObject *GetVertexObjPtr(int idx) const;
        TObject *GetPrimaryParticleObjPtr(int idx) const;

        int GetVertexSize() const;
        int GetPrimaryParticleSize() const;

        void Clear() const;

      private:
        TClonesArray *fTCAVertex;
        TClonesArray *fTCAPrimPart;

        ClassDefOverride(simobj::Primary, 1)
    };
}; // namespace simobj
#endif
