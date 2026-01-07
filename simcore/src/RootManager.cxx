#include "simcore/RootManager.h"
#include "simcore/MetadataManager.h"

#include "ROOT/TBufferMerger.hxx"

#include "TClonesArray.h"
#include "TTree.h"

#include "G4AutoLock.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include "simobj/Metadata.h"
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"

static void G4Step2SimStep(const G4Step *src, simobj::Step *dest) {
    G4ThreeVector pos = src->GetPostStepPoint()->GetPosition();
    G4double glob_t   = src->GetPostStepPoint()->GetGlobalTime();

    G4ThreeVector mom = src->GetPostStepPoint()->GetMomentum();
    G4double energy   = src->GetPostStepPoint()->GetKineticEnergy();

    G4double prop_t = src->GetPostStepPoint()->GetProperTime();
    G4double edep   = src->GetTotalEnergyDeposit();

    G4int nDaug = src->GetNumberOfSecondariesInCurrentStep();

    std::string procName, volName;

    const G4VProcess *nowProcess = src->GetPostStepPoint()->GetProcessDefinedStep();
    if (nowProcess == nullptr) {
        procName = "initStep";
    } else {
        procName = nowProcess->GetProcessName();
    }

    const G4VPhysicalVolume *nowVolume = src->GetPostStepPoint()->GetPhysicalVolume();
    if (nowVolume == nullptr) {
        volName = "OutOfWorld";
    } else {
        volName = nowVolume->GetName();
    }

    dest->SetNDaughters(nDaug);
    dest->SetDepositedEnergy(edep / MeV);
    dest->SetProperTime(prop_t / ns);
    dest->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    dest->SetPxPyPzE(mom.x() / MeV, mom.y() / MeV, mom.z() / MeV, energy / MeV);
    dest->SetProcessName(procName.c_str());
    dest->SetVolumeName(volName.c_str());
}

static void G4Track2SimStep(const G4Track *src, simobj::Step *dest, G4bool start) {
    G4ThreeVector pos = src->GetPosition();
    G4double glob_t   = src->GetGlobalTime();

    G4ThreeVector mom = src->GetMomentum();
    G4double energy   = src->GetKineticEnergy();

    G4double prop_t = src->GetProperTime();
    G4double edep;

    G4int nDaug;

    std::string procName, volName;

    if (start) {
        procName = "initStep";
        volName  = src->GetVolume()->GetName();
        edep     = 0;

        nDaug = 0;
    } else {
        const G4Step *nowStep            = src->GetStep();
        const G4StepPoint *postStepPoint = nowStep->GetPostStepPoint();
        const G4VProcess *nowProcess     = postStepPoint->GetProcessDefinedStep();
        const G4VPhysicalVolume *nowVol  = postStepPoint->GetPhysicalVolume();

        procName = nowProcess->GetProcessName();
        if (nowVol != nullptr) {
            volName = nowVol->GetName();
        } else {
            volName = "OutOfWorld";
        }

        nDaug = nowStep->GetNumberOfSecondariesInCurrentStep();
        edep  = nowStep->GetTotalEnergyDeposit();
    }

    dest->SetNDaughters(nDaug);
    dest->SetDepositedEnergy(edep / MeV);
    dest->SetProperTime(prop_t / ns);
    dest->SetXYZT(pos.x() / mm, pos.y() / mm, pos.z() / mm, glob_t / ns);
    dest->SetPxPyPzE(mom.x() / mm, mom.y() / mm, mom.z() / mm, energy / MeV);
    dest->SetProcessName(procName.c_str());
    dest->SetVolumeName(volName.c_str());
}

namespace simcore {
    G4ThreadLocal TLSContainer *RootManager::fgTLS = nullptr;

    void RootManager::Fill() const { fgTLS->fTree->Fill(); }

    void RootManager::Clear() const {
        fgTLS->fTCAStep->Clear("C");
        fgTLS->fTCATrack->Clear("C");

        fgTLS->fNTrack = 0;
        fgTLS->fNStep  = 0;

        fgTLS->fID2IdxTable.clear();
    }

