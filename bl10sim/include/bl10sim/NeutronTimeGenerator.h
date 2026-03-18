#ifndef __bl10sim_NeutronTimeGenerator_h__
#define __bl10sim_NeutronTimeGenerator_h__

#include <vector>

namespace bl10sim {
    class ColeWindsor {
      public:
        double operator()(double t) const { return PDF(t); }
        double operator()(double t, double e) { return PDF(t, e); }

        double PDF(double t) const;
        double PDF(double t, double en) {
            UpdateParameters(en);
            return PDF(t);
        }

        double CDF(double t) const { return IntegralZeroTo(t) / IntegralZeroTo(fIntXMax); }
        double CDF(double t, double en) {
            UpdateParameters(en);
            return CDF(t);
        }

        double GetCurrentXMax() const { return fIntXMax; }

        void UpdateParameters(double en, double xmax_multiplier = 10);

        double Evaluate_t0(double en) const;
        double Evaluate_s1(double en) const;
        double Evaluate_s2(double en) const;
        double Evaluate_g1(double en) const;
        double Evaluate_g2(double en) const;
        double Evaluate_R(double en) const;

      protected:
        double IntegralZeroTo(double t) const;

        double Integral(double lo, double hi) const;
        double Evaluate_C(double xmax) const { return 1. / IntegralZeroTo(xmax); }

      private:
        double fPrevE;
        double fNorm, fIntXMax, fFrac, fT0, fSigma1, fSigma2, fGamma1, fGamma2, fThres1, fThres2;
    };

    class ColeWindsorSampler {
      public:
        struct Config {
            // Paper-validated range: 1 eV -- 10 keV
            double eMin_eV = 1.0;
            double eMax_eV = 1.0e4;

            // Number of log-energy nodes
            std::size_t nE = 160;

            // Number of uniform-u quantile nodes
            std::size_t nU = 2048;

            // Avoid u=0,1 exactly because of infinite tails
            double epsU = 1.0e-6;

            // Extrapolation policy outside [eMin, eMax]:
            // false -> clamp to boundary
            // true  -> evaluate the paper formula outside the range at your own risk
            bool allowExtrapolation = false;
        };

        explicit ColeWindsorSampler(const Config &cfg);

        // Same as above, but input/output in eV / microsecond
        double Sample(double energy_eV) const;

      private:
        Config fConfig;
        std::vector<double> fLogEGrid;              // size nE
        std::vector<std::vector<double>> fCDFTable; // [nE][nU]

        double PrepareEnergy(double en) const { return std::max(en, 1e-12); }

        double InvertCDF(double u, double start_lo, double start_hi) const;
        void BuildTables();
        size_t FindEnergyBin(double logE) const;
        double InterpolateQuantileUs(std::size_t iE, double uEff) const;

        ColeWindsor fCWFunc;
    };

    class NeutronTimeGenerator {
      public:
        NeutronTimeGenerator();
        virtual ~NeutronTimeGenerator();

        void SetBunchSeparation(double a);

        double operator()(double en) const { return Generate(en); }
        double Generate(double en) const;

        double GetTimeOffset() const { return fTimeOffset; }
        double GetFirstBunchOffset() const { return fFirstBunchOffset; }
        double GetBunchSeparation() const { return fBunchSeparation; }
        double GetFirstBunchFWHM() const { return fBunchFWHM[0]; }
        double GetSecondBunchFWHM() const { return fBunchFWHM[1]; }

        void SetTimeOffset(double a) { fTimeOffset = a; }
        void SetFirstBunchOffset(double a) { fFirstBunchOffset = a; }
        void SetFirstBunchFWHM(double a) { SetFWHM(a, 0); }
        void SetSecondBunchFWHM(double a) { SetFWHM(a, 1); }

      protected:
        double BunchSeparation() const;
        double ModeratorDelay(double en) const;

      private:
        void SetFWHM(double a, int idx);

        double fTimeOffset;
        double fFirstBunchOffset;
        double fBunchSeparation;
        double fBunchFWHM[2];

        ColeWindsorSampler fCWSampler;
    };
}; // namespace bl10sim

#endif
