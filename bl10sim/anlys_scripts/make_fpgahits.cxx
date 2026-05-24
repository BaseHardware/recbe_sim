#include <algorithm>
#include <vector>

#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

const TDatabasePDG *pdb = TDatabasePDG::Instance();

// time unit: ns
// length unit: mm

constexpr Double_t time_window = 12;
constexpr Double_t hit_size    = 0.1;

bool operator<(const simobj::Track &lhs, const simobj::Track &rhs) {
    const simobj::Step &lstep = lhs.GetFinalStep();
    const simobj::Step &rstep = rhs.GetFinalStep();
    if (lstep.GetVolumeName() != rstep.GetVolumeName()) {
        return lstep.GetVolumeName() < rstep.GetVolumeName();
    } else if (lstep.GetEnvelopeCopyNumber() != rstep.GetEnvelopeCopyNumber()) {
        return lstep.GetEnvelopeCopyNumber() < rstep.GetEnvelopeCopyNumber();
    } else if (lstep.GetGlobalTime() != rstep.GetGlobalTime()) {
        return lstep.GetGlobalTime() < rstep.GetGlobalTime();
    } else if (lstep.GetDepositedEnergy() != rstep.GetDepositedEnergy()) {
        return lstep.GetDepositedEnergy() < rstep.GetDepositedEnergy();
    } else if (lstep.GetX() != rstep.GetX()) {
        return lstep.GetX() < rstep.GetX();
    } else if (lstep.GetY() != rstep.GetY()) {
        return lstep.GetY() < rstep.GetY();
    } else if (lstep.GetZ() != rstep.GetZ()) {
        return lstep.GetZ() < rstep.GetZ();
    } else if (lstep.GetPx() != rstep.GetPx()) {
        return lstep.GetPx() < rstep.GetPx();
    } else if (lstep.GetPy() != rstep.GetPy()) {
        return lstep.GetPy() < rstep.GetPy();
    } else if (lstep.GetPz() != rstep.GetPz()) {
        return lstep.GetPz() < rstep.GetPz();
    } else {
        return lstep.GetKineticEnergy() < rstep.GetKineticEnergy();
    }
}

struct HitInfo {
    int fEnvCopyNo;
    string fPVName;
    double fPrimaryKE, fPrimaryTime;
    double fX, fY, fZ, fT;
    int fChargeNum;

    HitInfo()
        : fEnvCopyNo(-1), fPVName(), fPrimaryKE(-1), fPrimaryTime(-1), fX(0), fY(0), fZ(0), fT(-1),
          fChargeNum(0) {};

