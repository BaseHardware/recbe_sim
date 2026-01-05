#include <fstream>
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

    void Metadata::SetGitDescription(const std::string &a) { fGitDescription = a; }
    std::string Metadata::GetGitDescription() const { return fGitDescription.Data(); }

    void Metadata::SetOutputTreename(const std::string &a) { fOutputTreename = a; }
    std::string Metadata::GetOutputTreename() const { return fOutputTreename.Data(); }

    void Metadata::SetRandomSeed(long a, bool aux) {
        if (aux)
            fRandomSeeds[1] = a;
        else
            fRandomSeeds[0] = a;
    }

    long Metadata::GetRandomSeed(bool aux) const {
        if (aux)
            return fRandomSeeds[1];
        else
            return fRandomSeeds[0];
    }

    bool Metadata::ReadGeometryFile(const std::string &filepath) {
        std::ifstream infile(filepath, std::ios::binary | std::ios::ate);
        if (!infile) {
            throw std::runtime_error("The input file is not opened");
        }

        const std::streamsize filesize = infile.tellg();
        if (filesize < 0) {
            throw std::runtime_error("tellg() failed.");
        }
        infile.seekg(0, std::ios::beg);

        fGeometryData.resize(static_cast<size_t>(filesize + 1));
        if (!infile.read(reinterpret_cast<char *>(fGeometryData.data()), filesize)) {
            throw std::runtime_error("Failed to read file.");
        }
        fGeometryData.back() = 0;
        infile.close();

        return true;
    }

    std::vector<unsigned char> Metadata::GetGeometryData() const { return fGeometryData; }

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
        cout << "Random seed pair: (" << fRandomSeeds[0] << ", " << fRandomSeeds[1] << ")" << endl;
        cout << "Number of threads: " << fNThreads << endl;
        cout << "Size of geometry data: " << fGeometryData.size() << endl;
    }

    void Metadata::Clear(Option_t *option) {
        fSimName        = "";
        fGeomType       = "";
        fGitHash        = "";
        fOutputTreename = "";

        fStepRecorded    = false;
        fPrimaryRecorded = false;

        fRandomSeeds[0] = 0;
        fRandomSeeds[1] = 0;

        fNThreads  = 0;
        fMaxNTrack = 0;
        fMaxNStep  = 0;

        fGeometryData.clear();
    }
} // namespace simobj
