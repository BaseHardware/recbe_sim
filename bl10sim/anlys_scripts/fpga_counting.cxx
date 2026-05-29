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

void fpga_counting() {
    TFile *pOutput = new TFile("fcount.root", "RECREATE");
    TTree *pOTree  = new TTree("counts", "tree for the FPGA hits");
    pOTree->SetDirectory(pOutput);

    bool recbe[3];
    bool mkii[3];
    bool roesti[3];

    pOTree->Branch("recbe_0", recbe, "recbe_0/O");
    pOTree->Branch("recbe_1", recbe + 1, "recbe_1/O");
    pOTree->Branch("recbe_2", recbe + 2, "recbe_2/O");
    pOTree->Branch("mkii_0", mkii, "mkii_0/O");
    pOTree->Branch("mkii_1", mkii + 1, "mkii_1/O");
    pOTree->Branch("mkii_2", mkii + 2, "mkii_2/O");
    pOTree->Branch("roesti_0", roesti, "roesti_0/O");
    pOTree->Branch("roesti_1", roesti + 1, "roesti_1/O");
    pOTree->Branch("roesti_2", roesti + 2, "roesti_2/O");

    auto check_step = [&](const simobj::Step &s) -> bool {
        const TString &volName = s.GetVolumeName();
        if (volName.Contains("FPGADiePV")) {
            int envCopyNo = s.GetEnvelopeCopyNumber();
            if (volName.Contains("RECBE")) {
                recbe[envCopyNo] = true;
            } else if (volName.Contains("MkII")) {
                mkii[envCopyNo] = true;
            } else if (volName.Contains("ROESTI")) {
                roesti[envCopyNo] = true;
            } else {
                cout << "WTF!?!?!?!?!?" << endl;
                exit(1);
                return false;
            }
            return true;
        }
        return false;
    };

    auto clear_flags = [&]() -> void {
        for (size_t i = 0; i < 3; i++) {
            recbe[i]  = false;
            mkii[i]   = false;
            roesti[i] = false;
        }
    };

    TFile *pInput = new TFile("./noedepcond.root");
    TTree *pTree  = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcl_tracks = nullptr;
    TClonesArray *tcl_steps  = nullptr;

    pTree->SetBranchAddress("Tracks", &tcl_tracks);
    pTree->SetBranchAddress("Steps", &tcl_steps);

    int entries = pTree->GetEntries();
    for (int i = 0; i < entries; i++) {
        pTree->GetEntry(i);

        clear_flags();

        simobj::Track *primtrack = static_cast<simobj::Track *>(tcl_tracks->At(0));

        bool result = false;

        result |= check_step(primtrack->GetFirstStep());
        size_t nStep = primtrack->GetNStep();

        for (size_t j = 0; j < nStep; j++) {
            size_t idx = primtrack->GetStepIndex(j);

            simobj::Step *nowstep = static_cast<simobj::Step *>(tcl_steps->At(idx));
            result |= check_step(*nowstep);
        }

        result |= check_step(primtrack->GetFinalStep());

        if (!result) {
            cout << "FUUUUUUUUUUUUUUUUCK!!!!!!!!!!!" << endl;
            return;
        } else {
            pOTree->Fill();
        }
    }

    pOutput->Write();
    pOutput->Close();
}
