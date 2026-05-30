#ifndef __eventviewer_EventViewer_h__
#define __eventviewer_EventViewer_h__

#include "eventviewer/EventData.h"
#include "eventviewer/EventViewerWidgets.h"
#include "eventviewer/RenderContext.h"
#include "eventviewer/ViewSettings.h"

#include "TGFrame.h"

#include <string>

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
        void ApplyCameraFromUi();
        void ResetCameraControls();
        void ApplyBackgroundColor(Pixel_t color);
        void SetGraphicalVerbosity(Int_t level);

      private:
        bool OpenFile(const std::string &filename);

        void BuildUi();
        void UpdateEventSummary();
        void UpdateObjectLists();
        void ApplyInitialWindowSize();
        void FlushInitialDisplay();

        EventData fEvent;
        EventViewerWidgets fUi;
        RenderContext fRender;
        ViewSettings fView;

        ClassDefOverride(EventViewer, 0)
    };
} // namespace eventviewer

#endif
