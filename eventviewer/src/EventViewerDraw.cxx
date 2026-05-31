#include "eventviewer/EventViewer.h"

#include "EventViewerDetail.h"

#include "simobj/Step.h"
#include "simobj/Track.h"

#include "TCanvas.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TGeoAtt.h"
#include "TGLEmbeddedViewer.h"
#include "TGLViewer.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoNode.h"
#include "TGeoPhysicalNode.h"
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
        void RestoreGeometryAttributes(TGeoNode &node, const ViewSettings &view);
        void ApplyHiddenGeometryPaths(const ViewSettings &view);
        void HideGeometryNode(TGeoNode &node);
    } // namespace

    void EventViewer::RedrawEvent() {
        fRender.Redraw(fEvent, fView, fUi.showGeometry->IsOn(), fUi.showTracks->IsOn(),
                       fUi.showSteps->IsOn());
    }

    void RenderContext::Redraw(const EventData &event, const ViewSettings &view, bool showGeometry,
                               bool showTracks, bool showSteps) {
        if (!canvas) return;
        ClearEventPrimitives();
        canvas->cd();
        canvas->Clear();

        if (showGeometry) DrawGeometry(view);
        if (showTracks) DrawTrackLines(event, showGeometry, view.graphicalVerbosity);
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

    void RenderContext::DrawGeometry(const ViewSettings &view) {
        if (!gGeoManager || !gGeoManager->GetTopVolume()) return;
        if (auto *top = gGeoManager->GetTopNode()) {
            RestoreGeometryAttributes(*top, view);
            ApplyHiddenGeometryPaths(view);
        }
        gGeoManager->SetVisLevel(GeometryVisLevel(view.graphicalVerbosity));
        gGeoManager->SetMaxVisNodes(GeometryMaxVisNodes(view.graphicalVerbosity));
        gGeoManager->SetNsegments(GeometrySegments(view.graphicalVerbosity));
        gGeoManager->SetVisOption(view.graphicalVerbosity >= 3 ? 1 : 0);
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
        void ApplyGeometryAttributes(TGeoAtt &attributes, const GeometryAttributeState &state) {
            attributes.SetAttBit(TGeoAtt::kVisOverride, state.visOverride);
            attributes.SetAttBit(TGeoAtt::kVisNone, state.visNone);
            attributes.SetAttBit(TGeoAtt::kVisThis, state.visThis);
            attributes.SetAttBit(TGeoAtt::kVisDaughters, state.visDaughters);
            attributes.SetAttBit(TGeoAtt::kVisOneLevel, state.visOneLevel);
            attributes.SetAttBit(TGeoAtt::kVisStreamed, state.visStreamed);
            attributes.SetAttBit(TGeoAtt::kVisTouched, state.visTouched);
            attributes.SetAttBit(TGeoAtt::kVisContainers, state.visContainers);
            attributes.SetAttBit(TGeoAtt::kVisOnly, state.visOnly);
            attributes.SetAttBit(TGeoAtt::kVisBranch, state.visBranch);
            attributes.SetAttBit(TGeoAtt::kVisRaytrace, state.visRaytrace);
        }

        void ApplyVolumeDrawState(TGeoVolume &volume, const GeometryVolumeDrawState &state) {
            volume.SetTransparency(state.transparency);
            volume.SetLineColor(state.lineColor);
            volume.SetLineStyle(state.lineStyle);
            volume.SetLineWidth(state.lineWidth);
            volume.SetFillColor(state.fillColor);
            volume.SetFillStyle(state.fillStyle);
        }

        void RestoreGeometryAttributes(TGeoNode &node, const ViewSettings &view) {
            const auto nodeAttributes = view.geometryNodeAttributeDefaults.find(&node);
            if (nodeAttributes != view.geometryNodeAttributeDefaults.end()) {
                ApplyGeometryAttributes(node, nodeAttributes->second);
            }

            if (auto *volume = node.GetVolume()) {
                const auto volumeAttributes = view.geometryVolumeAttributeDefaults.find(volume);
                if (volumeAttributes != view.geometryVolumeAttributeDefaults.end()) {
                    ApplyGeometryAttributes(*volume, volumeAttributes->second);
                }
                const auto volumeDrawState = view.geometryVolumeDrawDefaults.find(volume);
                if (volumeDrawState != view.geometryVolumeDrawDefaults.end()) {
                    ApplyVolumeDrawState(*volume, volumeDrawState->second);
                }
                if (auto *material = volume->GetMaterial()) {
                    const auto materialTransparency =
                        view.geometryMaterialTransparencyDefaults.find(material);
                    if (materialTransparency != view.geometryMaterialTransparencyDefaults.end()) {
                        material->SetTransparency(materialTransparency->second);
                    }
                }
            }

            const int nDaughters = node.GetNdaughters();
            for (int i = 0; i < nDaughters; ++i) {
                auto *daughter = node.GetDaughter(i);
                if (daughter) RestoreGeometryAttributes(*daughter, view);
            }
        }

        void ApplyHiddenGeometryPaths(const ViewSettings &view) {
            if (gGeoManager) gGeoManager->ClearPhysicalNodes(kTRUE);
            for (const auto &path : view.hiddenGeometryPaths) {
                if (path.empty()) continue;
                auto *physicalNode = gGeoManager ? gGeoManager->MakePhysicalNode(path.c_str()) : nullptr;
                if (physicalNode) {
                    physicalNode->SetVisibleFull(kTRUE);
                    physicalNode->SetVisibility(kFALSE);
                    gGeoManager->SetVisibility(physicalNode, kFALSE);
                    continue;
                }

                if (gGeoManager && gGeoManager->cd(path.c_str())) {
                    if (auto *node = gGeoManager->GetCurrentNode()) HideGeometryNode(*node);
                }
            }
            if (gGeoManager) gGeoManager->RefreshPhysicalNodes(kFALSE);
        }

        void HideGeometryNode(TGeoNode &node) {
            node.SetAttBit(TGeoAtt::kVisNone, kFALSE);
            node.SetAttBit(TGeoAtt::kVisThis, kFALSE);
            node.SetAttBit(TGeoAtt::kVisDaughters, kTRUE);
            node.SetAttBit(TGeoAtt::kVisOneLevel, kFALSE);
            node.SetAttBit(TGeoAtt::kVisOnly, kFALSE);
            node.SetAttBit(TGeoAtt::kVisBranch, kFALSE);
        }

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
