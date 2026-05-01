#include <fstream>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TH1F.h"

using namespace std;

void draw_flux() {
    ifstream input("./at_ductexit.txt");

    vector<double> binning;
    vector<double> flux;

    while (!input.eof()) {
        double r1, r2;

        input >> r1 >> r2;

        binning.push_back(r1);
        flux.push_back(r2);
    }

    binning.pop_back();

    TH1F *hist = new TH1F("hist", "Flux", binning.size() - 1, binning.data());

    for (size_t i = 0; i < binning.size(); i++) {
        hist->SetBinContent(i + 1, flux[i]);
    }

    TCanvas *c1 = new TCanvas();

    hist->SetLineColor(kBlue);
    hist->GetXaxis()->SetTitle("Neutron energy [MeV]");
    hist->GetYaxis()->SetTitle("Neutron flux [cm2/sec/MW]");

    hist->Draw("hists");

    c1->SetLogx();
    c1->SetLogy();
}
