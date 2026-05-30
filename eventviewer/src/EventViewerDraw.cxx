#include "eventviewer/EventViewer.h"

#include "EventViewerDetail.h"

#include "simobj/Step.h"
#include "simobj/Track.h"

#include "TCanvas.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TGLEmbeddedViewer.h"
#include "TGLViewer.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TParticlePDG.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"

#include <algorithm>
#include <array>
#include <map>
#include <vector>

namespace eventviewer {
    using namespace detail;
    namespace {
        int TrackColor(const simobj::Track &track);
    } // namespace

    void EventViewer::RedrawEvent() {
        fRender.Redraw(fEvent, fUi.showGeometry->IsOn(), fUi.showTracks->IsOn(),
                       fUi.showSteps->IsOn(), fView.graphicalVerbosity);
    }

    void RenderContext::Redraw(const EventData &event, bool showGeometry, bool showTracks,
                               bool showSteps, int graphicalVerbosity) {
        if (!canvas) return;
        ClearEventPrimitives();
        canvas->cd();
        canvas->Clear();

        if (showGeometry) DrawGeometry(graphicalVerbosity);
        if (showTracks) DrawTrackLines(event, showGeometry, graphicalVerbosity);
        if (showSteps) DrawStepMarkers(event, showGeometry, showTracks);

        if (glViewer) {
            glViewer->SetResetCamerasOnUpdate(false);
            glViewer->PadPaint(canvas);
            ApplyDefaultCamera();
            glViewer->RequestDraw();
        }

        canvas->Modified();
        canvas->Update();
    }

    void RenderContext::DrawGeometry(int graphicalVerbosity) {
        if (!gGeoManager || !gGeoManager->GetTopVolume()) return;
        gGeoManager->SetVisLevel(GeometryVisLevel(graphicalVerbosity));
        gGeoManager->SetMaxVisNodes(GeometryMaxVisNodes(graphicalVerbosity));
        gGeoManager->SetNsegments(GeometrySegments(graphicalVerbosity));
        gGeoManager->SetVisOption(graphicalVerbosity >= 3 ? 1 : 0);
        gGeoManager->GetTopVolume()->Draw();
    }

    void RenderContext::DrawTrackLines(const EventData &event, bool geometryVisible,
                                       int graphicalVerbosity) {
        if (!event.tracks) return;
        const int nTracks   = event.tracks->GetEntriesFast();
        bool drawnSomething = geometryVisible;
        std::map<int, std::vector<std::array<double, 3>>> pointMarkers;
        for (int i = 0; i < nTracks; ++i) {
            auto *track = static_cast<simobj::Track *>(event.tracks->At(i));
            if (!track) continue;

            const int color = TrackColor(*track);
            std::vector<const simobj::Step *> points;
            points.push_back(&track->GetFirstStep());
            if (event.steps) {
                for (size_t j = 0; j < track->GetNStep(); ++j) {
                    const size_t stepIndex = track->GetStepIndex(j);
                    auto *step = static_cast<simobj::Step *>(event.steps->At(stepIndex));
                    if (step) points.push_back(step);
                }
            }
            points.push_back(&track->GetFinalStep());
            if (points.size() < 2) continue;

            double pathLength = 0.0;
            for (size_t p = 1; p < points.size(); ++p) {
                pathLength += StepDistance(*points[p - 1], *points[p]);
            }

            auto *line = new TPolyLine3D(points.size());
            for (int p = 0; p < static_cast<int>(points.size()); ++p) {
                line->SetPoint(p, DisplayLength(points[p]->GetX()),
                               DisplayLength(points[p]->GetY()), DisplayLength(points[p]->GetZ()));
            }
            line->SetLineColor(color);
            line->SetLineWidth(track->GetParentID() == 0 ? 3
                                                         : std::max(1, graphicalVerbosity));
            line->Draw(drawnSomething ? "same" : "");
            drawnSomething = true;
            eventPrimitives.push_back(line);

            if (graphicalVerbosity >= 2 && (graphicalVerbosity >= 3 || pathLength < 0.001)) {
                auto &markers = pointMarkers[color];
                for (const auto *point : points) {
                    markers.push_back({DisplayLength(point->GetX()), DisplayLength(point->GetY()),
                                       DisplayLength(point->GetZ())});
                }
            }
        }

        for (const auto &entry : pointMarkers) {
            auto *markers = new TPolyMarker3D(entry.second.size());
            for (int i = 0; i < static_cast<int>(entry.second.size()); ++i) {
                markers->SetPoint(i, entry.second[i][0], entry.second[i][1], entry.second[i][2]);
            }
            markers->SetMarkerStyle(20);
            markers->SetMarkerSize(graphicalVerbosity >= 3 ? 0.70 : 0.45);
            markers->SetMarkerColor(entry.first);
            markers->Draw(drawnSomething ? "same" : "");
            drawnSomething = true;
            eventPrimitives.push_back(markers);
        }
    }

    void RenderContext::DrawStepMarkers(const EventData &event, bool geometryVisible,
                                        bool tracksVisible) {
        if (!event.steps) return;
        auto *markers = new TPolyMarker3D(event.steps->GetEntriesFast());
        for (int i = 0; i < event.steps->GetEntriesFast(); ++i) {
            const auto *step = static_cast<simobj::Step *>(event.steps->At(i));
            if (!step) continue;
            markers->SetPoint(i, DisplayLength(step->GetX()), DisplayLength(step->GetY()),
                              DisplayLength(step->GetZ()));
        }
        markers->SetMarkerStyle(6);
        markers->SetMarkerColor(kOrange + 7);
        markers->Draw((geometryVisible || tracksVisible) ? "same" : "");
        eventPrimitives.push_back(markers);
    }

    void RenderContext::ClearEventPrimitives() {
        for (auto *object : eventPrimitives)
            delete object;
        eventPrimitives.clear();
    }

    void RenderContext::ApplyDefaultCamera() {
        if (!glViewer || cameraInitialized) return;
        glViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
        glViewer->ResetCurrentCamera();
        cameraInitialized = true;
    }

    namespace {
        int TrackColor(const simobj::Track &track) {
            const int pdgCode = track.GetPDGCode();

            const int ionChargeSign = IonChargeSignFromEncoding(pdgCode);
            if (ionChargeSign != kUnknownCharge) return ColorFromChargeSign(ionChargeSign);

            const auto *particle = TDatabasePDG::Instance()->GetParticle(pdgCode);
            if (particle) {
                const double charge = particle->Charge();
                return ColorFromChargeSign(charge == 0.0 ? 0 : (charge > 0.0 ? 1 : -1));
            }

            return track.GetParentID() == 0 ? kMagenta + 2 : kGray + 2;
        }
    } // namespace

    void RenderContext::Destroy() {
        ClearEventPrimitives();
        delete glHandler;
        glHandler = nullptr;
        delete canvas;
        canvas = nullptr;
        glViewer = nullptr;
    }
} // namespace eventviewer
