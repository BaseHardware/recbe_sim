#include <iostream>
#include <string>

#include "simobj/Metadata.h"

namespace simobj {
    TObject *Metadata::Clone(const char *) const {
        return static_cast<Metadata *>(new Metadata(*this));
    }

    void Metadata::SetSimulationName(const std::string &a) { fSimName = a; }
    std::string Metadata::GetSimulationName() const { return fSimName.Data(); }

    void Metadata::SetGeometryType(const std::string &a) { fGeomType = a; }
    std::string Metadata::GetGeometryType() const { return fGeomType.Data(); }

    void Metadata::SetGitHash(const std::string &a) { fGitHash = a; }
    std::string Metadata::GetGitHash() const { return fGitHash.Data(); }

    void Metadata::Print(Option_t *option) const {
        using namespace std;
        cout << "Simulation name: " << fSimName << endl;
        cout << "Geometry type: " << fGeomType << endl;
        cout << "Git hash for the simulation binary:" << fGitHash << endl;
    }
} // namespace simobj
