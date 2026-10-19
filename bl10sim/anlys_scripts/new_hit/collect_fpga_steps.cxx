#include <iostream>
#include <vector>

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Metadata.h"
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

struct StepInfo {
    int fEnvCopyNo;
    string fPVName;
    double fStartX, fEndX;
    double fStartY, fEndY;
    double fStartZ, fEndZ;
    double fStartT, fEndT;
    double fStartE, fEndE;
    double fEdep, fNIEdep;

    int fPDG, fMakerPDG, fID, fMakerID;
    string fProc, fMakerProc;

    StepInfo() {}

    StepInfo(const simobj::Step *start, const simobj::Step *end, const simobj::Track *self,
             const simobj::Track *maker) {
        fEnvCopyNo = start->GetEnvelopeCopyNumber();
        fPVName    = start->GetVolumeName();

        fStartX = start->GetX();
        fStartY = start->GetY();
        fStartZ = start->GetZ();
        fStartT = start->GetGlobalTime();
        fStartE = start->GetKineticEnergy();

        fEndX = end->GetX();
        fEndY = end->GetY();
        fEndZ = end->GetZ();
        fEndT = end->GetGlobalTime();
        fEndE = end->GetKineticEnergy();

        fEdep   = end->GetDepositedEnergy();
        fNIEdep = end->GetNonIonDepositedEnergy();

        fPDG = self->GetPDGCode();

        fID = self->GetTrackID();

        fProc = self->GetFirstStep().GetProcessName();

        if (maker != nullptr) {
            fMakerPDG  = maker->GetPDGCode();
            fMakerID   = maker->GetTrackID();
            fMakerProc = maker->GetFirstStep().GetProcessName();
        } else {
            fMakerPDG  = 0;
            fMakerID   = 0;
            fMakerProc = "";
        }
    }

    double GetX() const { return (fEndX + fStartX) / 2.; }
    double GetY() const { return (fEndY + fStartY) / 2.; }
    double GetZ() const { return (fEndZ + fStartZ) / 2.; }
    double GetT() const { return (fEndT + fStartT) / 2.; }

    double GetLength() const {
        double lengthX = fEndX - fStartX;
        double lengthY = fEndY - fStartY;
        double lengthZ = fEndZ - fStartZ;

        return sqrt(lengthX * lengthX + lengthY * lengthY + lengthZ * lengthZ);
    }
};

bool comp_s(const StepInfo &lhs, const StepInfo &rhs) {
    if (lhs.fPVName != rhs.fPVName) {
        return lhs.fPVName < rhs.fPVName;
    } else if (lhs.fEnvCopyNo != rhs.fEnvCopyNo) {
        return lhs.fEnvCopyNo < rhs.fEnvCopyNo;
    } else if (lhs.fStartT != rhs.fStartT) {
        return lhs.fStartT < rhs.fStartT;
    } else if (lhs.fPDG != rhs.fPDG) {
        return lhs.fPDG < rhs.fPDG;
    } else if (lhs.fID != rhs.fID) {
        return lhs.fID < rhs.fID;
    } else if (lhs.fMakerPDG != rhs.fMakerPDG) {
        return lhs.fMakerPDG < rhs.fMakerPDG;
    } else if (lhs.fMakerID != rhs.fMakerID) {
        return lhs.fMakerID < rhs.fMakerID;
    } else if (lhs.fProc != rhs.fProc) {
        return lhs.fProc < rhs.fProc;
    } else if (lhs.fMakerProc != rhs.fMakerProc) {
        return lhs.fMakerProc < rhs.fMakerProc;
    } else if (lhs.fEdep != rhs.fEdep) {
        return lhs.fEdep < rhs.fEdep;
    } else if (lhs.fNIEdep != rhs.fNIEdep) {
        return lhs.fNIEdep < rhs.fNIEdep;
    } else if (lhs.GetX() != rhs.GetX()) {
        return lhs.GetX() < rhs.GetX();
    } else if (lhs.GetY() != rhs.GetY()) {
        return lhs.GetY() < rhs.GetY();
    } else {
        return lhs.GetZ() < rhs.GetZ();
    }
}

