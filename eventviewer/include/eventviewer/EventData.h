#ifndef __eventviewer_EventData_h__
#define __eventviewer_EventData_h__

#include "RtypesCore.h"
#include "TFile.h"

#include <memory>
#include <string>
#include <unordered_map>

class TClonesArray;
class TTree;

namespace simobj {
    class Metadata;
    class Primary;
    class Step;
    class Track;
} // namespace simobj

namespace eventviewer {
    class EventData {
      public:
        bool Open(const std::string &filename);
        bool ImportGeometry(int graphicalVerbosity);
        bool LoadEntry(Long64_t eventIndex);
        void CleanupGeometryFile();

        std::string MetadataText() const;
        std::string TrackListText() const;
        std::string StepListText() const;

        std::string filename;
        std::string geometryPath;
        std::unique_ptr<TFile> file;
        TTree *tree                = nullptr;
        TTree *persistentTree      = nullptr;
        bool complete              = false;
        TClonesArray *tracks       = nullptr;
        TClonesArray *steps        = nullptr;
        simobj::Primary *primary   = nullptr;
        simobj::Metadata *metadata = nullptr;
        Long64_t currentEvent      = 0;
        Long64_t eventCount        = 0;
        std::unordered_map<size_t, int> stepToTrackId;

      private:
        bool LoadMetadata();
        void BuildStepToTrackMap();

        static std::string FormatStep(const simobj::Step &step, int idx, int trackId);
        static std::string FormatTrack(const simobj::Track &track, int idx);
    };
} // namespace eventviewer

#endif
