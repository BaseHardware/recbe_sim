#ifndef __simcore_RootManager_h__
#define __simcore_RootManager_h__

#include <string>
#include <map>

#include "G4Threading.hh"

#include "ROOT/TBufferMerger.hxx"
class TClonesArray;
class TTree;

class G4Step;
class G4Track;

namespace simcore {

    struct TLSContainer {
        std::shared_ptr<ROOT::TBufferMergerFile> fFile;
        TTree *fTree;

        Int_t fNTrack;
        std::map<int, size_t> fID2IdxTable;
        TClonesArray *fTCATrack;

        Int_t fNStep;
        TClonesArray *fTCAStep;
    };

    class RootManager {
      public:
        RootManager(const char *filename = "simout.root", const char *treename = "tree")
            : fFilename(filename), fTreename(treename), fStarted(false), fMerger(nullptr) {};

        static RootManager &GetInstance() { return fgInstance; }

        bool StartRunMaster();
        bool EndRunMaster();
        bool StartRunSlave();
        bool EndRunSlave();

        bool IsStarted() const { return fStarted; }

        void SetFilename(const char *a) { fFilename = a; }
        void SetTreename(const char *a) { fTreename = a; }

        void Fill() const;
        void Clear() const;

        bool StartTrack(const G4Track *track) const;
        bool AppendStep(const G4Step *step) const;

        std::string GetFilename() const { return fFilename; }
        std::string GetTreename() const { return fTreename; }

      private:
        std::string fFilename;
        std::string fTreename;

        bool fStarted;
        ROOT::TBufferMerger *fMerger;

        inline static G4Mutex fgcMasterMutex = G4MUTEX_INITIALIZER;
        inline static G4Mutex fgcSlaveMutex  = G4MUTEX_INITIALIZER;

        inline static Int_t fgcMaxTrackNum = 20000;
        inline static Int_t fgcMaxStepNum  = 10000000;

        static RootManager fgInstance;
        static G4ThreadLocal TLSContainer *fgTLS;
    };
}; // namespace simcore

#endif