    bool RootManager::CheckTrack(const G4Track *track, G4bool start) const {
        using namespace simobj;

        if (!start && fRecordStep) return false;

        if (fgcMaxTrackNum <= fgTLS->fNTrack) {
            G4cerr << "WARNING: The number of tracks exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This track will not be added." << G4endl;
            return false;
        }

        Track *nowTrack;
        if (start) {
            fgTLS->fID2IdxTable[track->GetTrackID()] = fgTLS->fNTrack;

            nowTrack = new ((*fgTLS->fTCATrack)[fgTLS->fNTrack]) Track(
                track->GetDefinition()->GetPDGEncoding(), track->GetDefinition()->GetParticleName(),
                track->GetTrackID(), track->GetParentID());

            fgTLS->fNTrack++;
        } else {
            nowTrack = static_cast<Track *>(
                fgTLS->fTCATrack->At(fgTLS->fID2IdxTable[track->GetTrackID()]));
        }

        nowTrack->AppendStepIdx(fgTLS->fNStep);
        Step *newStep = (new ((*fgTLS->fTCAStep)[fgTLS->fNStep++]) Step());
        G4Track2SimStep(track, newStep, start);

        return true;
    }

    bool RootManager::AppendStep(const G4Step *step) const {
        using namespace simobj;

        if (!fRecordStep) return false;

        if (fgcMaxStepNum <= fgTLS->fNStep) {
            G4cerr << "WARNING: The number of steps exceeds the maximum number (" << fgcMaxTrackNum
                   << "). This step will not be added." << G4endl;
            return false;
        }

        const G4Track *track = step->GetTrack();
        Track *tcaTrack =
            static_cast<Track *>((*fgTLS->fTCATrack)[fgTLS->fID2IdxTable[track->GetTrackID()]]);
        tcaTrack->AppendStepIdx(fgTLS->fNStep);

        Step *newStep = (new ((*fgTLS->fTCAStep)[fgTLS->fNStep++]) Step());

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

            Vertex *newVertex = new (fgTLS->fPrimary->GetVertexObjPtr(idxVert)) Vertex();
            if (fgTLS->fPrimary->GetVertexSize() <= idxVert + 1) {
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
                if (fgTLS->fPrimary->GetPrimaryParticleSize() <= partCnt + 1) {
                    G4cerr << "WARNING: The number of primary particles exceeds the maximum number."
                              "This particle will not be added."
                           << G4endl;
                    break;
                }

                PrimaryParticle *newPP =
                    new (fgTLS->fPrimary->GetPrimaryParticleObjPtr(partCnt++)) PrimaryParticle();
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
        fgTLS->fTCATrack = new TClonesArray("simobj::Track", fgcMaxTrackNum);
        fgTLS->fNTrack   = 0;
        fgTLS->fTree->Branch("Tracks", &fgTLS->fTCATrack);

        fgTLS->fNStep   = 0;
        fgTLS->fTCAStep = new TClonesArray("simobj::Step", fgcMaxStepNum);
        fgTLS->fTree->Branch("Steps", &fgTLS->fTCAStep);

        if (fRecordPrimary) {
            fgTLS->fPrimary = new simobj::Primary;
            fgTLS->fTree->Branch("Primary", &fgTLS->fPrimary);
        } else {
            fgTLS->fPrimary = nullptr;
        }
    }

    bool RootManager::StartRunSlave() {
        G4AutoLock lock(&fgcStartMutex);
        if (!fStarted || fgTLS) return false;

        fgTLS = new TLSContainer;

        fgTLS->fFile = fMerger->GetFile();

        fgTLS->fTree =
            new TTree(fTreename.c_str(), "An instance of TTree for the simulation output");
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

        delete fgTLS->fTCATrack;
        delete fgTLS->fTCAStep;

        if (fRecordPrimary) delete fgTLS->fPrimary;

        delete fgTLS;
        fgTLS = nullptr;

        return true;
    }
} // namespace simcore
