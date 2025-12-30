#include <iostream>

#include "simobj/Track.h"

#include "TDatabasePDG.h"

namespace simobj {
    Track::Track() : fPDGCode(0), fTrackID(-1), fParentID(-1), fNStep(0), fStepIdxArray() {}

    Track::Track(int pdgCode, int tID, int pID)
        : fPDGCode(pdgCode), fTrackID(tID), fParentID(pID), fNStep(0), fStepIdxArray() {
        using namespace std;
        TDatabasePDG *pdb      = TDatabasePDG::Instance();
        TParticlePDG *particle = pdb->GetParticle(pdgCode);

        if (particle == nullptr) {
            cerr << "The given PDG code " << pdgCode
                 << " does not exist in TDatabasePDG. Naming lookup failed." << endl;
            fName = "";
        } else {
            fName = particle->GetName();
        }
    }

    Track::Track(int pdgCode, std::string pdgName, int tID, int pID)
        : TNamed(pdgName.c_str(), ""), fPDGCode(pdgCode), fTrackID(tID), fParentID(pID), fNStep(0),
          fStepIdxArray() {
        using namespace std;
        fName = pdgName;
    }

    Track::Track(const Track &orig)
        : TNamed(orig), fPDGCode(orig.fPDGCode), fTrackID(orig.fTrackID), fParentID(orig.fParentID),
          fNStep(orig.fNStep) {
        for (size_t i = 0; i < fNStep; i++) {
            fStepIdxArray[i] = orig.fStepIdxArray[i];
        }
    }

    TObject *Track::Clone(const char *) const { return new Track(*this); }

    size_t &Track::GetStepIndex(size_t idx) {
        using namespace std;

        if (fgcMaxStepSize >= idx || fNStep >= idx) {
            cerr << "idx for the step index array is wrong: " << idx << endl;
            throw;
        }

        return fStepIdxArray[idx];
    }

    void Track::Print(Option_t *option) const {
        using namespace std;
        cout << "Track ID: " << fTrackID << endl;
        cout << "Parent Track ID: " << fParentID << endl;
        cout << "PDG Code: " << fPDGCode << endl;
        cout << "Particle name" << fName << endl;
        cout << "Number of step: " << fNStep << " [max: " << fgcMaxStepSize << "]" << endl;
        cout << "Step indexes: ";
        for (size_t i = 0; i < fNStep; i++) {
            cout << fStepIdxArray[i] << " ";
        }
        cout << endl;
    }
} // namespace simobj
