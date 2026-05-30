#ifndef __eventviewer_EventViewer_h__
#define __eventviewer_EventViewer_h__

#include "TGFrame.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TCanvas;
class TClonesArray;
class TFile;
class TGCheckButton;
class TGLabel;
class TGNumberEntry;
class TGTab;
class TGTextButton;
class TGTextView;
class TGLEmbeddedViewer;
class TTree;
class TObject;

namespace simobj {
    class Metadata;
    class Primary;
    class Step;
    class Track;
} // namespace simobj

namespace eventviewer {
    class EventViewer : public TGMainFrame {
      public:
        EventViewer(const TGWindow *parent, const char *filename);
        ~EventViewer() override;

        void CloseWindow() override;

        void OpenFileDialog();
        void LoadEvent();
        void NextEvent();
        void PreviousEvent();
        void RedrawEvent();
        void ToggleGeometry();
        void ToggleTracks();
        void ToggleSteps();

      private:
        bool OpenFile(const std::string &filename);
        bool LoadMetadata();
        bool ImportGeometry();

        void BuildUi();
        void UpdateEventSummary();
        void UpdateObjectLists();
        void DrawGeometry();
        void DrawTrackLines();
        void DrawStepMarkers();
        void ClearEventPrimitives();
        void ApplyDefaultCamera();

        static double DisplayLength(double millimeter) { return 0.1 * millimeter; }
        static int TrackColor(const simobj::Track &track);
        static std::string FormatStep(const simobj::Step &step, int idx, int trackId);
        static std::string FormatTrack(const simobj::Track &track, int idx);

        std::string fFilename;
        std::string fGeometryPath;

        std::unique_ptr<TFile> fFile;
        TTree *fTree;
        TTree *fPersistentTree;

        bool fComplete;
        TClonesArray *fTracks;
        TClonesArray *fSteps;
        simobj::Primary *fPrimary;
        simobj::Metadata *fMetadata;

        Long64_t fCurrentEvent;
        Long64_t fEventCount;

        TGNumberEntry *fEventEntry;
        TGLabel *fFileLabel;
        TGLabel *fSummaryLabel;
        TGCheckButton *fShowGeometry;
        TGCheckButton *fShowTracks;
        TGCheckButton *fShowSteps;
        TGTextView *fMetadataText;
        TGTextView *fTrackText;
        TGTextView *fStepText;
        TCanvas *fCanvas;
        TGLEmbeddedViewer *fGLViewer;
        bool fCameraInitialized;

        std::vector<TObject *> fEventPrimitives;
        std::unordered_map<size_t, int> fStepToTrackId;

        ClassDefOverride(EventViewer, 0)
    };
} // namespace eventviewer

#endif
