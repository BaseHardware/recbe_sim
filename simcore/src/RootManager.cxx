#include "simcore/RootManager.h"
#include "simcore/MetadataManager.h"
#include "simcore/SafeTermination.h"

#include "ROOT/TBufferMerger.hxx"

#include "TClonesArray.h"
#include "TTree.h"

#include "G4AutoLock.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4TouchableHandle.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include "simobj/Metadata.h"
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"

template <typename T>
static G4int GetEnvelopeCopyNo(const T *trk) {
    G4int retval = -1;

    const G4TouchableHandle &nowHandle = trk->GetTouchableHandle();

    G4int hDepth = nowHandle->GetHistoryDepth();
    if (hDepth <= 1) return retval;

    for (G4int depth = 1; depth < hDepth - 1; depth++) {
        G4VPhysicalVolume *nowPV = nowHandle->GetVolume(depth);
        if (nowPV->GetName().find("Envelope") != nowPV->GetName().npos) {
            retval = nowHandle->GetCopyNumber(depth);
            break;
        }
    }

    return retval;
}

static void G4Step2SimStep(const G4Step *src, simobj::Step *dest) {
    G4ThreeVector pos = src->GetPostStepPoint()->GetPosition();
    G4double glob_t   = src->GetPostStepPoint()->GetGlobalTime();

    G4ThreeVector mom = src->GetPostStepPoint()->GetMomentum();
    G4double energy   = src->GetPostStepPoint()->GetKineticEnergy();

    G4double prop_t = src->GetPostStepPoint()->GetProperTime();
    G4double edep   = src->GetTotalEnergyDeposit();
    G4double niEdep = src->GetNonIonizingEnergyDeposit();

    G4int nDaug = src->GetNumberOfSecondariesInCurrentStep();
    G4int copyNo, envCopyNo;

    std::string procName, volName;

    const G4VProcess *nowProcess = src->GetPostStepPoint()->GetProcessDefinedStep();
    if (nowProcess == nullptr) {
        procName = "initStep";
    } else {
        procName = nowProcess->GetProcessName();
    }

    const G4VPhysicalVolume *nowVolume = src->GetPostStepPoint()->GetPhysicalVolume();
    if (nowVolume == nullptr) {
        volName   = "OutOfWorld";
        copyNo    = -1;
        envCopyNo = -1;
    } else {
        volName   = nowVolume->GetName();
        copyNo    = nowVolume->GetCopyNo();
        envCopyNo = GetEnvelopeCopyNo(src->GetPostStepPoint());
    }

    dest->SetNDaughters(nDaug);
    dest->SetDepositedEnergy(edep / MeV);
    dest->SetNonIonDepositedEnergy(niEdep / MeV);
    dest->SetProperTime(prop_t / ns);
    dest->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    dest->SetPxPyPzE(mom.x() / MeV, mom.y() / MeV, mom.z() / MeV, energy / MeV);
    dest->SetProcessName(procName.c_str());
    dest->SetVolumeName(volName.c_str());
    dest->SetCopyNumber(copyNo);
    dest->SetEnvelopeCopyNumber(envCopyNo);
}

static void G4Track2SimTrack(const G4Track *src, simobj::Track *dest, G4bool start) {
    G4ThreeVector pos = src->GetPosition();
    G4double glob_t   = src->GetGlobalTime();

    G4ThreeVector mom = src->GetMomentum();
    G4double energy   = src->GetKineticEnergy();

    G4double prop_t = src->GetProperTime();
    G4double edep, niEdep;

    G4int nDaug, copyNo, envCopyNo;

    std::string procName, volName;

    simobj::Step *destStep;

    if (start) {
        procName = "initStep";
        volName  = src->GetVolume()->GetName();
        edep     = 0;
        niEdep   = 0;

        nDaug     = 0;
        copyNo    = src->GetVolume()->GetCopyNo();
        envCopyNo = GetEnvelopeCopyNo(src);
        destStep  = &dest->FirstStep();
    } else {
        const G4Step *nowStep            = src->GetStep();
        const G4StepPoint *postStepPoint = nowStep->GetPostStepPoint();
        const G4VProcess *nowProcess     = postStepPoint->GetProcessDefinedStep();
        const G4VPhysicalVolume *nowVol  = postStepPoint->GetPhysicalVolume();

        procName = nowProcess->GetProcessName();
        if (nowVol != nullptr) {
            volName   = nowVol->GetName();
            copyNo    = nowVol->GetCopyNo();
            envCopyNo = GetEnvelopeCopyNo(src);
        } else {
            volName   = "OutOfWorld";
            copyNo    = -1;
            envCopyNo = -1;
        }

        nDaug    = nowStep->GetNumberOfSecondariesInCurrentStep();
        edep     = nowStep->GetTotalEnergyDeposit();
        niEdep   = nowStep->GetNonIonizingEnergyDeposit();
        destStep = &dest->FinalStep();
    }

    destStep->SetNDaughters(nDaug);
    destStep->SetDepositedEnergy(edep / MeV);
    destStep->SetNonIonDepositedEnergy(niEdep / MeV);
    destStep->SetProperTime(prop_t / ns);
    destStep->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    destStep->SetPxPyPzE(mom.x() / mm, mom.y() / mm, mom.z() / mm, energy / MeV);
    destStep->SetProcessName(procName.c_str());
    destStep->SetVolumeName(volName.c_str());
    destStep->SetCopyNumber(copyNo);
    destStep->SetEnvelopeCopyNumber(envCopyNo);
}

