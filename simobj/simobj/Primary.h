#ifndef __simobj_Primary_h__
#define __simobj_Primary_h__

#include "TObject.h"

#include "Math/LorentzVector.h"
#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"

namespace simobj {
    class Primary : public TObject {
      public:
        Primary() : fPDGCode(0), fPosition({0, 0, 0}), f4Momentum({0, 0, 0, 0}) {};
        Primary(const Primary &orig);

        TObject *Clone(const char *) const override;

        void SetXYZ(double x, double y, double z) { fPosition.SetXYZ(x, y, z); }
        void SetPxPyPzE(double px, double py, double pz, double e) {
            f4Momentum.SetPxPyPzE(px, py, pz, e);
        }

        void SetX(double a) { fPosition.SetX(a); }
        void SetY(double a) { fPosition.SetY(a); }
        void SetZ(double a) { fPosition.SetZ(a); }
        double GetX() const { return fPosition.X(); }
        double GetY() const { return fPosition.Y(); }
        double GetZ() const { return fPosition.Z(); }

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
        int fPDGCode;
        ROOT::Math::XYZVector fPosition;
        ROOT::Math::PxPyPzEVector f4Momentum;

        ClassDefOverride(simobj::Primary, 1)
    };
}; // namespace simobj
#endif
