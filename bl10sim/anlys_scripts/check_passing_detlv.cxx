#include <iomanip>
#include <iostream>
#include <limits>

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

void check_passing_detlv(const char *input_file  = "./simout.root",
                         const char *output_file = "output.root") {
    TFile *pOutput = new TFile(output_file, "RECREATE");
    TTree *pOTree  = new TTree("incident", "tree for incident particles");
    pOTree->SetDirectory(pOutput);

    Int_t copy_num, pdg, id;
    Double_t x, y, z, t, px, py, pz, e;

    pOTree->Branch("id", &id, "id/I");
    pOTree->Branch("pdg", &pdg, "pdg/I");
    pOTree->Branch("copynum", &copy_num, "copynum/I");
    pOTree->Branch("x", &x, "x/D");
    pOTree->Branch("y", &y, "y/D");
    pOTree->Branch("z", &z, "z/D");
    pOTree->Branch("t", &t, "t/D");
    pOTree->Branch("px", &px, "px/D");
    pOTree->Branch("py", &py, "py/D");
    pOTree->Branch("pz", &pz, "pz/D");
    pOTree->Branch("e", &e, "e/D");

    TFile *pInput = new TFile(input_file);

    TTree *pITree = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    pITree->SetBranchAddress("Steps", &tcaStep);
    pITree->SetBranchAddress("Tracks", &tcaTrack);
    pITree->SetBranchAddress("Primary", &primary);

    int n_evts = pITree->GetEntries();

    for (int i = 0; i < n_evts; i++) {
        pITree->GetEntry(i);

        int n_trk = tcaTrack->GetEntries();

        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));

            for (int idx_step = 0; idx_step < now_track->GetNStep(); idx_step++) {
                simobj::Step *now_step =
                    static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));

                const auto &volName = now_step->GetVolumeName();

                if (volName == "DetectorPV") {
                    if (idx_step == 0) break;

                    simobj::Step *prev_step = static_cast<simobj::Step *>(
                        tcaStep->At(now_track->GetStepIndex(idx_step - 1)));

                    if (prev_step->GetVolumeName() != volName) {
                        id       = now_track->GetTrackID();
                        pdg      = now_track->GetPDGCode();
                        copy_num = now_step->GetCopyNumber();
                        x        = now_step->GetX();
                        y        = now_step->GetY();
                        z        = now_step->GetZ();
                        t        = now_step->GetGlobalTime();
                        px       = now_step->GetPx();
                        py       = now_step->GetPy();
                        pz       = now_step->GetPz();
                        e        = now_step->GetKineticEnergy();

                        pOTree->Fill();
                    }
                }
            }
        }
    }

    pInput->Close();
    pOutput->Write();
    pOutput->Close();
}
