#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "THStack.h"
#include "TTree.h"

using namespace std;

const char *titles[] = {"50 cm", "100 cm", "150 cm", "200 cm", "250 cm"};
Color_t histColors[] = {kBlack, kRed, kBlue, kGreen + 1, kViolet + 1};

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

void draw_hists(const std::vector<TH1D *> &arrays, const char *xtitle, bool xlog, bool ylog) {
    TCanvas *canv = new TCanvas();

    canv->SetLogx(xlog);
    canv->SetLogy(ylog);

    THStack *hs = new THStack();
    for (size_t i = 0; i < arrays.size(); i++) {
        arrays[i]->GetXaxis()->SetTitle(xtitle);
        arrays[i]->SetTitle(titles[i]);
        arrays[i]->SetLineWidth(2);
        arrays[i]->SetLineColor(histColors[i]);
        hs->Add(arrays[i]);
    }

    hs->Draw("nostack");
    hs->GetXaxis()->SetTitle(xtitle);
    canv->BuildLegend();
}

void draw_hists(const std::vector<TGraph *> &arrays) {
    TCanvas *canv = new TCanvas("", "", 700, 700);

    double region_size = 200;

    for (int i = arrays.size() - 1; i >= 0; i--) {
        arrays[i]->SetTitle(titles[i]);
        arrays[i]->SetLineWidth(2);
        arrays[i]->SetLineColor(histColors[i]);
        arrays[i]->SetMarkerColor(histColors[i]);

        arrays[i]->GetXaxis()->SetTitle("X Position [cm]");
        arrays[i]->GetYaxis()->SetTitle("Y Position [cm]");
        if (i == arrays.size() - 1) {
            arrays[i]->Draw("ap");
            double xmean = arrays[i]->GetMean(1);
            double ymean = arrays[i]->GetMean(2);
            arrays[i]->GetXaxis()->SetRangeUser(xmean - region_size, xmean + region_size);
            arrays[i]->GetYaxis()->SetRangeUser(ymean - region_size, ymean + region_size);
        } else {
            arrays[i]->Draw("p same");
        }
    }

    canv->BuildLegend();
    arrays.back()->SetTitle("x vs. y");
}

void draw(const char *input_file = "output_11mm.root") {
    constexpr size_t nbins = 100;

    auto ebins = build_logbins<nbins>(1e-11, 1e3);
    auto tbins = build_logbins<nbins>(1e4, 1e11);

    TFile *pInput = new TFile(input_file);
    TTree *pTree  = static_cast<TTree *>(pInput->Get("incident"));

    int pdg, copynum;
    double x, y, z, e, t;

    pTree->SetBranchStatus("*", false);

    pTree->SetBranchStatus("pdg", true);
    pTree->SetBranchStatus("copynum", true);
    pTree->SetBranchStatus("x", true);
    pTree->SetBranchStatus("y", true);
    pTree->SetBranchStatus("z", true);
    pTree->SetBranchStatus("e", true);
    pTree->SetBranchStatus("t", true);

    pTree->SetBranchAddress("pdg", &pdg);
    pTree->SetBranchAddress("copynum", &copynum);
    pTree->SetBranchAddress("x", &x);
    pTree->SetBranchAddress("y", &y);
    pTree->SetBranchAddress("z", &z);
    pTree->SetBranchAddress("e", &e);
    pTree->SetBranchAddress("t", &t);

    int entries = pTree->GetEntries();

    TH1::AddDirectory(false);

    vector<TH1D *> energy_hist_arr, time_hist_arr;
    // vector<TH2D *> xy_hist_arr;
    vector<TGraph *> xy_hist_arr;

    for (int i = 0; i < 5; i++) {
        energy_hist_arr.push_back(
            new TH1D("energy_hist", "energy_hist", ebins.size() - 1, ebins.data()));
        time_hist_arr.push_back(new TH1D("time_hist", "time_hist", tbins.size() - 1, tbins.data()));
        xy_hist_arr.push_back(new TGraph());
        // xy_hist_arr.push_back(new TH2D("xy_hist", "xy_hist", 100, 0, 300, 100, 100, 400));
    }

    for (int i = 0; i < entries; i++) {
        pTree->GetEntry(i);

        if (pdg != 2112) continue;
        energy_hist_arr[copynum]->Fill(e);
        time_hist_arr[copynum]->Fill(t);
        xy_hist_arr[copynum]->AddPoint(x, y);
    }

    double zero_entries = energy_hist_arr[0]->GetEntries();
    for (int i = 0; i < 5; i++) {
        cout << titles[i] << ": " << energy_hist_arr[i]->GetEntries() << " ("
             << 100 * energy_hist_arr[i]->GetEntries() / zero_entries << "%)" << endl;
    }

    draw_hists(energy_hist_arr, "Kinematic energy [MeV]", true, true);
    // draw_hists(time_hist_arr, "Passing time [ns]", true, false);
    // draw_hists(xy_hist_arr);
}
