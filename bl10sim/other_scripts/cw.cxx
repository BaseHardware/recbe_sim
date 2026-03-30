#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "TRandom3.h"

using namespace std;

#define SQ(X) ((X) * (X))

constexpr double cSqrtHalfPi = 1.2533141373155002512; // sqrt(pi/2)
constexpr double cInvSqrt2   = 0.7071067811865475244; // 1/sqrt(2)

// 1eV 이하라면 Bunch 고려 안 하되, 일단 아래 PDF로 생성
// 1eV <-> 10 keV라면 Bunch와 아래 PDF를 동시에 고려할 것
// 10 keV 이상이라면 Bunch 영향만 고려?
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
    double GetCurrentT0() const { return fT0; }
    double GetCurrentThreshold1() const { return fThres1; }
    double GetCurrentThreshold2() const { return fThres2; }
    double GetCurrentFraction() const { return fFrac; }

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

    double GetMinEnergy() const { return fConfig.eMin_eV; }
    double GetMaxEnergy() const { return fConfig.eMax_eV; }

    const ColeWindsor &GetColeWindsor() const { return fCWFunc; }

  private:
    Config fConfig;
    std::vector<double> fLogEGrid;              // size nE
    std::vector<std::vector<double>> fCDFTable; // [nE][nU]

    double PrepareEnergy(double en) const { return std::max(en, 1e-12); }

    double InvertCDF(double u, double start_lo, double start_hi) const;
    void BuildTables();
    size_t FindEnergyBin(double logE) const;
    double InterpolateQuantile(std::size_t iE, double uEff) const;

    ColeWindsor fCWFunc;
};

double ColeWindsor::PDF(double t) const {
    double dt = t - fT0;

    double f1, f2;
    if (t <= fT0) {
        f1 = f2 = std::exp(-0.5 * SQ(dt / fSigma1));
    } else {
        if (fT0 < t && t < fT0 + fThres1) {
            f1 = std::exp(-0.5 * SQ(dt / fSigma2));
        } else {
            f1 = std::exp(0.5 * fGamma1 * fThres1 - fGamma1 * dt);
        }

        if (fT0 < t && t < fT0 + fThres2) {
            f2 = std::exp(-0.5 * SQ(dt / fSigma2));
        } else {
            f2 = std::exp(0.5 * fGamma2 * fThres2 - fGamma2 * dt);
        }
    }

    return fNorm * ((1 - fFrac) * f1 + fFrac * f2);
}

void ColeWindsor::UpdateParameters(double energy_eV, double xmax_multiplier) {
    if (energy_eV == fPrevE) {
        return;
    } else {
        fT0      = Evaluate_t0(energy_eV);
        fSigma1  = Evaluate_s1(energy_eV);
        fSigma2  = Evaluate_s2(energy_eV);
        fGamma1  = Evaluate_g1(energy_eV);
        fGamma2  = Evaluate_g2(energy_eV);
        fThres1  = fGamma1 * SQ(fSigma2);
        fThres2  = fGamma2 * SQ(fSigma2);
        fFrac    = std::clamp(Evaluate_R(energy_eV), 0.0, 1.0);
        fIntXMax = fT0 + std::max(fThres1 + xmax_multiplier * fGamma1,
                                  fThres2 + xmax_multiplier * fGamma2);
        fNorm    = Evaluate_C(fIntXMax);
    }
}

double ColeWindsor::Evaluate_t0(double energy_eV) const {
    return 2.27e-2 + 2.030 * std::pow(energy_eV, -0.460);
}
double ColeWindsor::Evaluate_s1(double energy_eV) const {
    return 6.80e-3 + 0.658 * std::pow(energy_eV, -0.468);
}
double ColeWindsor::Evaluate_s2(double energy_eV) const {
    return 3.15e-2 + 1.710 * std::pow(energy_eV, -0.476);
}
double ColeWindsor::Evaluate_g1(double energy_eV) const {
    return 2.95e-2 + 0.905 * std::pow(energy_eV, 0.343);
}
double ColeWindsor::Evaluate_g2(double energy_eV) const {
    return 6.78e-2 + 9.77e-2 * std::pow(energy_eV, 0.447);
}
double ColeWindsor::Evaluate_R(double energy_eV) const {
    return 0.404 - 0.290 * std::exp(-2.78e-4 * energy_eV);
}

double ColeWindsor::IntegralZeroTo(double t) const {
    double dt = t - fT0;

    double retval = -fSigma1 * cSqrtHalfPi * (1 + std::erf(-fT0 / fSigma1 * cInvSqrt2));

    if (dt <= 0) {
        retval += fSigma1 * cSqrtHalfPi * (1 + std::erf(dt / fSigma1 * cInvSqrt2));
    } else {
        retval += fSigma1 * cSqrtHalfPi;

        double i1, i2;
        i1 = i2 = 0;

        if (dt < fThres1) {
            i1 += fSigma2 * cSqrtHalfPi * std::erf(dt / fSigma2 * cInvSqrt2);
        } else {
            i1 += fSigma2 * cSqrtHalfPi * std::erf(fThres1 / fSigma2 * cInvSqrt2);
            i1 += std::exp(0.5 * fGamma1 * fThres1) *
                  (std::exp(-fGamma1 * fThres1) - std::exp(-fGamma1 * dt)) / fGamma1;
        }
        retval += i1 * (1 - fFrac);

        if (dt < fThres2) {
            i2 += fSigma2 * cSqrtHalfPi * std::erf(dt / fSigma2 * cInvSqrt2);
        } else {
            i2 += fSigma2 * cSqrtHalfPi * std::erf(fThres2 / fSigma2 * cInvSqrt2);
            i2 += std::exp(0.5 * fGamma2 * fThres2) *
                  (std::exp(-fGamma2 * fThres2) - std::exp(-fGamma2 * dt)) / fGamma2;
        }
        retval += i2 * fFrac;
    }

    return retval;
}

