#include <array>
#include <cmath>
#include <iostream>

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2F.h"
#include "THStack.h"
#include "TTree.h"

using namespace std;

template <size_t N>
constexpr array<double, N + 1> build_logbins(double xmin, double xmax) {
    array<double, N + 1> ebins;

    double log_binwidth = (log10(xmax) - log10(xmin)) / N;

    for (size_t i = 0; i < N; i++) {
        ebins[i] = xmin * pow(10, log_binwidth * i);
    }
    ebins[N] = xmax;

    return ebins;
}

void draw_prim2d() {
    constexpr size_t nbins = 200;

    // auto ebins = build_logbins<nbins>(1e-10, 50);
    // auto tbins = build_logbins<nbins>(1e2, 1e8);
    auto ebins = build_logbins<nbins>(1e-3, 50);
    auto tbins = build_logbins<nbins>(1e3, 1e5);

    TFile *pInput = new TFile("./big_output.root");
    TTree *pTree = static_cast<TTree *>(pInput->Get("hits"));

    double prim_e, prim_t;
    int trkCharge, stepCharge;

    pTree->SetBranchAddress("prim_e", &prim_e);
    pTree->SetBranchAddress("prim_t", &prim_t);
    pTree->SetBranchAddress("trkCharge", &trkCharge);
    pTree->SetBranchAddress("stepCharge", &stepCharge);

    TH2F *hist =
        new TH2F("h1", "h1", ebins.size() - 1, ebins.data(), tbins.size() - 1, tbins.data());

    Int_t entries = pTree->GetEntries();

    for (Int_t i = 0; i < entries; i++) {
        pTree->GetEntry(i);
        if (trkCharge > 0) hist->Fill(prim_e, prim_t);
    }

    hist->Draw("colz");

    gPad->SetLogx();
    gPad->SetLogy();
}
