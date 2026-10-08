#include "bl10sim/MicroElecRegionPhysics.h"

#include "G4Alpha.hh"
#include "G4BetheBlochModel.hh"
#include "G4BraggIonModel.hh"
#include "G4BraggModel.hh"
#include "G4DummyModel.hh"
#include "G4Electron.hh"
#include "G4EmConfigurator.hh"
#include "G4GenericIon.hh"
#include "G4IonFluctuations.hh"
#include "G4LossTableManager.hh"
#include "G4MicroElecElastic.hh"
#include "G4MicroElecElasticModel_new.hh"
#include "G4MicroElecInelastic.hh"
#include "G4MicroElecInelasticModel.hh"
#include "G4MicroElecInelasticModel_new.hh"
#include "G4MollerBhabhaModel.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4Proton.hh"
#include "G4UniversalFluctuation.hh"
#include "G4UrbanMscModel.hh"

#include "G4SystemOfUnits.hh"

static G4bool HasMsc(G4ProcessManager *pm) {
    G4bool res          = false;
    G4ProcessVector *pv = pm->GetProcessList();
    G4int nproc         = pm->GetProcessListLength();
    for (G4int i = 0; i < nproc; ++i) {
        if (((*pv)[i])->GetProcessSubType() == fMultipleScattering) {
            res = true;
            break;
        }
    }
    return res;
}