double ColeWindsor::Integral(double lo, double hi) const {
    if (lo == hi) {
        return 0;
    } else if (hi < lo) {
        return -Integral(hi, lo);
    } else {
        double dthi = hi - fT0;
        double dtlo = lo - fT0;
        if (hi <= 0) {
            return fSigma1 * cSqrtHalfPi *
                   (std::erf(dthi / fSigma1 * cInvSqrt2) - std::erf(dtlo / fSigma1 * cInvSqrt2));
        } else {
            if (lo <= 0) {
                return IntegralZeroTo(hi) + fSigma1 * cSqrtHalfPi *
                                                (std::erf(dtlo / fSigma1 * cInvSqrt2) -
                                                 std::erf(-fT0 / fSigma1 * cInvSqrt2));
            } else {
                return IntegralZeroTo(hi) - IntegralZeroTo(lo);
            }
        }
    }
}

ColeWindsorSampler::ColeWindsorSampler(const Config &cfg) : fConfig(cfg) {
    if (fConfig.nE < 2) {
        throw std::runtime_error("ColeWindsorSampler: nE must be >= 2");
    }
    if (fConfig.nU < 2) {
        throw std::runtime_error("ColeWindsorSampler: nU must be >= 2");
    }
    if (!(fConfig.eMin_eV > 0.0 && fConfig.eMax_eV > fConfig.eMin_eV)) {
        throw std::runtime_error("ColeWindsorSampler: invalid energy range");
    }
    if (!(fConfig.epsU > 0.0 && fConfig.epsU < 0.5)) {
        throw std::runtime_error("ColeWindsorSampler: epsU must be in (0, 0.5)");
    }

    cout << "Building a table for the neutron emission time..." << endl;
    BuildTables();
    cout << "Finished." << endl;
}

double ColeWindsorSampler::Sample(double energy_eV) const {
    const double logE    = std::log(PrepareEnergy(energy_eV));
    const std::size_t iE = FindEnergyBin(logE);
    const double xE      = (logE - fLogEGrid[iE]) / (fLogEGrid[iE + 1] - fLogEGrid[iE]);

    const double uRaw = gRandom->Uniform();
    const double uEff = fConfig.epsU + (1.0 - 2.0 * fConfig.epsU) * uRaw;

    const double t0 = InterpolateQuantile(iE, uEff);
    const double t1 = InterpolateQuantile(iE + 1, uEff);

    return (1.0 - xE) * t0 + xE * t1;
}

size_t ColeWindsorSampler::FindEnergyBin(double logE) const {
    if (logE <= fLogEGrid.front()) return 0;
    if (logE >= fLogEGrid.back()) return fLogEGrid.size() - 2;

    auto it         = std::upper_bound(fLogEGrid.begin(), fLogEGrid.end(), logE);
    std::size_t idx = static_cast<std::size_t>(std::distance(fLogEGrid.begin(), it)) - 1;
    if (idx >= fLogEGrid.size() - 1) idx = fLogEGrid.size() - 2;
    return idx;
}

void ColeWindsorSampler::BuildTables() {
    fLogEGrid.resize(fConfig.nE);
    fCDFTable.assign(fConfig.nE, std::vector<double>(fConfig.nU, 0.0));

    const double logEmin = std::log(fConfig.eMin_eV);
    const double logEmax = std::log(fConfig.eMax_eV);

    for (std::size_t iE = 0; iE < fConfig.nE; ++iE) {
        const double fE = static_cast<double>(iE) / static_cast<double>(fConfig.nE - 1);
        fLogEGrid[iE]   = logEmin + (logEmax - logEmin) * fE;

        const double energy_eV = std::exp(fLogEGrid[iE]);
        fCWFunc.UpdateParameters(energy_eV);

        for (std::size_t iU = 0; iU < fConfig.nU; ++iU) {
            const double fu   = static_cast<double>(iU) / static_cast<double>(fConfig.nU - 1);
            const double u    = fConfig.epsU + (1.0 - 2.0 * fConfig.epsU) * fu; // uniform-u grid
            fCDFTable[iE][iU] = InvertCDF(u, 0, fCWFunc.GetCurrentXMax());
        }
    }
}

double ColeWindsorSampler::InterpolateQuantile(size_t iE, double uEff) const {
    const double fu =
        (uEff - fConfig.epsU) / (1.0 - 2.0 * fConfig.epsU) * static_cast<double>(fConfig.nU - 1);

    std::size_t i0 = static_cast<std::size_t>(std::floor(fu));
    if (i0 >= fConfig.nU - 1) i0 = fConfig.nU - 2;
    const std::size_t i1 = i0 + 1;

    const double du = fu - static_cast<double>(i0);

    const double q0 = fCDFTable[iE][i0];
    const double q1 = fCDFTable[iE][i1];
    return (1.0 - du) * q0 + du * q1;
}

double ColeWindsorSampler::InvertCDF(double u, double start_lo, double start_hi) const {
    double lo = start_lo;
    double hi = start_hi;

    const double width = hi - lo;
    // Ensure bracket really contains u
    while (fCWFunc.CDF(lo) > u) {
        lo -= width;
    }
    while (fCWFunc.CDF(hi) < u) {
        hi += width;
    }

    // Bisection is robust enough since table build is offline/init-time work
    for (int iter = 0; iter < 80; ++iter) {
        const double mid = 0.5 * (lo + hi);
        const double c   = fCWFunc.CDF(mid);
        if (c < u)
            lo = mid;
        else
            hi = mid;
    }
    return 0.5 * (lo + hi);
}

