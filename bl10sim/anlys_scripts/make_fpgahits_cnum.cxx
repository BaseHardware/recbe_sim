#include <algorithm>
#include <vector>

#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Metadata.h"
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

constexpr double yield_factor  = 1;
constexpr double charge_per_eV = 1. / 3.6;

void print_onestep(const simobj::Step *s) {
    cout << setw(9) << s->GetX() << "  " << setw(9) << s->GetY() << "  " << setw(9) << s->GetZ()
         << "  " << setw(12) << s->GetKineticEnergy() << "  " << setw(13) << s->GetDepositedEnergy()
         << "  " << setw(20) << s->GetVolumeName() << "  " << setw(9) << s->GetCopyNumber()
         << setw(9) << s->GetEnvelopeCopyNumber() << " " << setw(13) << " " << s->GetProcessName()
         << "  " << s->GetNDaughters() << endl;
}

bool comp(const simobj::Step *lhs, const simobj::Step *rhs) {
    if (lhs->GetVolumeName() != rhs->GetVolumeName()) {
        return lhs->GetVolumeName() < rhs->GetVolumeName();
    } else if (lhs->GetEnvelopeCopyNumber() != rhs->GetEnvelopeCopyNumber()) {
        return lhs->GetEnvelopeCopyNumber() < rhs->GetEnvelopeCopyNumber();
    } else if (lhs->GetGlobalTime() != rhs->GetGlobalTime()) {
        return lhs->GetGlobalTime() < rhs->GetGlobalTime();
    } else if (lhs->GetDepositedEnergy() != rhs->GetDepositedEnergy()) {
        return lhs->GetDepositedEnergy() < rhs->GetDepositedEnergy();
    } else if (lhs->GetX() != rhs->GetX()) {
        return lhs->GetX() < rhs->GetX();
    } else if (lhs->GetY() != rhs->GetY()) {
        return lhs->GetY() < rhs->GetY();
    } else if (lhs->GetZ() != rhs->GetZ()) {
        return lhs->GetZ() < rhs->GetZ();
    } else if (lhs->GetPx() != rhs->GetPx()) {
        return lhs->GetPx() < rhs->GetPx();
    } else if (lhs->GetPy() != rhs->GetPy()) {
        return lhs->GetPy() < rhs->GetPy();
    } else if (lhs->GetPz() != rhs->GetPz()) {
        return lhs->GetPz() < rhs->GetPz();
    } else {
        return lhs->GetKineticEnergy() < rhs->GetKineticEnergy();
    }
}

