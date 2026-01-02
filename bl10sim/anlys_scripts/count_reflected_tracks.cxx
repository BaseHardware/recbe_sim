#include <iostream>

#include "TClonesArray.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

void count_reflected_tracks(const char *infile_path = "./simout.root") {
    TFile *pInput = new TFile(infile_path);
    TTree *pTree  = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcaStep, *tcaTrack;
    tcaStep = tcaTrack = nullptr;

    pTree->SetBranchAddress("Step", &tcaStep);
    pTree->SetBranchAddress("Track", &tcaTrack);

    int error_tracks      = 0;
    int reflected_tracks  = 0;
    int borninduct_tracks = 0;
    int straight_tracks   = 0;
    for (int idx_event = 0; idx_event < pTree->GetEntries(); idx_event++) {
        pTree->GetEntry(idx_event);

        for (int idx_track = 0; idx_track < tcaTrack->GetEntries(); idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));
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

            track_status now_state = INIT;
            for (int idx_step = 0; idx_step < now_track->GetNStep(); idx_step++) {
                simobj::Step *now_step =
                    static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));

                const auto &volName = now_step->GetVolumeName();
                switch (now_state) {
                    case INIT:
                        if (volName == "DuctInnerPV")
                            now_state = INNER;
                        else if (volName == "DuctOuterPV")
                            now_state = BORN_IN_DUCT;
                        else if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_NOREFLECT;
                        else
                            now_state = OUTSIDE;
                        break;
                    case OUTSIDE:
                        if (volName == "DuctInnerPV" || volName == "FilterPV")
                            now_state = INNER;
                        else if (volName == "DuctOuterPV")
                            now_state = DUCT;
                        else if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_NOREFLECT;
                        else
                            now_state = OUTSIDE;
                        break;
                    case INNER:
                        if (volName == "DuctInnerPV" || volName == "FilterPV")
                            now_state = INNER;
                        else if (volName == "DuctOuterPV")
                            now_state = DUCT;
                        else if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_NOREFLECT;
                        else
                            now_state = OUTSIDE;
                        break;
                    case DUCT:
                        if (volName == "DuctOuterPV")
                            now_state = DUCT;
                        else if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_REFLECT;
                        else
                            now_state = REFLECTED;
                        break;
                    case BORN_IN_DUCT:
                        if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_BID;
                        else
                            now_state = BORN_IN_DUCT;
                        break;
                    case REFLECTED:
                        if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_REFLECT;
                        else
                            now_state = REFLECTED;
                        break;
                    case DETECTED_NOREFLECT:
                        if (volName == "DuctOuterPV")
                            now_state = DUCT_AFTER_DNR;
                        else
                            now_state = DETECTED_NOREFLECT;
                        break;
                    case DUCT_AFTER_DNR:
                        if (volName == "DuctOuterPV")
                            now_state = DUCT_AFTER_DNR;
                        else if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_REFLECT;
                        else
                            now_state = REFLECTED_AFTER_DNR;
                        break;
                    case REFLECTED_AFTER_DNR:
                        if (volName == "NeutronDetectorPV")
                            now_state = DETECTED_REFLECT;
                        else
                            now_state = REFLECTED_AFTER_DNR;
                        break;
                    case DETECTED_REFLECT:
                    case DETECTED_BID:
                        break;
                }
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
}
