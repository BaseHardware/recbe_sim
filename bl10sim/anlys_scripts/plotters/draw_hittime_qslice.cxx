#include <array>
#include <iomanip>
#include <iostream>

#include "TCanvas.h"
#include "TFile.h"
#include "THStack.h"
#include "TTree.h"

#include "./palette.h"

using namespace std;

// 0    :   0
// 0.1  :   625
// 0.5  :   3125
// 1    :   6250
// 2    :   12500
// 4    :   25000

void draw_hittime_qslice() {
    TFile *pInput = new TFile("./big_output.root");
    TTree *pTree = static_cast<TTree *>(pInput->Get("hits"));

    double time, prim_t;
    int trkCharge, stepCharge;

    pTree->SetBranchAddress("time", &time);
    pTree->SetBranchAddress("prim_t", &prim_t);
    pTree->SetBranchAddress("trkCharge", &trkCharge);
    pTree->SetBranchAddress("stepCharge", &stepCharge);

    array<double, 6> qTrkCrits, qStepCrits;
    array<TH1D *, 6> trkHists, stepHists;

    double q_per_fC = 6250;

    qTrkCrits[0] = 0.2;
    qTrkCrits[1] = 0.4;
    qTrkCrits[2] = 0.8;
    qTrkCrits[3] = 1;
    qTrkCrits[4] = 1.5;
    qTrkCrits[5] = 2;

    qStepCrits[0] = 1.5;
    qStepCrits[1] = 2;
    qStepCrits[2] = 2.5;
    qStepCrits[3] = 3;
    qStepCrits[4] = 3.5;
    qStepCrits[5] = 4;

    THStack *trkHStack, *stepHStack;

    trkHStack = new THStack("", "Hittime w/ qStop");
    stepHStack = new THStack("", "Hittime w/ qIon");

    for (int i = 0; i < 6; i++) {
        stringstream trkCritPostfix;
        trkCritPostfix << fixed << setprecision(1) << qTrkCrits[i];

        stringstream stepCritPostfix;
        stepCritPostfix << fixed << setprecision(1) << qStepCrits[i];

        trkHists[i] = new TH1D(("hTrk_" + to_string(int(qTrkCrits[i] * 10))).c_str(),
                               ("qCrit: " + trkCritPostfix.str()).c_str(), 60, 2000, 8000);
        stepHists[i] = new TH1D(("hStep" + to_string(int(qStepCrits[i] * 10))).c_str(),
                                ("qCrit: " + stepCritPostfix.str()).c_str(), 60, 2000, 8000);

        trkHists[i]->SetLineWidth(2);
        stepHists[i]->SetLineWidth(2);
        trkHStack->Add(trkHists[i]);
        stepHStack->Add(stepHists[i]);
    }

    Int_t entries = pTree->GetEntries();

    for (Int_t i = 0; i < entries; i++) {
        pTree->GetEntry(i);
        for (Int_t j = 0; j < 6; j++) {
            if (trkCharge > qTrkCrits[j] * q_per_fC)
                trkHists[j]->Fill(time);
            else
                break;
        }
        for (Int_t j = 0; j < 6; j++) {
            if (stepCharge > qStepCrits[j] * 5 * q_per_fC)
                stepHists[j]->Fill(time);
            else
                break;
        }
    }

    SetMatplotlibTab10Palette(trkHStack);
    SetMatplotlibTab10Palette(stepHStack);

    TCanvas *trkCanv = new TCanvas();
    trkHStack->Draw("plc nostack");
    TCanvas *stepCanv = new TCanvas();
    stepHStack->Draw("plc nostack");

    trkCanv->BuildLegend(0.570201, 0.502101, 0.869628, 0.869748);
    stepCanv->BuildLegend(0.570201, 0.502101, 0.869628, 0.869748);

    trkCanv->Update();
    stepCanv->Update();

    trkHStack->GetXaxis()->SetTitle("Hittime(ns)");
    stepHStack->GetXaxis()->SetTitle("Hittime(ns)");

    trkCanv->Modified();
    trkCanv->Update();
    stepCanv->Modified();
    stepCanv->Update();
}
