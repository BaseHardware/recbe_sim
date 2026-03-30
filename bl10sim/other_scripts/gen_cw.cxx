#include "TF1.h"
#include "TH1.h"
#include "TStyle.h"

#include "./cw.cxx"

using namespace std;

void gen_cw() {
    ColeWindsor *a = new ColeWindsor;

    auto testFunc = [a](double *x, double *p) -> double { return p[0] * a->PDF(x[0], p[1]); };

    double e;

    e = 1;

    ColeWindsorSampler b(
        {.eMin_eV = 0.001, .eMax_eV = 10e4, .nE = 200, .nU = 4096, .allowExtrapolation = true});
    cout << "LUT generation finished." << endl;
    TH1F *h1 = new TH1F("h1", "Cole-Windsor Generation with e=1 eV", 1000, 0, 10);
    for (int i = 0; i < 1000000; i++) {
        double t = b.Sample(e);
        h1->Fill(t);
    }

    h1->GetXaxis()->SetTitle("Emission time(us)");
    h1->GetYaxis()->SetTitle("Number of entries / 10 ns");

    TF1 *f3 = new TF1("testf2", testFunc, 0, a->GetCurrentXMax(), 2);
    f3->SetParameter(0, 2.2 * 100000);
    f3->SetParameter(1, e);

    f3->SetParName(0, "Constants");
    f3->SetParName(1, "E_{n}");

    gStyle->SetOptFit(11111);
    h1->Draw("");
    h1->Fit(f3, "L");
    f3->Draw("same");
}
