#include "eventviewer/EventViewer.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TGLEventHandler.h"

#include <cstdio>

ClassImp(eventviewer::EventViewer)

    namespace eventviewer {
    EventViewer::EventViewer(const TGWindow *parent, const char *filename)
        : TGMainFrame(parent, 1600, 900) {
        BuildUi();
        MapSubwindows();
        ApplyInitialWindowSize();
        MapWindow();
        if (OpenFile(filename)) {
            LoadEvent();
            FlushInitialDisplay();
        }
    }

    EventViewer::~EventViewer() {
        fRender.Destroy();
        fEvent.CleanupGeometryFile();
        Cleanup();
    }

    void EventViewer::CloseWindow() { DeleteWindow(); }
} // namespace eventviewer
