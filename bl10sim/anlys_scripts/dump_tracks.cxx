#include <iomanip>
#include <iostream>

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

void print_onestep(const simobj::Step *s) {
    cout << setw(9) << s->GetX() << "  " << setw(9) << s->GetY() << "  " << setw(9) << s->GetZ()
         << "  " << setw(12) << s->GetKineticEnergy() << "  " << setw(13) << s->GetDepositedEnergy()
         << "  " << setw(20) << s->GetVolumeName() << "  " << setw(9) << s->GetCopyNumber()
         << setw(9) << s->GetEnvelopeCopyNumber() << " " << setw(13) << " " << s->GetProcessName()
         << "  " << s->GetNDaughters() << endl;
}

void dump_tracks() {
    TFile *pInput = new TFile("./simout.root");

    TTree *pTree = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    pTree->SetBranchAddress("Steps", &tcaStep);
    pTree->SetBranchAddress("Tracks", &tcaTrack);
    pTree->SetBranchAddress("Primary", &primary);

    pTree->GetEntry(0);

    tcaTrack->GetEntries();

    size_t nTrack = tcaTrack->GetEntries();

    for (size_t idx_track = 0; idx_track < nTrack; idx_track++) {
        simobj::Track *nowtrk = static_cast<simobj::Track *>(tcaTrack->At(idx_track));

        cout << "Track #" << idx_track + 1 << ": " << nowtrk->GetName() << endl;
        size_t nStep = nowtrk->GetNStep();
        cout << " Step#         X          Y          Z           KineE         dEStep             "
                "    Volume        CopyNo       EnvCopyNo   Process  #daug"
             << endl;

        cout.precision(4);

        cout << fixed << " " << setw(5) << "START" << "   ";
        print_onestep(&nowtrk->GetFirstStep());
        for (size_t idx_step = 0; idx_step < nStep; idx_step++) {
            simobj::Step *nowstep =
                static_cast<simobj::Step *>(tcaStep->At(nowtrk->GetStepIndex(idx_step)));
            cout << fixed << " " << setw(5) << idx_step << "   ";
            print_onestep(nowstep);
        }
        cout << fixed << " " << setw(5) << "END" << "   ";
        print_onestep(&nowtrk->GetFinalStep());
    }
}
