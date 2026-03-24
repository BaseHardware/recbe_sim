#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

constexpr Double_t time_window = 12;

bool operator<(const simobj::Step &lhs, const simobj::Step &rhs) {
    if (lhs.GetVolumeName() != rhs.GetVolumeName()) {
        return lhs.GetVolumeName() < rhs.GetVolumeName();
    } else if (lhs.GetEnvelopeCopyNumber() != rhs.GetEnvelopeCopyNumber()) {
        return lhs.GetEnvelopeCopyNumber() < rhs.GetEnvelopeCopyNumber();
    } else if (lhs.GetGlobalTime() != rhs.GetGlobalTime()) {
        return lhs.GetGlobalTime() < rhs.GetGlobalTime();
    } else if (lhs.GetDepositedEnergy() != rhs.GetDepositedEnergy()) {
        return lhs.GetDepositedEnergy() < rhs.GetDepositedEnergy();
    } else if (lhs.GetX() != rhs.GetX()) {
        return lhs.GetX() < rhs.GetX();
    } else if (lhs.GetY() != rhs.GetY()) {
        return lhs.GetY() < rhs.GetY();
    } else if (lhs.GetZ() != rhs.GetZ()) {
        return lhs.GetZ() < rhs.GetZ();
    } else if (lhs.GetPx() != rhs.GetPx()) {
        return lhs.GetPx() < rhs.GetPx();
    } else if (lhs.GetPy() != rhs.GetPy()) {
        return lhs.GetPy() < rhs.GetPy();
    } else if (lhs.GetPz() != rhs.GetPz()) {
        return lhs.GetPz() < rhs.GetPz();
    } else {
        return lhs.GetKineticEnergy() < rhs.GetKineticEnergy();
    }
}

struct HitInfo {
    int fEnvCopyNo;
    double fPrimaryKE, fPrimaryTime;
    double fTotalEdep, fHitTime;
    string fPVName;

    vector<int> fStepPID, fStepPDG;
    vector<double> fStepEdep, fStepX, fStepY, fStepZ, fStepT;
    vector<string> fStepBPVName;

    HitInfo()
        : fEnvCopyNo(-1), fPrimaryKE(-1), fPrimaryTime(-1), fTotalEdep(0), fHitTime(-1),
          fPVName() {};