namespace simcore {
    G4ThreadLocal TLSContainer *RootManager::fgTLS = nullptr;

    void RootManager::Fill() const {
        size_t &filledBytes = fgTLS->fFilledBytes;

        fgTLS->fBranches.fComplete = (!fgTLS->fTrackDisabled) && (!fgTLS->fStepDisabled);

        filledBytes += fgTLS->fTree->Fill();
        if (fTreeBufferLimit != 0 && filledBytes >= fTreeBufferLimit) {
            G4cout << "The size of raw data filled into TTree is " << filledBytes / 1024. / 1024.
                   << " MiB (> " << fTreeBufferLimit / 1024. / 1024.
                   << " MiB). The I/O buffer will be flushed." << G4endl;
            fgTLS->fTree->FlushBaskets();
            fgTLS->fFile->Write();
            filledBytes = 0;
        }
        SafeTermination::RestoreSignalHandler();
    }

    void RootManager::Clear() const {
        if (fgTLS->fBranches.fTCAStep != nullptr) fgTLS->fBranches.fTCAStep->Clear("C");
        fgTLS->fBranches.fTCATrack->Clear("C");

        fgTLS->fNTrack = 0;
        fgTLS->fNStep  = 0;

        fgTLS->fTrackDisabled = false;
        fgTLS->fStepDisabled  = false;

        fgTLS->fID2IdxTable.clear();
    }

    bool RootManager::CheckTrack(const G4Track *track, G4bool start) const {
        using namespace simobj;

        if (fgTLS->fTrackDisabled) return true;

        if (fgcMaxTrackNum <= fgTLS->fNTrack) {
            G4cerr << "WARNING: The number of tracks exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxStepNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        Track *nowTrack;
        if (start) {
            fgTLS->fID2IdxTable[track->GetTrackID()] = fgTLS->fNTrack;

            nowTrack = new ((*fgTLS->fBranches.fTCATrack)[fgTLS->fNTrack]) Track(
                track->GetDefinition()->GetPDGEncoding(), track->GetDefinition()->GetParticleName(),
                track->GetTrackID(), track->GetParentID());

            fgTLS->fNTrack++;
        } else {
            nowTrack = static_cast<Track *>(
                fgTLS->fBranches.fTCATrack->At(fgTLS->fID2IdxTable[track->GetTrackID()]));
        }
        G4Track2SimTrack(track, nowTrack, start);

        return true;
    }

    bool RootManager::AppendStep(const G4Step *step) const {
        using namespace simobj;

        const G4Track *track = step->GetTrack();

        if (fgTLS->fStepDisabled) {
            return true;
        } else if (!fRecordStep) {
            return false;
        } else if (fgTLS->fTrackDisabled &
                   fgTLS->fID2IdxTable.find(track->GetTrackID()) == fgTLS->fID2IdxTable.end()) {
            return true;
        }

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxStepNum
                   << "). This step will not be added." << G4endl;
            return false;
        }

        Track *tcaTrack = static_cast<Track *>(
            (*fgTLS->fBranches.fTCATrack)[fgTLS->fID2IdxTable[track->GetTrackID()]]);
        tcaTrack->AppendStepIdx(fgTLS->fNStep);

        Step *newStep = (new ((*fgTLS->fBranches.fTCAStep)[fgTLS->fNStep++]) Step());

        G4Step2SimStep(step, newStep);
        return true;
    }

