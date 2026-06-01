#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"

using namespace std;

void draw_hittime() {
    TFile *pInput = new TFile("./big_output.root");
    TTree *pTree = static_cast<TTree *>(pInput->Get("hits"));

    double time, prim_t;
    int trkCharge, stepCharge;

    pTree->SetBranchAddress("time", &time);
    pTree->SetBranchAddress("prim_t", &prim_t);
    pTree->SetBranchAddress("trkCharge", &trkCharge);
    pTree->SetBranchAddress("stepCharge", &stepCharge);

    TH1D *trkHist = new TH1D("hTrk", "hTrk", 60, 2000, 8000);
    TH1D *stepHist = new TH1D("hStep", "hStep", 60, 2000, 8000);

    Int_t entries = pTree->GetEntries();

    for (Int_t i = 0; i < entries; i++) {
        pTree->GetEntry(i);
        if (trkCharge > 0) trkHist->Fill(time);
        if (stepCharge > 0) stepHist->Fill(time);
    }

    trkHist->GetXaxis()->SetTitle("Hittime(ns)");
    stepHist->GetXaxis()->SetTitle("Hittime(ns)");

    new TCanvas();
    trkHist->Draw("");
    new TCanvas();
    stepHist->Draw("");
}
