#ifndef __simcore_RootManager_h__
#define __simcore_RootManager_h__

#include <map>
#include <string>

#include "G4Threading.hh"

#include "RootManagerMessenger.h"

namespace ROOT {
    class TBufferMerger;
    class TBufferMergerFile;
} // namespace ROOT

class TClonesArray;
class TTree;

class G4Step;
class G4Track;

namespace simcore {
    struct TLSContainer {
        std::shared_ptr<ROOT::TBufferMergerFile> fFile;
        TTree *fTree;

        int fNTrack;
        std::map<int, size_t> fID2IdxTable;
        TClonesArray *fTCATrack;

        int fNStep;
        TClonesArray *fTCAStep;
    };

    class RootManager {
      public:
        RootManager(const char *filename = "simout.root", const char *treename = "tree")
            : fMessenger(nullptr), fFilename(filename), fTreename(treename), fStarted(false),
              fRecordStep(true), fRecordPrimary(true), fMerger(nullptr) {
            fMessenger = new RootManagerMessenger(this);
        };
        virtual ~RootManager() { delete fMessenger; };

        static RootManager &GetInstance() {
            static RootManager fgInstance;
            return fgInstance;
        }

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

        void RecordStep(bool a = true) { fRecordStep = a; }
        bool DoesRecordStep() const { return fRecordStep; }

        void RecordPrimary(bool a = true) { fRecordPrimary = a; }
        bool DoesRecordPrimary() const { return fRecordPrimary; }

        void SetMaxTrackNum(int a) { fgcMaxTrackNum = a; }
        int GetMaxTrackNum() const { return fgcMaxTrackNum; }

        void SetMaxStepNum(int a) { fgcMaxStepNum = a; }
        int GetMaxStepNum() const { return fgcMaxStepNum; }

        std::string GetFilename() const { return fFilename; }
        std::string GetTreename() const { return fTreename; }

      protected:
        void MakeBranches() const;

      private:
        RootManagerMessenger *fMessenger;

        std::string fFilename;
        std::string fTreename;

        bool fStarted;
        bool fRecordStep;
        bool fRecordPrimary;
        ROOT::TBufferMerger *fMerger;

        inline static G4Mutex fgcStartMutex = G4MUTEX_INITIALIZER;

        inline static int fgcMaxTrackNum = 20000;
        inline static int fgcMaxStepNum  = 10000000;

        static G4ThreadLocal TLSContainer *fgTLS;
    };
}; // namespace simcore

#endif
