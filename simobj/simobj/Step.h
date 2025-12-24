#ifndef __simobj_Step_h__
#define __simobj_Step_h__

#include "TObject.h"
#include "TString.h"

#include "Math/LorentzVector.h"
#include "Math/Vector4Dfwd.h"

namespace simobj {
    class Step : public TObject {
      public:
        Step()
            : fNDaughters(0), fEdep(0), fProperTime(0), f4Position(), f4Momentum(),
              fProcessName(""), fVolumeName("") {};
        Step(const Step &orig);

        TObject *Clone(const char *) const override;

        void SetNDaughters(int a) { fNDaughters = a; }
        int GetNDaughters() const { return fNDaughters; }

        void SetDepositedEnergy(double a) { fEdep = a; }
        double GetDepositedEnergy() const { return fEdep; }

        void SetProperTime(double a) { fProperTime = a; }
        double GetProperTime() const { return fProperTime; }

        void SetXYZT(double x, double y, double z, double t) { f4Position.SetXYZT(x, y, z, t); }
        void SetPxPyPzE(double px, double py, double pz, double e) {
            f4Momentum.SetPxPyPzE(px, py, pz, e);
        }

        void SetX(double a) { f4Position.SetPx(a); }
        void SetY(double a) { f4Position.SetPy(a); }
        void SetZ(double a) { f4Position.SetPz(a); }
        void SetT(double a) { f4Position.SetE(a); }
        double GetX() const { return f4Position.X(); }
        double GetY() const { return f4Position.Y(); }
        double GetZ() const { return f4Position.Z(); }
        double GetGlobalTime() const { return f4Position.T(); }

        void SetPx(double a) { f4Momentum.SetPx(a); }
        void SetPy(double a) { f4Momentum.SetPy(a); }
        void SetPz(double a) { f4Momentum.SetPz(a); }
        void SetE(double a) { f4Momentum.SetE(a); }
        double GetPx() const { return f4Momentum.Px(); }
        double GetPy() const { return f4Momentum.Py(); }
        double GetPz() const { return f4Momentum.Pz(); }
        double GetE() const { return f4Momentum.E(); }

        void SetProcessName(const char *a) { fProcessName = a; }
        void SetVolumeName(const char *a) { fVolumeName = a; }
        TString GetProcessName() const { return fProcessName; }
        TString GetVolumeName() const { return fVolumeName; }

      private:
        int fNDaughters;
        double fEdep;
        double fProperTime;
        ROOT::Math::XYZTVector f4Position;
        ROOT::Math::PxPyPzEVector f4Momentum;
        TString fProcessName;
        TString fVolumeName;

        ClassDefOverride(simobj::Step, 1)
    };
}; // namespace simobj
#endif