bool comp_t(const simobj::Track *lhs, const simobj::Track *rhs) {
    const simobj::Step &lstep = lhs->GetFinalStep();
    const simobj::Step &rstep = rhs->GetFinalStep();
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
    int fTrkCharge, fStepCharge;

    HitInfo()
        : fEnvCopyNo(-1), fPVName(), fPrimaryKE(-1), fPrimaryTime(-1), fX(0), fY(0), fZ(0), fT(-1),
          fTrkCharge(0), fStepCharge(0) {};

    HitInfo(const simobj::Track *t, const simobj::Primary *p)
        : fEnvCopyNo(t->GetFinalStep().GetEnvelopeCopyNumber()),
          fPVName(t->GetFinalStep().GetVolumeName()),
          fPrimaryKE(p->GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(p->GetVertexObjPtr(0)->GetT()), fTrkCharge(1), fStepCharge(0) {
        fX = t->GetFinalStep().GetX();
        fY = t->GetFinalStep().GetY();
        fZ = t->GetFinalStep().GetZ();
        fT = t->GetFinalStep().GetGlobalTime();
    };

    HitInfo(const simobj::Step *s, const simobj::Primary *p)
        : fEnvCopyNo(s->GetEnvelopeCopyNumber()), fPVName(s->GetVolumeName()),
          fPrimaryKE(p->GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(p->GetVertexObjPtr(0)->GetT()), fTrkCharge(0) {
        fX = s->GetX();
        fY = s->GetY();
        fZ = s->GetZ();
        fT = s->GetGlobalTime();

        fStepCharge = s->GetIonDepositedEnergy() * 1e6 * charge_per_eV * yield_factor;
    };

    void UpdateHitPosition(int addedCharge, double x, double y, double z, double t) {
        double totalCharge = fTrkCharge + fStepCharge;
        double newCharge   = totalCharge + addedCharge;

        fX = (fX * totalCharge + addedCharge * x) / newCharge;
        fY = (fY * totalCharge + addedCharge * y) / newCharge;
        fZ = (fZ * totalCharge + addedCharge * z) / newCharge;
        fT = (fT * totalCharge + addedCharge * t) / newCharge;
    }

    bool IsAcceptable(const simobj::Step *target) const {
        double time_lower_bound = fT - time_window / 2.;
        double time_upper_bound = fT + time_window / 2.;
        double target_time      = target->GetGlobalTime();

        double x_diff = fX - target->GetX();
        double y_diff = fY - target->GetY();
        double z_diff = fZ - target->GetZ();

        double distance = sqrt(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);

        if (fPVName != target->GetVolumeName()) {
            return false;
        } else if (fEnvCopyNo != target->GetEnvelopeCopyNumber()) {
            return false;
        } else if (target_time < time_lower_bound || time_upper_bound < target_time) {
            return false;
        } else if (distance > hit_size) {
            return false;
        } else {
            return true;
        }
    }

    bool IsAcceptable(const simobj::Track *target) const {
        const simobj::Step &step = target->GetFinalStep();

        auto *particle = pdb->GetParticle(target->GetPDGCode());
        if (particle != nullptr && particle->Charge() == 0) {
            return false;
        } else {
            return IsAcceptable(&step);
        }
    }

    bool AppendTrack(const simobj::Track *track) {
        const simobj::Step &step = track->GetFinalStep();

        auto *particle = pdb->GetParticle(track->GetPDGCode());

        int trkCharge;
        if (particle == nullptr)
            trkCharge = 1;
        else
            trkCharge = abs(particle->Charge());

        if (IsAcceptable(track)) {
            UpdateHitPosition(trkCharge, step.GetX(), step.GetY(), step.GetZ(),
                              step.GetGlobalTime());

            fTrkCharge += trkCharge;
            return true;
        } else {
            return false;
        }
    }

    bool AppendStep(const simobj::Step *step) {
        int stepCharge = step->GetIonDepositedEnergy() * 1e6 * charge_per_eV * yield_factor;

        if (IsAcceptable(step)) {
            UpdateHitPosition(stepCharge, step->GetX(), step->GetY(), step->GetZ(),
                              step->GetGlobalTime());

            fStepCharge += stepCharge;
            return true;
        } else {
            return false;
        }
    }

    int GetTotalCharge() const { return fTrkCharge + fStepCharge; }
};

void make_fpgahits_cnum(const char *input_file  = "simout.root",
                        const char *output_file = "output.root") {

    TFile *pOutput = new TFile(output_file, "RECREATE");
    TTree *pOTree  = new TTree("hits", "tree for the FPGA hits");
    pOTree->SetDirectory(pOutput);

    Int_t evtid, env_copyno;
    Double_t time;
    Double_t prim_e, prim_t;
    string *pvname = nullptr;
    double x, y, z;
    int trkCharge, stepCharge;
    bool complete;

    int maxNTrack;

    const simobj::Track **tracks_in_fpga           = new const simobj::Track *[40000];
    pair<const simobj::Step *, int> *steps_in_fpga = new pair<const simobj::Step *, int>[20000];

    auto FillTreeWithHit = [&](vector<HitInfo> &buffer, int entry) -> void {
        bool max_track = false;
        evtid          = entry;
        if (buffer.size() == 1 && !complete) {
            max_track = true;
        }
        for (auto &nowhit : buffer) {
            env_copyno = nowhit.fEnvCopyNo;
            *pvname    = nowhit.fPVName;
            prim_e     = nowhit.fPrimaryKE;
            prim_t     = nowhit.fPrimaryTime;
            time       = nowhit.fT;
            x          = nowhit.fX;
            y          = nowhit.fY;
            z          = nowhit.fZ;
            if (trkCharge != 0 && max_track) {
                trkCharge = maxNTrack;
            } else {
                trkCharge = nowhit.fTrkCharge;
            }
            stepCharge = nowhit.fStepCharge;

            pOTree->Fill();
        }
    };

    size_t tracknum, stepnum;
    auto AddFPGATrack = [&](const simobj::Track *s) -> void {
        if (s->GetFinalStep().GetVolumeName().Contains("FPGADiePV") &&
            pdb->GetParticle(s->GetPDGCode()) != nullptr) {
            tracks_in_fpga[tracknum] = s;
            ++tracknum;
        }
    };
    auto AddFPGAStep = [&](const simobj::Step *s, int trk_idx) -> void {
        if (s->GetVolumeName().Contains("FPGADiePV") &&
            (s->GetProcessName() == "ionIoni" || s->GetProcessName() == "hIoni")) {
            steps_in_fpga[stepnum].first  = s;
            steps_in_fpga[stepnum].second = trk_idx;
            // print_onestep(s);
            ++stepnum;
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
    pOTree->Branch("trkCharge", &trkCharge);
    pOTree->Branch("stepCharge", &stepCharge);
    pOTree->Branch("complete", &complete);

    TFile *pInput = new TFile(input_file);

    TTree *pIP = static_cast<TTree *>(pInput->Get("persistent"));

    simobj::Metadata *metadata = nullptr;
    pIP->SetBranchAddress("Metadata", &metadata);
    pIP->GetEntry(0);

    TTree *pITree = static_cast<TTree *>(pInput->Get(metadata->GetOutputTreename().c_str()));

    maxNTrack = metadata->GetMaxTrackNum();

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    pITree->SetBranchAddress("Steps", &tcaStep);
    pITree->SetBranchAddress("Tracks", &tcaTrack);
    pITree->SetBranchAddress("Primary", &primary);
    pITree->SetBranchAddress("complete", &complete);

    int n_evts = pITree->GetEntries();

    vector<HitInfo> hiBuffer;

    for (int i_evt = 0; i_evt < n_evts; i_evt++) {
        pITree->GetEntry(i_evt);

        int n_trk = tcaTrack->GetEntries();

        stepnum = tracknum = 0;
        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));
            AddFPGATrack(now_track);

            const simobj::Step *f_step = &now_track->GetFirstStep();
            AddFPGAStep(f_step, idx_track);

            for (int idx_step = 0; idx_step < now_track->GetNStep(); idx_step++) {
                simobj::Step *now_step =
                    static_cast<simobj::Step *>(tcaStep->At(now_track->GetStepIndex(idx_step)));

                AddFPGAStep(now_step, idx_track);
            }

            f_step = &now_track->GetFinalStep();
            AddFPGAStep(f_step, idx_track);
        }

        cout << "EvtID: " << i_evt << " | Tracknum: " << tracknum << " | Stepnum: " << stepnum
             << endl;

        cout << "Sorting start" << endl;
        sort(tracks_in_fpga, tracks_in_fpga + tracknum, comp_t);
        cout << "Sorting end. Hitting start." << endl;

        for (size_t idx_track = 0; idx_track < tracknum; idx_track++) {
            auto trk = tracks_in_fpga[idx_track];

            bool appended = false;
            for (auto &i : hiBuffer) {
                if (i.AppendTrack(trk)) {
                    appended = true;
                    break;
                }
            }

            if (!appended) {
                hiBuffer.push_back(HitInfo(trk, primary));
            }
        }

        for (size_t idx_step = 0; idx_step < stepnum; idx_step++) {
            auto &step = steps_in_fpga[idx_step];

            bool appended = false;
            for (auto &i : hiBuffer) {
                if (i.AppendStep(step.first)) {
                    appended = true;
                    break;
                }
            }

            if (!appended) {
                HitInfo newHit = HitInfo(step.first, primary);

                if (newHit.GetTotalCharge() != 0) hiBuffer.push_back(std::move(newHit));
            }
        }
        cout << "Hitting end. [N = " << hiBuffer.size() << "] Filling start." << endl;

        if (hiBuffer.size() > 0) {
            FillTreeWithHit(hiBuffer, i_evt);
        }

        cout << "Filling end." << endl;
        hiBuffer.clear();
    }

    delete[] tracks_in_fpga;
    delete[] steps_in_fpga;
    tracks_in_fpga = nullptr;
    steps_in_fpga  = nullptr;

    pInput->Close();
    pOutput->Write();
    pOutput->Close();
}
