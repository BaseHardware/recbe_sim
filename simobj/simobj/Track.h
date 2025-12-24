#ifndef __simobj_Track_h__
#define __simobj_Track_h__

#include "TNamed.h"

namespace simobj {
    class Track : public TNamed {
      public:
        Track();
        Track(int pdgCode, int trkID, int parID);
        Track(int pdgCode, std::string pdgName, int trkID, int parID);
        Track(const Track &orig);
        virtual ~Track() {};

        TObject *Clone(const char *) const override;

        bool AppendStepIdx(size_t idx) {
            if (IsStepArrayFull()) {
                return false;
            } else {
                fStepIdxArray[fNStep++] = idx;
                return true;
            }
        }
        void ClearStepIdxArray() { fNStep = 0; }
        size_t GetNStep() const { return fNStep; }

        bool IsStepArrayFull() const { return fgcMaxStepSize == fNStep; }

        size_t &GetStepIndex(size_t idx);
        size_t &operator[](const size_t idx) { return GetStepIndex(idx); }
        size_t operator[](const size_t idx) const {
            return const_cast<Track *>(this)->GetStepIndex(idx);
        }

        void SetPDGCode(int a) { fPDGCode = a; }
        void SetTrackID(int a) { fTrackID = a; }
        void SetParentID(int a) { fParentID = a; }
        int GetPDGCode() const { return fPDGCode; }
        int GetTrackID() const { return fTrackID; }
        int GetParentID() const { return fParentID; }

        void Print(Option_t *option = "") const override;

        static const int fgcMaxStepSize = 10000;

      private:
        int fPDGCode;
        int fTrackID;
        int fParentID;

        size_t fNStep;
        size_t fStepIdxArray[fgcMaxStepSize];

        ClassDefOverride(simobj::Track, 1)
    };
} // namespace simobj
#endif
