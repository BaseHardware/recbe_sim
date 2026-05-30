#include <random>

#include "kobetdsim/ActionInitialization.h"
#include "kobetdsim/DetectorConstruction.h"

#include "simcore/MetadataManager.h"
#include "simcore/SafeTermination.h"

#include "QGSP_BIC_AllHP.hh"

#include "G4EmParameters.hh"
#include "G4SystemOfUnits.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4ThermalNeutrons.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "G4NuclideTable.hh"

#include "Randomize.hh"

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char **argv) {
    simcore::SafeTermination::RegisterSignalHandler();
    std::random_device rng;
    long seeds[2] = {rng(), rng()};
    // Detect interactive mode (if no arguments) and define UI session
    //
    G4UIExecutive *ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    auto &mdinstance = simcore::MetadataManager::GetInstance();
    mdinstance.SetSimulationName("kobetd_mainsim");

    mdinstance.SetRandomSeed(seeds[0], false);
    mdinstance.SetRandomSeed(seeds[1], true);
    G4Random::setTheSeeds(seeds);

    // Optionally: choose a different Random engine...
    // G4Random::setTheEngine(new CLHEP::MTwistEngine);

    // use G4SteppingVerboseWithUnits
    G4int precision = 4;
    G4SteppingVerbose::UseBestUnit(precision);

    // Construct the default run manager
    //
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    // Set mandatory initialization classes
    //
    runManager->SetUserInitialization(new kobetdsim::DetectorConstruction());

    // auto physicsList = new QGSP_BIC_HP;
    auto physicsList = new QGSP_BIC_AllHP;
    // G4VModularPhysicsList* physicsList = new Shielding(true);

    G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0. * s);
    physicsList->RegisterPhysics(new G4ThermalNeutrons());
    runManager->SetUserInitialization(physicsList);

    G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0. * s);
    G4NuclideTable::GetInstance()->SetMeanLifeThreshold(0. * s);
    auto empar = G4EmParameters::Instance();
    empar->SetFluo(true);
    empar->SetAuger(true);
    empar->SetAugerCascade(true);
    empar->SetDeexcitationIgnoreCut(true);
    // Set user action classes
    runManager->SetUserInitialization(new kobetdsim::ActionInitialization());

    // Initialize visualization with the default graphics system
    auto visManager = new G4VisExecutive(argc, argv);
    // Constructors can also take optional arguments:
    // - a graphics system of choice, eg. "OGL"
    // - and a verbosity argument - see /vis/verbose guidance.
    // auto visManager = new G4VisExecutive(argc, argv, "OGL", "Quiet");
    // auto visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();

    // Get the pointer to the User Interface manager
    auto UImanager = G4UImanager::GetUIpointer();

    // Process macro or start UI session
    //
    UImanager->ApplyCommand("/control/macroPath macros");
    if (!ui) {
        // batch mode
        G4String command  = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // interactive mode
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        if (ui->IsGUI()) {
            UImanager->ApplyCommand("/control/execute gui.mac");
        }
        ui->SessionStart();
        delete ui;
    }

    // Job termination
    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager, so they should not be deleted
    // in the main() program !
    //
    delete visManager;
    delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