    HitInfo(const simobj::Track &t, const simobj::Primary &p)
        : fEnvCopyNo(t.GetFinalStep().GetEnvelopeCopyNumber()),
          fPVName(t.GetFinalStep().GetVolumeName()),
          fPrimaryKE(p.GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(p.GetVertexObjPtr(0)->GetT()), fT(t.GetFinalStep().GetGlobalTime()),
          fChargeNum(1) {};

    bool IsAcceptableTrack(const simobj::Track &target) const {
        const simobj::Step &step = target.GetFinalStep();

        double time_lower_bound = fT - time_window / 2.;
        double time_upper_bound = fT + time_window / 2.;
        double target_time      = step.GetGlobalTime();

        double x_diff = fX - step.GetX();
        double y_diff = fY - step.GetY();
        double z_diff = fZ - step.GetZ();

        double distance = sqrt(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);

        auto *particle = pdb->GetParticle(target.GetPDGCode());
        if (particle != nullptr && particle->Charge() == 0) {
            return false;
        } else if (fPVName != step.GetVolumeName()) {
            return false;
        } else if (fEnvCopyNo != step.GetEnvelopeCopyNumber()) {
            return false;
        } else if (target_time < time_lower_bound || time_upper_bound < target_time) {
            return false;
        } else if (distance > hit_size) {
            return false;
        } else {
            return true;
        }
    }

    bool AppendTrack(const simobj::Track &track) {
        const simobj::Step &step = track.GetFinalStep();

        auto *particle = pdb->GetParticle(track.GetPDGCode());

        int trkCharge;
        if (particle == nullptr)
            trkCharge = 1;
        else
            trkCharge = abs(particle->Charge());

        int newCharge = fChargeNum + trkCharge;

        if (IsAcceptableTrack(track)) {
            fX = (fX * fChargeNum + trkCharge * step.GetX()) / newCharge;
            fY = (fY * fChargeNum + trkCharge * step.GetY()) / newCharge;
            fZ = (fZ * fChargeNum + trkCharge * step.GetZ()) / newCharge;
            fT = (fT * fChargeNum + trkCharge * step.GetGlobalTime()) / newCharge;

            fChargeNum = newCharge;
            return true;
        } else {
            return false;
        }
    }
};

void make_fpgahits(const char *input_file  = "./simout.root",
                   const char *output_file = "output.root") {

    TFile *pOutput = new TFile(output_file, "RECREATE");
    TTree *pOTree  = new TTree("hits", "tree for the FPGA hits");
    pOTree->SetDirectory(pOutput);

    Int_t evtid, env_copyno;
    Double_t time;
    Double_t prim_e, prim_t;
    string *pvname = nullptr;
    double x, y, z;
    int chargeNum;

    simobj::Track *tracks_in_fpga = new simobj::Track[40000];

    auto FillTreeWithHit = [&](vector<HitInfo> &buffer, int entry) -> void {
        evtid = entry;
        for (auto &nowhit : buffer) {
            env_copyno = nowhit.fEnvCopyNo;
            *pvname    = nowhit.fPVName;
            prim_e     = nowhit.fPrimaryKE;
            prim_t     = nowhit.fPrimaryTime;
            time       = nowhit.fT;
            x          = nowhit.fX;
            y          = nowhit.fY;
            z          = nowhit.fZ;
            chargeNum  = nowhit.fChargeNum;
        }

        pOTree->Fill();
    };

    size_t tracknum;
    auto AddFPGATrack = [&](const simobj::Track *s) -> void {
        if (s->GetFinalStep().GetVolumeName().Contains("FPGADiePV")) {
            tracks_in_fpga[tracknum] = *s;
            ++tracknum;
        }
    };

    pOTree->Branch("evtid", &evtid);
    pOTree->Branch("envelope_copyno", &env_copyno);
    pOTree->Branch("pvname", &pvname);
    pOTree->Branch("prim_e", &prim_e);
    pOTree->Branch("prim_t", &prim_t);
    pOTree->Branch("x", &x);
    pOTree->Branch("y", &y);
    pOTree->Branch("z", &z);
    pOTree->Branch("time", &time);
    pOTree->Branch("chargeNum", &chargeNum);

    TFile *pInput = new TFile(input_file);

    TTree *pITree = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;
    bool complete;

    pITree->SetBranchAddress("Steps", &tcaStep);
    pITree->SetBranchAddress("Tracks", &tcaTrack);
    pITree->SetBranchAddress("Primary", &primary);
    pITree->SetBranchAddress("complete", &complete);

    int n_evts = pITree->GetEntries();

    vector<HitInfo> hiBuffer;

    for (int i_evt = 0; i_evt < n_evts; i_evt++) {
        pITree->GetEntry(i_evt);
        if (!complete) continue;

        int n_trk = tcaTrack->GetEntries();

        tracknum = 0;
        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));
            AddFPGATrack(now_track);
        }

        cout << "EvtID: " << i_evt << " | Tracknum: " << tracknum << endl;

        sort(tracks_in_fpga, tracks_in_fpga + tracknum);

        HitInfo currentHit;
        for (size_t idx_track = 0; idx_track < tracknum; idx_track++) {
            auto &trk = tracks_in_fpga[idx_track];

            bool appended = false;
            for (auto &i : hiBuffer) {
                if (i.AppendTrack(trk)) {
                    appended = true;
                    break;
                }
            }

            if (!appended) {
                hiBuffer.push_back(HitInfo(trk, *primary));
            }
        }

        if (hiBuffer.size() > 0) {
            FillTreeWithHit(hiBuffer, i_evt);
        }

        hiBuffer.clear();
    }

    delete[] tracks_in_fpga;

    pInput->Close();
    pOutput->Write();
    pOutput->Close();
}
