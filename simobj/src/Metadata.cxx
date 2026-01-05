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

    void Metadata::SetOutputTreename(const std::string &a) { fOutputTreename = a; }
    std::string Metadata::GetOutputTreename() const { return fOutputTreename.Data(); }

    void Metadata::Print(Option_t *option) const {
        using namespace std;
        cout << "Simulation name: " << fSimName << endl;
        cout << "Geometry type: " << fGeomType << endl;
        cout << "Name of output tree: " << fOutputTreename << endl;
        cout << "Git hash for the simulation binary:" << fGitHash << endl;
        cout << "All step recording: " << (fStepRecorded ? "Yes" : "No") << endl;
        cout << "Primary vertex recording: " << (fPrimaryRecorded ? "Yes" : "No") << endl;
        cout << "The maximum number of tracks: " << fMaxNTrack << endl;
        cout << "The maximum number of steps: " << fMaxNStep << endl;
    }
} // namespace simobj
