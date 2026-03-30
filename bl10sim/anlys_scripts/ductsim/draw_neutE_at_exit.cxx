#include <iostream>
#include <vector>

#include "TClonesArray.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

void draw_neutE_at_exit(const char *infile_path = "./simout.root") {
    TFile *pInput = new TFile(infile_path);
    TTree *pTree  = static_cast<TTree *>(pInput->Get("tree"));

    simobj::Primary *primary = nullptr;
    TClonesArray    *tcaStep, *tcaTrack;
    tcaStep = tcaTrack = nullptr;

    pTree->SetBranchAddress("Steps", &tcaStep);
    pTree->SetBranchAddress("Tracks", &tcaTrack);
    pTree->SetBranchAddress("Primary", &primary);

    vector<double> binList = {
        1E-11,      8.9124E-11, 1.1220E-10, 1.4125E-10, 1.7783E-10, 2.2387E-10, 2.8184E-10,
        3.5481E-10, 4.4668E-10, 5.6234E-10, 7.0795E-10, 8.9124E-10, 1.1220E-09, 1.4125E-09,
        1.7783E-09, 2.2387E-09, 2.8184E-09, 3.5481E-09, 4.4668E-09, 5.6234E-09, 7.0795E-09,
        8.9124E-09, 1.1220E-08, 1.4125E-08, 1.7783E-08, 2.2387E-08, 2.8184E-08, 3.5481E-08,
        4.4668E-08, 5.6234E-08, 7.0795E-08, 8.9124E-08, 1.1220E-07, 1.4125E-07, 1.7783E-07,
        2.2387E-07, 2.8184E-07, 3.5481E-07, 4.4668E-07, 5.6234E-07, 7.0795E-07, 8.9124E-07,
        1.1220E-06, 1.4125E-06, 1.7783E-06, 2.2387E-06, 2.8184E-06, 3.5481E-06, 4.4668E-06,
        5.6234E-06, 7.0795E-06, 8.9124E-06, 1.1220E-05, 1.4125E-05, 1.7783E-05, 2.2387E-05,
        2.8184E-05, 3.5481E-05, 4.4668E-05, 5.6234E-05, 7.0795E-05, 8.9124E-05, 1.1220E-04,
        1.4125E-04, 1.7783E-04, 2.2387E-04, 2.8184E-04, 3.5481E-04, 4.4668E-04, 5.6234E-04,
        7.0795E-04, 8.9124E-04, 1.1220E-03, 1.4125E-03, 1.7783E-03, 2.2387E-03, 2.8184E-03,
        3.5481E-03, 4.4668E-03, 5.6234E-03, 7.0795E-03, 8.9124E-03, 1.1220E-02, 1.4125E-02,
        1.7783E-02, 2.2387E-02, 2.8184E-02, 3.5481E-02, 4.4668E-02, 5.6234E-02, 7.0795E-02,
        8.9124E-02, 1.1220E-01, 1.4125E-01, 1.7783E-01, 2.2387E-01, 2.8184E-01, 3.5481E-01,
        4.4668E-01, 5.6234E-01, 7.0795E-01, 8.9124E-01, 1.0000E+00, 1.5849E+00, 2.5119E+00,
        3.9811E+00, 6.3096E+00, 1.0000E+01, 1.5849E+01, 2.5119E+01, 3.9811E+01, 6.3096E+01,
        1.0000E+02, 1.5849E+02, 2.5119E+02, 3.9811E+02, 1E4};

    // TH1F *nEHist = new TH1F("nEHist", "Energy of neutrons at exit of the duct", binList.size() -
    // 1,
    //                         binList.data());
    TH1F *nEHist = new TH1F("nEHist", "Energy of neutrons at exit of the duct", 100, -0.01, 0.01);

    int error_tracks      = 0;
    int reflected_tracks  = 0;
    int borninduct_tracks = 0;
    int straight_tracks   = 0;
    for (int idx_event = 0; idx_event < pTree->GetEntries(); idx_event++) {
        pTree->GetEntry(idx_event);

        for (int idx_track = 0; idx_track < tcaTrack->GetEntries(); idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));
            if (now_track->GetPDGCode() != 2112) continue;
            enum track_status {
                INIT,
                OUTSIDE,
                INNER,
                DUCT,
                BORN_IN_DUCT,
                REFLECTED,
                DETECTED_REFLECT,
                DETECTED_BID,
                DETECTED_NOREFLECT,
                DUCT_AFTER_DNR,
                REFLECTED_AFTER_DNR,
            };

            double       nE_first_exit = -1;
            track_status now_state     = INIT;
            for (int idx_step = 0; idx_step < now_track->GetNStep(); idx_step++) {
                simobj::Step *now_step =
                    static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));

                const auto &volName = now_step->GetVolumeName();
                switch (now_state) {
                    case INIT:
                        if (volName == "DuctInnerPV") {
                            now_state = INNER;
                        } else if (volName == "DuctOuterPV") {
                            now_state = BORN_IN_DUCT;
                        } else if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_NOREFLECT;
                        } else {
                            now_state = OUTSIDE;
                        }
                        break;
                    case OUTSIDE:
                        if (volName == "DuctInnerPV" || volName == "FilterPV") {
                            now_state = INNER;
                        } else if (volName == "DuctOuterPV") {
                            now_state = DUCT;
                        } else if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_NOREFLECT;
                        } else {
                            now_state = OUTSIDE;
                        }
                        break;
                    case INNER:
                        if (volName == "DuctInnerPV" || volName == "FilterPV") {
                            now_state = INNER;
                        } else if (volName == "DuctOuterPV") {
                            now_state = DUCT;
                        } else if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_NOREFLECT;
                        } else {
                            now_state = OUTSIDE;
                        }
                        break;
                    case DUCT:
                        if (volName == "DuctOuterPV") {
                            now_state = DUCT;
                        } else if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_REFLECT;
                        } else {
                            now_state = REFLECTED;
                        }
                        break;
                    case BORN_IN_DUCT:
                        if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_BID;
                        } else {
                            now_state = BORN_IN_DUCT;
                        }
                        break;
                    case REFLECTED:
                        if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_REFLECT;
                        } else {
                            now_state = REFLECTED;
                        }
                        break;
                    case DETECTED_NOREFLECT:
                        if (volName == "DuctOuterPV")
                            now_state = DUCT_AFTER_DNR;
                        else
                            now_state = DETECTED_NOREFLECT;
                        break;
                    case DUCT_AFTER_DNR:
                        if (volName == "DuctOuterPV") {
                            now_state = DUCT_AFTER_DNR;
                        } else if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_REFLECT;
                        } else {
                            now_state = REFLECTED_AFTER_DNR;
                        }
                        break;
                    case REFLECTED_AFTER_DNR:
                        if (volName == "NeutronDetectorPV") {
                            if (nE_first_exit == -1) nE_first_exit = now_step->GetKineticEnergy();
                            now_state = DETECTED_REFLECT;
                        } else {
                            now_state = REFLECTED_AFTER_DNR;
                        }
                        break;
                    case DETECTED_REFLECT:
                    case DETECTED_BID:
                        break;
                }

                if (nE_first_exit != -1)
                    nEHist->Fill(nE_first_exit -
                                 primary->GetPrimaryParticleObjPtr(0)->GetKineticEnergy());
            }

            switch (now_state) {
                case INIT:
                    error_tracks++;
                    break;
                case OUTSIDE:
                case INNER:
                case DUCT:
                case BORN_IN_DUCT:
                case REFLECTED:
                    break;
                case DETECTED_NOREFLECT:
                case DUCT_AFTER_DNR:
                case REFLECTED_AFTER_DNR:
                    straight_tracks++;
                    break;
                case DETECTED_BID:
                    borninduct_tracks++;
                    break;
                case DETECTED_REFLECT:
                    reflected_tracks++;
                    break;
            }
        }
    }

    cout << "Detected tracks: " << borninduct_tracks + reflected_tracks + straight_tracks << endl;
    cout << "Straight tracks: " << straight_tracks << endl;
    cout << "Reflected tracks: " << reflected_tracks << endl;
    cout << "Born-in-duct tracks: " << borninduct_tracks << endl;
    cout << "Error tracks: " << error_tracks << endl;

    nEHist->Draw();
}
