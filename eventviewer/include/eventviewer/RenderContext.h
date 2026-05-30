#ifndef __eventviewer_RenderContext_h__
#define __eventviewer_RenderContext_h__

#include <vector>

class TCanvas;
class TGLEmbeddedViewer;
class TGLEventHandler;
class TObject;

namespace eventviewer {
    class EventData;

    class RenderContext {
      public:
        void Destroy();
        void ClearEventPrimitives();
        void Redraw(const EventData &event, bool showGeometry, bool showTracks, bool showSteps,
                    int graphicalVerbosity);
        void ApplyDefaultCamera();

        TCanvas *canvas             = nullptr;
        TGLEmbeddedViewer *glViewer = nullptr;
        TGLEventHandler *glHandler  = nullptr;
        bool cameraInitialized      = false;
        std::vector<TObject *> eventPrimitives;

      private:
        void DrawGeometry(int graphicalVerbosity);
        void DrawTrackLines(const EventData &event, bool geometryVisible, int graphicalVerbosity);
        void DrawStepMarkers(const EventData &event, bool geometryVisible, bool tracksVisible);
    };
} // namespace eventviewer

#endif