namespace bl10sim {
    void MicroElecRegionPhysics::ConstructProcess() {
        std::size_t nreg = fTargetRegions.size();
        if (0 == nreg) {
            return;
        }
        G4LossTableManager *man = G4LossTableManager::Instance();

        const G4ParticleDefinition *elec = G4Electron::Electron();
        const G4ParticleDefinition *prot = G4Proton::Proton();
        const G4ParticleDefinition *alph = G4Alpha::Alpha();
        const G4ParticleDefinition *gion = G4GenericIon::GenericIon();
        G4ProcessManager *eman           = elec->GetProcessManager();
        G4ProcessManager *pman           = prot->GetProcessManager();
        G4ProcessManager *aman           = alph->GetProcessManager();
        G4ProcessManager *iman           = gion->GetProcessManager();

        G4bool emsc = HasMsc(eman);

        // MicroElec elastic is not active in the world
        G4MicroElecElastic *eElasProc = new G4MicroElecElastic("e-G4MicroElecElastic");
        eman->AddDiscreteProcess(eElasProc);

        G4MicroElecInelastic *eInelProc = new G4MicroElecInelastic("e-G4MicroElecInelastic");
        eman->AddDiscreteProcess(eInelProc);

        G4MicroElecInelastic *pInelProc = new G4MicroElecInelastic("p_G4MicroElecInelastic");
        pman->AddDiscreteProcess(pInelProc);

        G4MicroElecInelastic *aInelProc = new G4MicroElecInelastic("alpha_G4MicroElecInelastic");
        aman->AddDiscreteProcess(aInelProc);

        G4MicroElecInelastic *iInelProc = new G4MicroElecInelastic("ion_G4MicroElecInelastic");
        iman->AddDiscreteProcess(iInelProc);

        // start configuration of models
        G4EmConfigurator *em_config = man->EmConfigurator();
        G4VEmModel *mod;

        // limits for MicroElec applicability
        G4double eminel = 0.1 * eV;
        G4double emaxel = 500 * keV;
        G4double eminin = 2. * eV;
        G4double emaxin = 10. * MeV;
        G4double pmin   = 100 * eV;
        G4double pmax   = 10 * MeV;
        G4double amin   = 200 * keV;
        G4double amax   = 40 * MeV;
        G4double imin   = 28 * 50 * keV; // only for Si recoiling
        G4double imax   = 10 * MeV;

        for (std::size_t i = 0; i < nreg; ++i) {
            G4String reg = fTargetRegions[i];
            G4cout << "### MicroElec models are activated for G4Region " << reg << G4endl
                   << "    Energy limits for e- elastic:    " << eminel / eV << " eV - "
                   << emaxel / MeV << " MeV" << G4endl
                   << "    Energy limits for e- inelastic:  " << eminin / eV << " eV - "
                   << emaxin / MeV << " MeV" << G4endl
                   << "    Energy limits for protons:  " << pmin / eV << " eV - " << pmax / MeV
                   << " MeV" << G4endl << "    Energy limits for alphas:  " << amin / keV
                   << " keV - " << amax / MeV << " MeV" << G4endl
                   << "    Energy limits for ions:  " << imin / keV << " keV - " << imax / MeV
                   << " MeV" << G4endl;

            // e-
            if (emsc) {
                G4UrbanMscModel *msc = new G4UrbanMscModel();
                msc->SetActivationLowEnergyLimit(emaxel);
                em_config->SetExtraEmModel("e-", "msc", msc, reg);
            } else {
                mod = new G4DummyModel();
                em_config->SetExtraEmModel("e-", "CoulombScat", mod, reg, 0.0, emaxel);
            }

            mod = new G4MicroElecElasticModel_new();
            em_config->SetExtraEmModel("e-", "e-G4MicroElecElastic", mod, reg, eminel, emaxel);

            mod = new G4MollerBhabhaModel();
            mod->SetActivationLowEnergyLimit(emaxin);
            em_config->SetExtraEmModel("e-", "eIoni", mod, reg, 0.0, 10 * TeV,
                                       new G4UniversalFluctuation());

            mod = new G4MicroElecInelasticModel_new();
            em_config->SetExtraEmModel("e-", "e-G4MicroElecInelastic", mod, reg, eminin, emaxin);

            // proton
            mod = new G4BraggModel();
            mod->SetActivationHighEnergyLimit(pmin);
            em_config->SetExtraEmModel("proton", "hIoni", mod, reg, 0.0, 2 * MeV,
                                       new G4UniversalFluctuation());

            mod = new G4BetheBlochModel();
            mod->SetActivationLowEnergyLimit(pmax);
            em_config->SetExtraEmModel("proton", "hIoni", mod, reg, 2 * MeV, 10 * TeV,
                                       new G4UniversalFluctuation());

            mod = new G4MicroElecInelasticModel_new();
            em_config->SetExtraEmModel("proton", "p_G4MicroElecInelastic", mod, reg, pmin, pmax);

            // alpha
            mod = new G4BraggIonModel();
            mod->SetActivationHighEnergyLimit(amin);
            em_config->SetExtraEmModel("alpha", "ionIoni", mod, reg, 0.0, 2 * MeV,
                                       new G4IonFluctuations());

            mod = new G4BetheBlochModel();
            mod->SetActivationLowEnergyLimit(amax);
            em_config->SetExtraEmModel("alpha", "ionIoni", mod, reg, 2 * MeV, 10 * TeV,
                                       new G4IonFluctuations());

            mod = new G4MicroElecInelasticModel();
            em_config->SetExtraEmModel("alpha", "alpha_G4MicroElecInelastic", mod, reg, amin, amax);

            // ions
            mod = new G4BraggIonModel();
            mod->SetActivationHighEnergyLimit(imin);
            em_config->SetExtraEmModel("GenericIon", "ionIoni", mod, reg, 0.0, 2 * MeV,
                                       new G4IonFluctuations());

            mod = new G4BetheBlochModel();
            mod->SetActivationLowEnergyLimit(imax);
            em_config->SetExtraEmModel("GenericIon", "ionIoni", mod, reg, 2 * MeV, 10 * TeV,
                                       new G4IonFluctuations());

            mod = new G4MicroElecInelasticModel();
            em_config->SetExtraEmModel("GenericIon", "ion_G4MicroElecInelastic", mod, reg, imin,
                                       imax);
        }
    }
} // namespace bl10sim
