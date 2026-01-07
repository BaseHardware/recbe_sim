#ifndef __simobj_Metadata_h__
#define __simobj_Metadata_h__

#include "TObject.h"
#include "TString.h"

#include <vector>

namespace simobj {
    class Metadata : public TObject {
      public:
        Metadata()                     = default;
        Metadata(const Metadata &orig) = default;
        virtual ~Metadata() {};

        TObject *Clone(const char *) const override;

        void SetSimulationName(const std::string &);
        std::string GetSimulationName() const;

        void SetGeometryType(const std::string &);
        std::string GetGeometryType() const;

        void SetGitHash(const std::string &);
        std::string GetGitHash() const;

        void SetGitDescription(const std::string &);
        std::string GetGitDescription() const;

        void SetOutputTreename(const std::string &);
        std::string GetOutputTreename() const;

        void SetAllStepRecorded(bool a) { fStepRecorded = a; }
        bool GetAllStepRecorded() const { return fStepRecorded; }

        void SetPrimaryRecorded(bool a) { fPrimaryRecorded = a; }
        bool GetPrimaryRecorded() const { return fPrimaryRecorded; }

        void SetMaxTrackNum(size_t a) { fMaxNTrack = a; }
        size_t GetMaxTrackNum() const { return fMaxNTrack; }

        void SetMaxStepNum(size_t a) { fMaxNStep = a; }
        size_t GetMaxStepNum() const { return fMaxNStep; }

        void SetThreadNum(size_t a) { fNThreads = a; }
        size_t GetThreadNum() const { return fNThreads; }

        void SetNumberOfRequestedEvents(int a) { fNReqEvts = a; }
        int GetNumberOfRequestedEvents() const { return fNReqEvts; }

        void SetNumberOfProcessedEvents(int a) { fNEvts = a; }
        int GetNumberOfProcessedEvents() const { return fNEvts; }

        void SetRandomSeed(long a, bool aux);
        long GetRandomSeed(bool aux) const;

        bool ReadGeometryFile(const std::string &filepath);
        std::vector<unsigned char> GetGeometryData() const;

        void Print(Option_t *option = "") const override;
        void Clear(Option_t *option = "") override;

      protected:
        TString fSimName;
        TString fGeomType;
        TString fGitHash;
        TString fGitDescription;
        TString fOutputTreename;

        bool fStepRecorded;
        bool fPrimaryRecorded;

        long fRandomSeeds[2];

        size_t fNThreads;
        size_t fMaxNTrack;
        size_t fMaxNStep;

        int fNEvts;
        int fNReqEvts;

        std::vector<unsigned char> fGeometryData;

        ClassDefOverride(simobj::Metadata, 1)
    };
} // namespace simobj
#endif
