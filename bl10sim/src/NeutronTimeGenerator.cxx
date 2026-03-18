#include <algorithm>
#include <stdexcept>

#include "bl10sim/NeutronTimeGenerator.h"

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include "Randomize.hh"

#define SQ(X) ((X) * (X))

constexpr double cSqrtHalfPi = 1.2533141373155002512; // sqrt(pi/2)
constexpr double cInvSqrt2   = 0.7071067811865475244; // 1/sqrt(2)

namespace bl10sim {
    NeutronTimeGenerator::NeutronTimeGenerator()
        : fCWSampler({.eMin_eV            = 0.001,
                      .eMax_eV            = 1.0e4,
                      .nE                 = 200,
                      .nU                 = 2048,
                      .allowExtrapolation = true}),
          fTimeOffset(0), fFirstBunchOffset(0), fBunchSeparation(0.6) {
        // fBunchFWHM[0] = 0.127; // at an acceleration power of 200 kW
        // fBunchFWHM[1] = 0.129;
        fBunchFWHM[0] = 0.130; // at an acceleration power of 300 kW
        fBunchFWHM[1] = 0.136;
    }

    void NeutronTimeGenerator::SetBunchSeparation(double a) {
        a /= us;
        if (a < 0) {
            G4ExceptionDescription msg;
            msg << "The separation time for the two bunches must be positive. (request: " << a / us
                << " us). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetBunchSeparation()", "NeutronTime0001",
                        JustWarning, msg);
        } else {
            fBunchSeparation = a;
        }
    }

    void NeutronTimeGenerator::SetFWHM(double a, int idx) {
        a /= us;
        if (idx != 0 && idx != 1) {
            G4ExceptionDescription msg;
            msg << "The index for the SetFWHM() must be 0 or 1. Bug in the code? (request: " << idx
                << " ). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetFWHM()", "NeutronTime0002", JustWarning, msg);
        } else if (a < 0) {
            G4ExceptionDescription msg;
            msg << "The FWHM value for the two bunches must be positive. (request: " << a
                << " us). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetFWHM()", "NeutronTime0001", JustWarning, msg);
        } else {
            fBunchFWHM[idx] = a;
        }
    }

    // 1eV 이하라면 Bunch 고려 안 하되, 일단 아래 PDF로 생성
    // 1eV <-> 10 keV라면 Bunch와 아래 PDF를 동시에 고려할 것
    // 10 keV 이상이라면 Bunch 영향만 고려?
    double NeutronTimeGenerator::Generate(double en) const {
        double retval = fTimeOffset;
        en /= eV;

        if (en > 1) retval += 0.;

        return retval;
    }

    double NeutronTimeGenerator::BunchSeparation() const {
        using namespace std;

        const double firstSigma  = fBunchFWHM[0] * 2 * sqrt(2 * log(2));
        const double secondSigma = fBunchFWHM[1] * 2 * sqrt(2 * log(2));

        double mean  = fFirstBunchOffset;
        double sigma = firstSigma;

        if (fBunchSeparation != 0 && G4UniformRand() > 0.5) {
            mean += fBunchSeparation;
            sigma = secondSigma;
        }

        return G4RandGauss::shoot(mean, sigma);
    }

    double NeutronTimeGenerator::ModeratorDelay(double en) const {
        return fCWSampler.Sample(en / eV) * us;
    }

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

    void ColeWindsor::UpdateParameters(double en, double xmax_multiplier) {
        if (en == fPrevE) {
            return;
        } else {
            fT0      = Evaluate_t0(en);
            fSigma1  = Evaluate_s1(en);
            fSigma2  = Evaluate_s2(en);
            fGamma1  = Evaluate_g1(en);
            fGamma2  = Evaluate_g2(en);
            fThres1  = fGamma1 * SQ(fSigma2);
            fThres2  = fGamma2 * SQ(fSigma2);
            fFrac    = std::clamp(Evaluate_R(en), 0.0, 1.0);
            fIntXMax = fT0 + std::max(fThres1 + xmax_multiplier * fGamma1,
                                      fThres2 + xmax_multiplier * fGamma2);
            fNorm    = Evaluate_C(fIntXMax);
        }
    }

    double ColeWindsor::Evaluate_t0(double en) const {
        return 2.27e-2 + 2.030 * std::pow(en, -0.460);
    }
    double ColeWindsor::Evaluate_s1(double en) const {
        return 6.80e-3 + 0.658 * std::pow(en, -0.468);
    }
    double ColeWindsor::Evaluate_s2(double en) const {
        return 3.15e-2 + 1.710 * std::pow(en, -0.476);
    }
    double ColeWindsor::Evaluate_g1(double en) const {
        return 2.95e-2 + 0.905 * std::pow(en, 0.343);
    }
    double ColeWindsor::Evaluate_g2(double en) const {
        return 6.78e-2 + 9.77e-2 * std::pow(en, 0.447);
    }
    double ColeWindsor::Evaluate_R(double en) const {
        return 0.404 - 0.290 * std::exp(-2.78e-4 * en);
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
                       (std::erf(dthi / fSigma1 * cInvSqrt2) -
                        std::erf(dtlo / fSigma1 * cInvSqrt2));
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

        BuildTables();
    }

    double ColeWindsorSampler::Sample(double energy_eV) const {
        const double logE    = std::log(PrepareEnergy(energy_eV));
        const std::size_t iE = FindEnergyBin(logE);
        const double xE      = (logE - fLogEGrid[iE]) / (fLogEGrid[iE + 1] - fLogEGrid[iE]);

        const double uRaw = G4UniformRand();
        const double uEff = fConfig.epsU + (1.0 - 2.0 * fConfig.epsU) * uRaw;

        const double t0 = InterpolateQuantileUs(iE, uEff);
        const double t1 = InterpolateQuantileUs(iE + 1, uEff);

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

            const double en = std::exp(fLogEGrid[iE]);
            fCWFunc.UpdateParameters(en);

            for (std::size_t iU = 0; iU < fConfig.nU; ++iU) {
                const double fu = static_cast<double>(iU) / static_cast<double>(fConfig.nU - 1);
                const double u  = fConfig.epsU + (1.0 - 2.0 * fConfig.epsU) * fu; // uniform-u grid
                fCDFTable[iE][iU] = InvertCDF(u, 0, fCWFunc.GetCurrentXMax());
            }
        }
    }

    double ColeWindsorSampler::InterpolateQuantileUs(size_t iE, double uEff) const {
        const double fu = (uEff - fConfig.epsU) / (1.0 - 2.0 * fConfig.epsU) *
                          static_cast<double>(fConfig.nU - 1);

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
} // namespace bl10sim