    HitInfo(const simobj::Track &t, const simobj::Step &s, const simobj::Primary &p,
            const string &bpvname)
        : fEnvCopyNo(s.GetEnvelopeCopyNumber()),
          fPrimaryKE(p.GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(p.GetVertexObjPtr(0)->GetT()), fTotalEdep(s.GetDepositedEnergy()),
          fHitTime(s.GetGlobalTime()), fPVName(s.GetVolumeName()) {
        fStepPID.push_back(t.GetTrackID());
        fStepPDG.push_back(t.GetPDGCode());
        fStepEdep.push_back(s.GetDepositedEnergy());
        fStepX.push_back(s.GetX());
        fStepY.push_back(s.GetY());
        fStepZ.push_back(s.GetZ());
        fStepT.push_back(s.GetGlobalTime());
        fStepBPVName.push_back(bpvname);
    };

    bool IsAcceptableStep(const simobj::Step &target) const {
        double time_lower_bound = fHitTime;
        double time_upper_bound = fHitTime + time_window;
        double target_time      = target.GetGlobalTime();
        if (fPVName == target.GetVolumeName() && fEnvCopyNo == target.GetEnvelopeCopyNumber() &&
            (time_lower_bound < target_time && target_time < time_upper_bound)) {
            return true;
        } else {
            return false;
        }
    }

    bool AppendStep(const simobj::Step &target, const int pid, const int pdg,
                    const string &bpvname) {
        if (IsAcceptableStep(target)) {
            fTotalEdep += target.GetDepositedEnergy();
            fStepPID.push_back(pid);
            fStepPDG.push_back(pdg);
            fStepEdep.push_back(target.GetDepositedEnergy());
            fStepX.push_back(target.GetX());
            fStepY.push_back(target.GetY());
            fStepZ.push_back(target.GetZ());
            fStepT.push_back(target.GetGlobalTime());
            fStepBPVName.push_back(bpvname);
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
    Double_t total_edep, time;
    Double_t prim_e, prim_t;
    string *pvname = nullptr;
    vector<int> pid, pdg;
    vector<double> edep;
    vector<double> in_x, in_y, in_z, in_t;
    vector<string> born_pvname;

    array<pair<simobj::Step, int>, 1000> step_in_fpga;

    auto FillTreeWithHit = [&](const HitInfo &hit, int entry) -> void {
        evtid       = entry;
        env_copyno  = hit.fEnvCopyNo;
        total_edep  = hit.fTotalEdep;
        time        = hit.fHitTime;
        prim_e      = hit.fPrimaryKE;
        prim_t      = hit.fPrimaryTime;
        *pvname     = hit.fPVName;
        pid         = std::move(hit.fStepPID);
        pdg         = std::move(hit.fStepPDG);
        edep        = std::move(hit.fStepEdep);
        in_x        = std::move(hit.fStepX);
        in_y        = std::move(hit.fStepY);
        in_z        = std::move(hit.fStepZ);
        in_t        = std::move(hit.fStepT);
        born_pvname = std::move(hit.fStepBPVName);

        pOTree->Fill();

        pid.clear();
        pdg.clear();
        edep.clear();
        in_x.clear();
        in_y.clear();
        in_z.clear();
        in_t.clear();
        born_pvname.clear();
    };

    pOTree->Branch("evtid", &evtid);
    pOTree->Branch("total_edep", &total_edep);
    pOTree->Branch("time", &time);
    pOTree->Branch("pvname", &pvname);
    pOTree->Branch("prim_e", &prim_e);
    pOTree->Branch("prim_t", &prim_t);
    pOTree->Branch("envelope_copyno", &env_copyno);
    pOTree->Branch("particle_pid", &pid);
    pOTree->Branch("particle_pdg", &pdg);
    pOTree->Branch("particle_edep", &edep);
    pOTree->Branch("particle_incident_x", &in_x);
    pOTree->Branch("particle_incident_y", &in_y);
    pOTree->Branch("particle_incident_z", &in_z);
    pOTree->Branch("particle_incident_t", &in_t);
    pOTree->Branch("particle_born_pvname", &born_pvname);

    TFile *pInput = new TFile(input_file);

    TTree *pITree = static_cast<TTree *>(pInput->Get("tree"));

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    pITree->SetBranchAddress("Steps", &tcaStep);
    pITree->SetBranchAddress("Tracks", &tcaTrack);
    pITree->SetBranchAddress("Primary", &primary);

    int n_evts = pITree->GetEntries();

    for (int i_evt = 0; i_evt < n_evts; i_evt++) {
        pITree->GetEntry(i_evt);

        int n_trk = tcaTrack->GetEntries();

        size_t stepnum = 0;
        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));

            for (int idx_step = 0; idx_step < now_track->GetNStep(); idx_step++) {
                simobj::Step *now_step =
                    static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));

                const auto &volName = now_step->GetVolumeName();

                if (volName.Contains("FPGADiePV")) {
                    step_in_fpga[stepnum].first  = *(now_step);
                    step_in_fpga[stepnum].second = idx_track;
                    ++stepnum;
                }
            }
        }

        cout << "EvtID: " << i_evt << " | Stepnum: " << stepnum << endl;

        sort(step_in_fpga.begin(), step_in_fpga.begin() + stepnum);

        HitInfo currentHit;
        for (size_t i_step = 0; i_step < stepnum; i_step++) {
            auto &step = step_in_fpga[i_step].first;
            auto track = static_cast<simobj::Track *>(tcaTrack->At(step_in_fpga[i_step].second));
            string bpvname = static_cast<simobj::Step *>(tcaStep->At(track->GetStepIndex(0)))
                                 ->GetVolumeName()
                                 .Data();

            if (i_step == 0) {
                currentHit = HitInfo(*track, step, *primary, bpvname);
            } else {
                if (!currentHit.AppendStep(step, track->GetTrackID(), track->GetPDGCode(),
                                           bpvname)) {
                    FillTreeWithHit(currentHit, i_evt);
                    currentHit = HitInfo(*track, step, *primary, bpvname);
                }
            }
        }
        if (stepnum > 0) {
            FillTreeWithHit(currentHit, i_evt);
        }
    }

    pInput->Close();
    pOutput->Write();
    pOutput->Close();
}
