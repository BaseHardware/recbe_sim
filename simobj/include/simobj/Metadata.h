#ifndef __simobj_Metadata_h__
#define __simobj_Metadata_h__

#include "TObject.h"
#include "TString.h"

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

        void Print(Option_t *option = "") const override;

      protected:
        TString fSimName;
        TString fGeomType;
        TString fGitHash;
        TString fOutputTreename;

        bool fStepRecorded;
        bool fPrimaryRecorded;

        size_t fMaxNTrack;
        size_t fMaxNStep;

        ClassDefOverride(simobj::Metadata, 1)
    };
} // namespace simobj
#endif
