#include "simcore/SafeTermination.h"

#include "G4RunManager.hh"
#include "G4StateManager.hh"

#include <csignal>
#include <iostream>

void __local_signal_handler(int signum) {
    using namespace std;
    if (signum == SIGTERM) {
        cout << "SIGTERM received.";
    } else if (signum == SIGINT) {
        cout << "\nSIGINT (Ctrl+C) received.";
    }
    auto nowState = G4StateManager::GetStateManager()->GetCurrentState();
    switch (nowState) {
        case G4State_GeomClosed:
        case G4State_EventProc:
            cout << " The simulation will be aborted as soon as possible..." << endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            break;
        case G4State_PreInit:
        case G4State_Init:
        case G4State_Idle:
        case G4State_Quit:
        case G4State_Abort:
            cout << " Exit the application RIGHT NOW." << endl;
            exit(0);
            break;
    }
}

namespace simcore {
    void SafeTermination::RegisterSignalHandler() {
        std::signal(SIGTERM, __local_signal_handler);
        std::signal(SIGINT, __local_signal_handler);
    }

    void SafeTermination::DeregisterSignalHandler() {
        std::signal(SIGTERM, SIG_DFL);
        std::signal(SIGINT, SIG_DFL);
    }
} // namespace simcore
