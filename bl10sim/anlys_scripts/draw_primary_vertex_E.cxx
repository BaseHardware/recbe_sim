#include <iostream>
#include <vector>

#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#endif

void draw_primary_vertex_E(const char *infile_path = "./simout.root") {
    using namespace std;
    TFile *pInput = new TFile(infile_path);
    TTree *pTree  = static_cast<TTree *>(pInput->Get("tree"));

    simobj::Primary *primaries = nullptr;
    pTree->SetBranchAddress("Primary", &primaries);

    vector<double> binList = {
        1E-11,
        8.9124E-11, 1.1220E-10, 1.4125E-10, 1.7783E-10, 2.2387E-10, 2.8184E-10, 3.5481E-10,
        4.4668E-10, 5.6234E-10, 7.0795E-10, 8.9124E-10, 1.1220E-09, 1.4125E-09, 1.7783E-09,
        2.2387E-09, 2.8184E-09, 3.5481E-09, 4.4668E-09, 5.6234E-09, 7.0795E-09, 8.9124E-09,
        1.1220E-08, 1.4125E-08, 1.7783E-08, 2.2387E-08, 2.8184E-08, 3.5481E-08, 4.4668E-08,
        5.6234E-08, 7.0795E-08, 8.9124E-08, 1.1220E-07, 1.4125E-07, 1.7783E-07, 2.2387E-07,
        2.8184E-07, 3.5481E-07, 4.4668E-07, 5.6234E-07, 7.0795E-07, 8.9124E-07, 1.1220E-06,
        1.4125E-06, 1.7783E-06, 2.2387E-06, 2.8184E-06, 3.5481E-06, 4.4668E-06, 5.6234E-06,
        7.0795E-06, 8.9124E-06, 1.1220E-05, 1.4125E-05, 1.7783E-05, 2.2387E-05, 2.8184E-05,
        3.5481E-05, 4.4668E-05, 5.6234E-05, 7.0795E-05, 8.9124E-05, 1.1220E-04, 1.4125E-04,
        1.7783E-04, 2.2387E-04, 2.8184E-04, 3.5481E-04, 4.4668E-04, 5.6234E-04, 7.0795E-04,
        8.9124E-04, 1.1220E-03, 1.4125E-03, 1.7783E-03, 2.2387E-03, 2.8184E-03, 3.5481E-03,
        4.4668E-03, 5.6234E-03, 7.0795E-03, 8.9124E-03, 1.1220E-02, 1.4125E-02, 1.7783E-02,
        2.2387E-02, 2.8184E-02, 3.5481E-02, 4.4668E-02, 5.6234E-02, 7.0795E-02, 8.9124E-02,
        1.1220E-01, 1.4125E-01, 1.7783E-01, 2.2387E-01, 2.8184E-01, 3.5481E-01, 4.4668E-01,
        5.6234E-01, 7.0795E-01, 8.9124E-01, 1.0000E+00, 1.5849E+00, 2.5119E+00, 3.9811E+00,
        6.3096E+00, 1.0000E+01, 1.5849E+01, 2.5119E+01, 3.9811E+01, 6.3096E+01, 1.0000E+02,
        1.5849E+02, 2.5119E+02, 3.9811E+02, 1E4};

    double prev = 0;
    for (auto &i : binList) {
        if (prev > i) cout << "WRONG: " << i << endl;
        prev = i;
    }

    TH1F *hist = new TH1F("hist", "hist", binList.size() - 1, binList.data());

    Int_t entries = pTree->GetEntries();
    for (Int_t i = 0; i < entries; i++) {
        pTree->GetEntry(i);
        simobj::PrimaryParticle *part = primaries->GetPrimaryParticleObjPtr(0);
        hist->Fill(part->GetKineticEnergy());
    }

    hist->Draw("");
}
