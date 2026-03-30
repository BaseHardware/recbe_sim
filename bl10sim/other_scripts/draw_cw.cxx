#include <iostream>

#include "TCanvas.h"
#include "TF1.h"
#include "TGaxis.h"
#include "TH1F.h"
#include "TLine.h"
#include "TPad.h"

#include "./cw.cxx"

using namespace std;

void draw_cw() {
    ColeWindsor *a = new ColeWindsor;

    double e = 1;
    a->UpdateParameters(e);

    auto testPDF = [a](double *x, double *p) -> double { return a->PDF(x[0], p[0]); };
    auto testCDF = [a](double *x, double *p) -> double { return a->CDF(x[0], p[0]); };

    TF1 *fPDF = new TF1("testf1", testPDF, 0, a->GetCurrentXMax(), 1);
    TF1 *fCDF = new TF1("testf1", testCDF, 0, a->GetCurrentXMax(), 1);

    TCanvas *c1 = new TCanvas();

    TPad *pad1 = new TPad("pad1", "", 0, 0, 1, 1);
    pad1->Draw();
    pad1->cd();

    fPDF->SetTitle("Cole-Windsor");
    fPDF->GetXaxis()->SetTitle("t");

    fPDF->SetParameter(0, e);
    fPDF->SetNpx(10000);
    fPDF->SetLineColor(kRed);

    fPDF->GetYaxis()->SetTitle("PDF (Red)");
    fPDF->GetYaxis()->SetTitleColor(kRed);
    fPDF->GetYaxis()->SetAxisColor(kRed);
    fPDF->GetYaxis()->SetLabelColor(kRed);

    fCDF->SetTitle("");
    fCDF->GetXaxis()->SetTitle("t");

    fCDF->SetParameter(0, e);
    fCDF->SetNpx(10000);
    fCDF->SetLineColor(kGreen + 1);

    fPDF->Draw();
    gPad->Update();

    c1->cd();
    TPad *pad2 = new TPad("pad2", "", 0, 0, 1, 1);
    pad2->SetFillStyle(4000); // Make the pad transparent
    pad2->SetFrameFillStyle(4000);
    pad2->Draw();
    pad2->cd();

    fCDF->Draw("");
    gPad->Update();
    fCDF->GetYaxis()->SetTickSize(0);
    fCDF->GetYaxis()->SetLabelSize(0);

    Double_t xmax = pad1->GetUxmax();
    Double_t ymin = pad2->GetUymin();
    Double_t ymax = pad2->GetUymax();

    TGaxis *axis =
        new TGaxis(xmax, ymin, xmax, ymax, fCDF->GetMinimum(), fCDF->GetMaximum(), 510, "+L");
    axis->SetLineColor(kGreen + 1);
    axis->SetLabelColor(kGreen + 1);
    axis->SetTitle("CDF (Green)");
    axis->Draw();

    fCDF->Draw("same");

    TLine *l1 = new TLine(a->Evaluate_t0(e), 0, a->Evaluate_t0(e), fCDF->GetMaximum());
    l1->SetLineColor(kBlack);
    l1->Draw("same");
    Double_t thres1 = a->GetCurrentT0() + a->GetCurrentThreshold1();
    TLine *l2       = new TLine(thres1, 0, thres1, fCDF->GetMaximum());
    l2->SetLineColor(kBlue);
    l2->Draw("same");

    cout << "t0: " << a->Evaluate_t0(e) << endl;
    cout << "s1: " << a->Evaluate_s1(e) << endl;
    cout << "s2: " << a->Evaluate_s2(e) << endl;
    cout << "g0: " << a->Evaluate_g1(e) << endl;
}
