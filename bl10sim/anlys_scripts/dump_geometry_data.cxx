#include <iostream>

#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Metadata.h"
#endif

using namespace std;

void dump_geometry_data() {
    TFile *pInput = new TFile("./simout.root");
    TTree *pTree  = static_cast<TTree *>(pInput->Get("persistent"));

    simobj::Metadata *md = nullptr;

    pTree->SetBranchAddress("Metadata", &md);
    pTree->GetEntry(0);

    std::vector<unsigned char> geomdata = std::move(md->GetGeometryData());

    cout << geomdata.data() << endl;
}
