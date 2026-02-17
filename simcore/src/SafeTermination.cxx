#include "simcore/SafeTermination.h"

#include "G4RunManager.hh"
#include "G4StateManager.hh"
#include "G4Threading.hh"

#include <iostream>

#include <unistd.h>

void my_local_signal_handler(int signum) {
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
            simcore::SafeTermination::GetInstance().Terminate();
            break;
        case G4State_PreInit:
        case G4State_Init:
        case G4State_Idle:
        case G4State_Quit:
        case G4State_Abort:
            cout << " Exit the application RIGHT NOW." << endl;
            _exit(0);
            break;
    }
}

namespace simcore {
    bool SafeTermination::fgRegistered = false;

    SafeTermination &SafeTermination::GetInstance() {
        static SafeTermination fgInstance;
        return fgInstance;
    }

    bool SafeTermination::IsTerminated() const { return (fgStopFlag != 0); }

    void SafeTermination::RegisterSignalHandler() {
        if (G4Threading::IsWorkerThread()) return;
        std::signal(SIGTERM, my_local_signal_handler);
        std::signal(SIGINT, my_local_signal_handler);
        fgRegistered = true;
    }

    void SafeTermination::DeregisterSignalHandler() {
        if (G4Threading::IsWorkerThread()) return;
        std::signal(SIGTERM, SIG_DFL);
        std::signal(SIGINT, SIG_DFL);
        fgRegistered = false;
    }

    void SafeTermination::RestoreSignalHandler() {
        if (!fgRegistered) return;

        std::signal(SIGINT, my_local_signal_handler);
    }
} // namespace simcore
