#include <algorithm>
#include <stdexcept>

#include "bl10sim/NeutronTimeGenerator.h"

#include "G4Exception.hh"
#include "G4ParticleTable.hh"
#include "G4ios.hh"

#include "Randomize.hh"

#define SQ(X) ((X) * (X))

constexpr double cSqrtHalfPi = 1.2533141373155002512; // sqrt(pi/2)
constexpr double cInvSqrt2   = 0.7071067811865475244; // 1/sqrt(2)

namespace bl10sim {
    NeutronTimeGenerator::NeutronTimeGenerator()
        : fCWSampler({.eMin_eV            = 0.001,
                      .eMax_eV            = 1.0e5,
                      .nE                 = 200,
                      .nU                 = 2048,
                      .allowExtrapolation = true}),
          fTimeOffset(0.35), fFirstBunchOffset(0), fBunchSeparation(0.6) {
        // double fwhm_200kW[2] = {0.127, 0.129}; // fwhm at an accelerator power of 200 kW
        double fwhm_300kW[2] = {0.130, 0.136}; // fwhm at an accelerator power of 300 kW
        SetFirstBunchFWHM(fwhm_300kW[0]);
        SetSecondBunchFWHM(fwhm_300kW[1]);
    }

    void NeutronTimeGenerator::SetBunchSeparation(double a) {
        if (a < 0) {
            G4ExceptionDescription msg;
            msg << "The separation time for the two bunches must be positive. (request: " << a
                << " us). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetBunchSeparation()", "NeutronTime0001",
                        JustWarning, msg);
        } else {
            fBunchSeparation = a;
        }
    }

    void NeutronTimeGenerator::SetBunchSigma(double a, int idx) {
        if (idx != 0 && idx != 1) {
            G4ExceptionDescription msg;
            msg << "The index for the SetBunchFWHM() must be 0 or 1. Bug in the code? (request: "
                << idx << " ). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetBunchFWHM()", "NeutronTime0002", JustWarning,
                        msg);
        } else if (a < 0) {
            G4ExceptionDescription msg;
            msg << "The FWHM value for the two bunches must be positive. (request: " << a
                << " us). This will be ignored." << G4endl;
            G4Exception("NeutronTimeGenerator::SetBunchFWHM()", "NeutronTime0001", JustWarning,
                        msg);
        } else {
            fBunchSigma[idx] = a;
        }
    }

    double NeutronTimeGenerator::Generate(double energy_eV, double dist_m) const {
        double retval = fTimeOffset;

        retval += BunchSeparation();

        if (energy_eV <= fCWSampler.GetMaxEnergy()) {
            retval += ModeratorDelay(energy_eV);
        } else {
            retval += SpallationDelay(energy_eV);
        }

        retval += DuctFlight(energy_eV, dist_m);

        return retval;
    }

    double NeutronTimeGenerator::BunchSeparation() const {
        using namespace std;

        double mean  = fFirstBunchOffset;
        double sigma = fBunchSigma[0];

        if (fBunchSeparation != 0 && G4UniformRand() > 0.5) {
            mean += fBunchSeparation;
            sigma = fBunchSigma[1];
        }

        return G4RandGauss::shoot(mean, sigma);
    }

    double NeutronTimeGenerator::SpallationDelay(double energy_eV) const {
        double mean   = fCWSampler.GetColeWindsor().Evaluate_t0(energy_eV);
        double sigma1 = fCWSampler.GetColeWindsor().Evaluate_s1(energy_eV);
        double sigma2 = fCWSampler.GetColeWindsor().Evaluate_s2(energy_eV);

        double s1_int = 0.5 - sigma1 * cSqrtHalfPi * (1 + std::erf(-mean / sigma1 * cInvSqrt2));
        double s2_int = 0.5;
        if (G4UniformRand() < s1_int / (s1_int + s2_int)) {
            return mean - abs(G4RandGauss::shoot(0, sigma1));
        } else {
            return mean + abs(G4RandGauss::shoot(0, sigma2));
        }
    }

    double NeutronTimeGenerator::ModeratorDelay(double energy_eV) const {
        return fCWSampler.Sample(energy_eV);
    }

    double NeutronTimeGenerator::DuctFlight(double energy_eV, double dist_m) const {
        static G4ParticleDefinition *neutronDef = nullptr;
        if (neutronDef == nullptr) {
            neutronDef = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        }

        double cla_t = dist_m * sqrt(neutronDef->GetPDGMass() / (2 * energy_eV));
        return cla_t;
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

        G4cout << "Building a table for the neutron emission time..." << G4endl;
        BuildTables();
        G4cout << "Finished." << G4endl;
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

            const double energy_eV = std::exp(fLogEGrid[iE]);
            fCWFunc.UpdateParameters(energy_eV);

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
