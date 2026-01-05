#include "simcore/MetadataManager.h"
#include "simcore/RootManager.h"

#include <array>
#include <filesystem>
#include <random>

#include "recbesim/GitInformation.h"

#include "simobj/Metadata.h"

#include "G4GDMLParser.hh"
#include "G4RunManager.hh"
#include "Randomize.hh"

constexpr std::array<char, 62> MakeNAArray() {
    std::array<char, 62> a{};
    int idx = 0;
    for (char c = '0'; c <= '9'; ++c)
        a[idx++] = c;
    for (char c = 'A'; c <= 'Z'; ++c)
        a[idx++] = c;
    for (char c = 'a'; c <= 'z'; ++c)
        a[idx++] = c;
    return a;
}

std::string MakeRandomString(size_t length = 10) {
    static constexpr std::array<char, 62> kNumericAndAlphabet = MakeNAArray();

    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    std::mt19937_64 gen(seed);

    std::uniform_int_distribution<std::size_t> dist(0, kNumericAndAlphabet.size() - 1);

    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        s.push_back(kNumericAndAlphabet[dist(gen)]);
    }
    return s;
}

namespace simcore {
    void MetadataManager::FillMetadata(simobj::Metadata *target) const {
        using namespace std;
        using namespace recbesim;
        GitInformation &gitInfo = GitInformation::GetInstance();
        RootManager &rootman    = RootManager::GetInstance();

        target->SetGeometryType(fGeomTypename);
        target->SetSimulationName(fSimName);
        target->SetGitHash(gitInfo.GetHash());
        target->SetGitDescription(gitInfo.GetDescription());

        target->SetMaxTrackNum(rootman.GetMaxTrackNum());
        target->SetMaxStepNum(rootman.GetMaxStepNum());
        target->SetAllStepRecorded(rootman.DoesRecordStep());
        target->SetPrimaryRecorded(rootman.DoesRecordPrimary());
        target->SetOutputTreename(rootman.GetTreename());

        const long *seeds;
        if (fAutoRandomSeed) {
            seeds = G4Random::getTheSeeds();
        } else {
            seeds = fRandomSeeds;
        }
        target->SetRandomSeed(seeds[0], false);
        target->SetRandomSeed(seeds[1], true);

        target->SetThreadNum(G4RunManager::GetRunManager()->GetNumberOfThreads());

        string temp_filename = MakeRandomString();
        temp_filename += ".gdml";

        filesystem::path tempfile_path = filesystem::temp_directory_path() / temp_filename;
        G4GDMLParser parser;
        parser.Write(tempfile_path.c_str());
        target->ReadGeometryFile(tempfile_path);
        filesystem::remove(tempfile_path);
    }

    void MetadataManager::SetRandomSeed(long a, bool aux) {
        if (aux)
            fRandomSeeds[1] = a;
        else
            fRandomSeeds[0] = a;
    }

    long MetadataManager::GetRandomSeed(bool aux) const {
        if (aux)
            return fRandomSeeds[1];
        else
            return fRandomSeeds[0];
    }
} // namespace simcore
