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

bool comp_s(const simobj::Step *lhs, const simobj::Step *rhs) {
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
    double fM2;
    int fCharge;
    vector<int> fMakerID, fMakerPDG;
    vector<string> fMakerProc;

    HitInfo()
        : fEnvCopyNo(-1), fPVName(), fPrimaryKE(-1), fPrimaryTime(-1), fX(0), fY(0), fZ(0), fT(-1),
          fM2(0), fCharge(0) {};

    HitInfo(const simobj::Track *t, const simobj::Track *par, const simobj::Primary *prim)
        : fEnvCopyNo(t->GetFinalStep().GetEnvelopeCopyNumber()),
          fPVName(t->GetFinalStep().GetVolumeName()),
          fPrimaryKE(prim->GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(prim->GetVertexObjPtr(0)->GetT()), fM2(0), fCharge(1) {
        fX = t->GetFinalStep().GetX();
        fY = t->GetFinalStep().GetY();
        fZ = t->GetFinalStep().GetZ();
        fT = t->GetFinalStep().GetGlobalTime();

        fMakerID.push_back(par->GetTrackID());
        fMakerPDG.push_back(par->GetPDGCode());
        fMakerProc.push_back(par->GetFirstStep().GetProcessName().Data());
    };

    HitInfo(const simobj::Step *s, const simobj::Track *par, const simobj::Primary *prim)
        : fEnvCopyNo(s->GetEnvelopeCopyNumber()), fPVName(s->GetVolumeName()),
          fPrimaryKE(prim->GetPrimaryParticleObjPtr(0)->GetKineticEnergy()),
          fPrimaryTime(prim->GetVertexObjPtr(0)->GetT()), fCharge(0) {
        fX  = s->GetX();
        fY  = s->GetY();
        fZ  = s->GetZ();
        fT  = s->GetGlobalTime();
        fM2 = 0;

        fMakerID.push_back(par->GetTrackID());
        fMakerPDG.push_back(par->GetPDGCode());
        fMakerProc.push_back(par->GetFirstStep().GetProcessName().Data());

        fCharge = s->GetIonDepositedEnergy() * 1e6 * charge_per_eV * yield_factor;
    };

    void UpdateHitPosition(int addedCharge, double x, double y, double z, double t) {
        double newCharge = fCharge + addedCharge;

        double oldX = fX, oldY = fY, oldZ = fZ;

        fX = (fX * fCharge + addedCharge * x) / newCharge;
        fY = (fY * fCharge + addedCharge * y) / newCharge;
        fZ = (fZ * fCharge + addedCharge * z) / newCharge;
        fT = (fT * fCharge + addedCharge * t) / newCharge;

        double odx = x - oldX;
        double ody = y - oldY;
        double odz = z - oldZ;
        double dx  = x - fX;
        double dy  = y - fY;
        double dz  = z - fZ;

        fM2 += addedCharge * (odx * dx + ody * dy + odz * dz);
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

    bool AppendTrack(const simobj::Track *track, const simobj::Track *parent) {
        const simobj::Step &step = track->GetFinalStep();

        auto *particle = pdb->GetParticle(track->GetPDGCode());

        int trkCharge;
        if (particle == nullptr)
            trkCharge = 1;
        else
            trkCharge = abs(particle->Charge()) / 3;

        if (IsAcceptable(track)) {
            UpdateHitPosition(trkCharge, step.GetX(), step.GetY(), step.GetZ(),
                              step.GetGlobalTime());

            fCharge += trkCharge;

            auto findres = find(fMakerID.begin(), fMakerID.end(), parent->GetTrackID());
            if (fMakerID.size() == 0 || fMakerID.end() == findres) {
                fMakerID.push_back(parent->GetTrackID());
                fMakerPDG.push_back(parent->GetPDGCode());
                fMakerProc.push_back(parent->GetFirstStep().GetProcessName().Data());
            }
            return true;
        } else {
            return false;
        }
    }

    bool AppendStep(const simobj::Step *step, const simobj::Track *parent) {
        int stepCharge = step->GetIonDepositedEnergy() * 1e6 * charge_per_eV * yield_factor;

        if (IsAcceptable(step)) {
            UpdateHitPosition(stepCharge, step->GetX(), step->GetY(), step->GetZ(),
                              step->GetGlobalTime());

            fCharge += stepCharge;

            auto findres = find(fMakerID.begin(), fMakerID.end(), parent->GetTrackID());
            if (fMakerID.size() == 0 || fMakerID.end() == findres) {
                fMakerID.push_back(parent->GetTrackID());
                fMakerPDG.push_back(parent->GetPDGCode());
                fMakerProc.push_back(parent->GetFirstStep().GetProcessName().Data());
            }
            return true;
        } else {
            return false;
        }
    }

    int GetTotalCharge() const { return fCharge + fChargeDensity; }
};

void make_fpgahits_edep(const char *input_file  = "simout.root",
                        const char *output_file = "output.root") {

    TFile *pOutput = new TFile(output_file, "RECREATE");
    TTree *pOTree  = new TTree("hits", "tree for the FPGA hits");
    pOTree->SetDirectory(pOutput);

    Int_t evtid, env_copyno;
    Double_t time, total_edep;
    Double_t prim_e, prim_t;
    string *pvname = nullptr, *filename = nullptr;
    double x, y, z, r_rms;
    int charge;
    bool complete;

    vector<int> makerPDG;
    vector<string> makerProc;

    int maxNTrack;

    vector<int> id2idxTbl;

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
            total_edep = nowhit.fTotalEdep;
            prim_e     = nowhit.fPrimaryKE;
            prim_t     = nowhit.fPrimaryTime;
            time       = nowhit.fT;
            x          = nowhit.fX;
            y          = nowhit.fY;
            z          = nowhit.fZ;
            r_rms      = sqrt(nowhit.fM2 / nowhit.fCharge);
            charge     = nowhit.fCharge;
            // 각 Step의 Edep, Step Length, 각 Step의 중점의 x,y,z,t 를 추가해서 넣을 것

            makerPDG.clear();
            makerProc.clear();

            for (int i = 0; i < nowhit.fMakerID.size(); i++) {
                makerPDG.push_back(nowhit.fMakerPDG[i]);
                makerProc.push_back(nowhit.fMakerProc[i]);
            }

            pOTree->Fill();
        }
    };

    size_t stepnum;
    auto AddFPGAStep = [&](const simobj::Step *s, int idx) -> void {
        if (s->GetVolumeName().Contains("FPGADiePV")) {
            step_in_fpga[stepnum].first  = (*s);
            step_in_fpga[stepnum].second = idx;
            ++stepnum;
        }
    };

    pOTree->Branch("evtid", &evtid);
    pOTree->Branch("filename", &filename);
    pOTree->Branch("envelope_copyno", &env_copyno);
    pOTree->Branch("pvname", &pvname);
    pOTree->Branch("total_edep", &total_edep);
    pOTree->Branch("prim_e", &prim_e);
    pOTree->Branch("prim_t", &prim_t);
    pOTree->Branch("x", &x);
    pOTree->Branch("y", &y);
    pOTree->Branch("z", &z);
    pOTree->Branch("time", &time);
    pOTree->Branch("r_rms", &r_rms);
    pOTree->Branch("charge", &charge);
    pOTree->Branch("makerPDGs", &makerPDG);
    pOTree->Branch("makerProcs", &makerProc);
    pOTree->Branch("complete", &complete);

    TFile *pInput = new TFile(input_file);

    *filename = input_file;

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
    id2idxTbl.resize(metadata->GetMaxTrackNum(), 0);

    size_t tracknum, stepnum;

    auto GetParentOutsideFPGA = [&](const simobj::Track *s) -> simobj::Track * {
        int parent_tid        = s->GetTrackID();
        simobj::Track *parent = nullptr;
        while (parent_tid != 0) {
            parent = static_cast<simobj::Track *>(tcaTrack->At(id2idxTbl[parent_tid]));
            if (!parent->FirstStep().GetVolumeName().Contains("FPGADiePV")) break;
            parent_tid = parent->GetParentID();
        }
        return parent;
    };

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

    for (int i_evt = 0; i_evt < n_evts; i_evt++) {
        pITree->GetEntry(i_evt);

        int n_trk = tcaTrack->GetEntries();

        stepnum = 0;
        for (int idx_track = 0; idx_track < n_trk; idx_track++) {
            simobj::Track *now_track = static_cast<simobj::Track *>(tcaTrack->At(idx_track));

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

        cout << "EvtID: " << i_evt << " | Stepnum: " << stepnum << endl;

        sort(step_in_fpga.begin(), step_in_fpga.begin() + stepnum);

        HitInfo currentHit;
        for (size_t i_step = 0; i_step < stepnum; i_step++) {
            auto &step = step_in_fpga[i_step].first;
            auto track = static_cast<simobj::Track *>(tcaTrack->At(step_in_fpga[i_step].second));

            if (i_step == 0) {
                currentHit = HitInfo(*track, step, *primary);
            } else {
                if (!currentHit.AppendStep(*track, step)) {
                    FillTreeWithHit(currentHit, i_evt);
                    currentHit = HitInfo(*track, step, *primary);
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