    bool RootManager::StorePrimary(const G4Event *event) const {
        using namespace simobj;

        if (!fRecordPrimary) return false;

        int partCnt = 0;
        int nVert   = event->GetNumberOfPrimaryVertex();
        for (int idxVert = 0; idxVert < nVert; idxVert++) {
            const G4PrimaryVertex *nowVert = event->GetPrimaryVertex(idxVert);

            Vertex *newVertex = new (fgTLS->fBranches.fPrimary->GetVertexObjPtr(idxVert)) Vertex();
            if (fgTLS->fBranches.fPrimary->GetVertexSize() <= idxVert + 1) {
                G4cerr << "WARNING: The number of primary vertices exceeds the maximum number."
                          "This vertex will not be added."
                       << G4endl;
                return false;
            }
            newVertex->SetXYZT(nowVert->GetX0() / mm, nowVert->GetY0() / mm, nowVert->GetZ0() / mm,
                               nowVert->GetT0() / ns);
            newVertex->SetWeight(nowVert->GetWeight());

            int nPart = nowVert->GetNumberOfParticle();
            newVertex->SetNParticle(nPart);
            for (int idxPart = 0; idxPart < nPart; idxPart++) {
                G4PrimaryParticle *nowPart = nowVert->GetPrimary(idxPart);
                if (fgTLS->fBranches.fPrimary->GetPrimaryParticleSize() <= partCnt + 1) {
                    G4cerr << "WARNING: The number of primary particles exceeds the maximum number."
                              "This particle will not be added."
                           << G4endl;
                    break;
                }

                PrimaryParticle *newPP =
                    new (fgTLS->fBranches.fPrimary->GetPrimaryParticleObjPtr(partCnt++))
                        PrimaryParticle();
                newPP->SetVertexIdx(idxVert);
                newPP->SetPxPyPzE(nowPart->GetPx() / MeV, nowPart->GetPy() / MeV,
                                  nowPart->GetPz() / MeV, nowPart->GetKineticEnergy() / MeV);
                newPP->SetPolX(nowPart->GetPolX());
                newPP->SetPolY(nowPart->GetPolY());
                newPP->SetPolZ(nowPart->GetPolZ());
                newPP->SetWeight(nowPart->GetWeight());
                newPP->SetPDGCode(nowPart->GetPDGcode());
            }
        }

        return true;
    }

    bool RootManager::StartRunMaster() {
        if (fStarted) return false;

        fStarted = true;
        fMerger  = new ROOT::TBufferMerger(fFilename.c_str());

        fFileForMaster = fMerger->GetFile();
        fPTree =
            new TTree(fPTreename.c_str(), "An instance of TTree for the persistency information");
        if (fPTree == nullptr) return false;
        fPTree->ResetBit(kMustCleanup);
        fPTree->SetDirectory(fFileForMaster.get());

        fMetadata = new simobj::Metadata();
        fPTree->Branch("Metadata", &fMetadata);

        return true;
    }

    bool RootManager::EndRunMaster() {
        if (!fStarted) return false;

        MetadataManager::GetInstance().FillMetadata(fMetadata);
        fPTree->Fill();

        fFileForMaster->Write();

        delete fPTree;
        fFileForMaster.reset();

        delete fMetadata;

        delete fMerger;

        fMerger   = nullptr;
        fPTree    = nullptr;
        fMetadata = nullptr;
        fStarted  = false;

        return true;
    }

    void RootManager::MakeBranches() const {
        fgTLS->fTree->Branch("complete", &fgTLS->fBranches.fComplete);

        fgTLS->fBranches.fTCATrack = new TClonesArray("simobj::Track", fgcMaxTrackNum);
        fgTLS->fNTrack             = 0;
        fgTLS->fTree->Branch("Tracks", &fgTLS->fBranches.fTCATrack);

        fgTLS->fID2IdxTable.clear();

        fgTLS->fNStep = 0;
        if (fRecordStep) {
            fgTLS->fBranches.fTCAStep = new TClonesArray("simobj::Step", fgcMaxStepNum);
            fgTLS->fTree->Branch("Steps", &fgTLS->fBranches.fTCAStep);
        } else {
            fgTLS->fBranches.fTCAStep = nullptr;
        }

        if (fRecordPrimary) {
            fgTLS->fBranches.fPrimary = new simobj::Primary;
            fgTLS->fTree->Branch("Primary", &fgTLS->fBranches.fPrimary);
        } else {
            fgTLS->fBranches.fPrimary = nullptr;
        }
    }

    bool RootManager::StartRunSlave() {
        G4AutoLock lock(&fgcStartMutex);
        if (!fStarted || fgTLS) return false;

        fgTLS = new TLSContainer;

        fgTLS->fFile = fMerger->GetFile();

        fgTLS->fTree =
            new TTree(fTreename.c_str(), "An instance of TTree for the simulation output");
        fgTLS->fFilledBytes = 0;

        if (fgTLS->fTree == nullptr) return false;
        fgTLS->fTree->ResetBit(kMustCleanup);
        fgTLS->fTree->SetDirectory(fgTLS->fFile.get());

        MakeBranches();

        return true;
    }

    bool RootManager::EndRunSlave() {
        G4AutoLock lock(&fgcStartMutex);
        if (!fStarted || !fgTLS) return false;

        fgTLS->fFile->Write();

        delete fgTLS->fTree;
        fgTLS->fFile.reset();

        delete fgTLS->fBranches.fTCATrack;

        if (fRecordStep) delete fgTLS->fBranches.fTCAStep;
        if (fRecordPrimary) delete fgTLS->fBranches.fPrimary;

        delete fgTLS;
        fgTLS = nullptr;

        return true;
    }
} // namespace simcore