void collect_fpga_steps(const char *input_file  = "simout.root",
                        const char *output_file = "output.root") {
    TFile *pOutput = TFile::Open(output_file, "RECREATE");

    if (!pOutput || pOutput->IsZombie()) {
        cerr << "[ERROR] Cannot create output file: " << output_file << endl;
        delete pOutput;
        return;
    }

    TTree *pOTree = new TTree("steps", "tree for the FPGA steps");

    pOTree->SetDirectory(pOutput);

    Int_t evtid, env_copyno;
    Double_t prim_e, prim_t;
    string *pvname = nullptr, *filename = nullptr;
    double s_x, s_y, s_z, s_t, s_e, e_x, e_y, e_z, e_t, e_e;
    double edep, niedep, length;
    int pdg, maker_pdg, id, maker_id;
    string *proc = nullptr, *maker_proc = nullptr;

    pOTree->Branch("evtid", &evtid);
    pOTree->Branch("filename", &filename);
    pOTree->Branch("envelope_copyno", &env_copyno);
    pOTree->Branch("pvname", &pvname);
    pOTree->Branch("prim_e", &prim_e);
    pOTree->Branch("prim_t", &prim_t);
    pOTree->Branch("start_x", &s_x);
    pOTree->Branch("start_y", &s_y);
    pOTree->Branch("start_z", &s_z);
    pOTree->Branch("start_t", &s_t);
    pOTree->Branch("start_e", &s_e);
    pOTree->Branch("end_x", &e_x);
    pOTree->Branch("end_y", &e_y);
    pOTree->Branch("end_z", &e_z);
    pOTree->Branch("end_t", &e_t);
    pOTree->Branch("end_e", &e_e);

    pOTree->Branch("edep", &edep);
    pOTree->Branch("niedep", &niedep);
    pOTree->Branch("length", &length);

    pOTree->Branch("pdg", &pdg);
    pOTree->Branch("maker_pdg", &maker_pdg);
    pOTree->Branch("id", &id);
    pOTree->Branch("maker_id", &maker_id);
    pOTree->Branch("proc", &proc);
    pOTree->Branch("maker_proc", &maker_proc);

    *filename = input_file;

    TFile *pInput = TFile::Open(input_file, "READ");

    if (!pInput || pInput->IsZombie()) {
        cerr << "[ERROR] Cannot open input file: " << input_file << endl;
        delete pInput;
        pOutput->Close();
        delete pOutput;
        return;
    }

    *filename = input_file;

    TTree *pIP = nullptr;
    pInput->GetObject("persistent", pIP);

    if (!pIP || pIP->GetEntries() == 0) {
        cerr << "[ERROR] Invalid or empty persistent tree: " << input_file << endl;
        pInput->Close();
        delete pInput;
        pOutput->Close();
        delete pOutput;
        return;
    }

    simobj::Metadata *metadata = nullptr;

    if (pIP->SetBranchAddress("Metadata", &metadata) < 0 || pIP->GetEntry(0) <= 0 || !metadata) {
        cerr << "[ERROR] Cannot read Metadata: " << input_file << endl;
        pInput->Close();
        delete pInput;
        pOutput->Close();
        delete pOutput;
        return;
    }

    TTree *pITree = nullptr;
    pInput->GetObject(metadata->GetOutputTreename().c_str(), pITree);

    if (!pITree) {
        cerr << "[ERROR] Cannot find simulation tree: " << metadata->GetOutputTreename() << endl;
        pInput->Close();
        delete pInput;
        pOutput->Close();
        delete pOutput;
        return;
    }

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    if (pITree->SetBranchAddress("Steps", &tcaStep) < 0 ||
        pITree->SetBranchAddress("Tracks", &tcaTrack) < 0 ||
        pITree->SetBranchAddress("Primary", &primary) < 0) {
        cerr << "[ERROR] Cannot set simulation branch addresses: " << input_file << endl;

        pInput->Close();
        delete pInput;
        pOutput->Close();
        delete pOutput;
        return;
    }

    int n_evts = pITree->GetEntries();

    vector<int> id2idxTbl;
    id2idxTbl.resize(metadata->GetMaxTrackNum() * 10, 0);

    vector<StepInfo> steps_in_fpga;
    steps_in_fpga.reserve(10000);

    auto GetParentOutsideFPGA = [&](const simobj::Track *s) -> simobj::Track * {
        int parent_tid = s->GetTrackID();
        if (parent_tid == 0) {
            return nullptr;
        } else if (parent_tid == 1) {
            return static_cast<simobj::Track *>(tcaTrack->At(id2idxTbl[parent_tid]));

        } else {
            simobj::Track *parent = nullptr;
            while (parent_tid != 0) {
                parent = static_cast<simobj::Track *>(tcaTrack->At(id2idxTbl[parent_tid]));
                if (!parent->FirstStep().GetVolumeName().Contains("FPGADiePV")) break;
                parent_tid = parent->GetParentID();
            }
            return parent;
        }
    };

    auto FillTreeWithHit = [&](int entry) -> void {
        evtid = entry;

        for (auto &nowstep : steps_in_fpga) {

            env_copyno = nowstep.fEnvCopyNo;
            *pvname    = nowstep.fPVName;

            s_x = nowstep.fStartX;
            s_y = nowstep.fStartY;
            s_z = nowstep.fStartZ;
            s_t = nowstep.fStartT;
            s_e = nowstep.fStartE;

            e_x = nowstep.fEndX;
            e_y = nowstep.fEndY;
            e_z = nowstep.fEndZ;
            e_t = nowstep.fEndT;
            e_e = nowstep.fEndE;

            pdg       = nowstep.fPDG;
            maker_pdg = nowstep.fMakerPDG;

            edep   = nowstep.fEdep;
            niedep = nowstep.fNIEdep;
            length = nowstep.GetLength();

            id       = nowstep.fID;
            maker_id = nowstep.fMakerID;

            *proc       = nowstep.fProc;
            *maker_proc = nowstep.fMakerProc;

            pOTree->Fill();
        }
    };

    for (int i_evt = 0; i_evt < n_evts; i_evt++) {
        pITree->GetEntry(i_evt);

        int n_trk = tcaTrack->GetEntries();

        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));
            id2idxTbl[now_track->GetTrackID()] = idx_track;

            simobj::Track *maker_track;
            const simobj::Step *before_step, *now_step;
            for (int idx_step = 0; idx_step < now_track->GetNStep() + 1; idx_step++) {

                if (idx_step >= now_track->GetNStep()) {
                    now_step = &now_track->GetFinalStep();
                } else {
                    now_step =
                        static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));
                }

                if (idx_step == 0) {
                    before_step = &now_track->GetFirstStep();
                } else {
                    before_step = static_cast<simobj::Step *>(
                        tcaStep->At(now_track->GetStepIndex(idx_step - 1)));
                }

                if (!before_step->GetVolumeName().Contains("FPGADiePV") ||
                    now_step->GetDepositedEnergy() == 0)
                    continue;

                maker_track = GetParentOutsideFPGA(now_track);

                steps_in_fpga.push_back(StepInfo(before_step, now_step, now_track, maker_track));
            }
        }

        cout << "EvtID: " << i_evt << " | Stepnum: " << steps_in_fpga.size() << endl;

        cout << "Sorting start" << endl;
        stable_sort(steps_in_fpga.begin(), steps_in_fpga.end(), comp_s);
        cout << "Sorting end. Filling start." << endl;

        if (steps_in_fpga.size() > 0) {
            prim_e = primary->GetPrimaryParticleObjPtr(0)->GetKineticEnergy();
            prim_t = primary->GetVertexObjPtr(0)->GetT();
            FillTreeWithHit(i_evt);
        }

        steps_in_fpga.clear();

        cout << "Filling end." << endl;
    }

    cout << pOTree->GetEntries() << endl;

    pOutput->Write();

    pInput->Close();
    pOutput->Close();

    delete pInput;
    delete pOutput;
}
