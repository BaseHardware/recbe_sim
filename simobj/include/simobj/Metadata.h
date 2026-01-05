#ifndef __simobj_Metadata_h__
#define __simobj_Metadata_h__

#include "TObject.h"
#include "TString.h"

#include <string>

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

        void Print(Option_t *option = "") const override;

      protected:
        TString fSimName;
        TString fGeomType;
        TString fGitHash;

        ClassDefOverride(simobj::Metadata, 1)
    };
} // namespace simobj
#endif
