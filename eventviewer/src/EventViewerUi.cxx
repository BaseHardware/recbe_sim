#include "eventviewer/EventViewer.h"

#include "EventViewerDetail.h"

#include "TCanvas.h"
#include "TColor.h"
#include "TGButton.h"
#include "TGCanvas.h"
#include "TGClient.h"
#include "TGColorSelect.h"
#include "TGComboBox.h"
#include "TGLCamera.h"
#include "TGLEmbeddedViewer.h"
#include "TGLUtil.h"
#include "TGLViewer.h"
#include "TGLWidget.h"
#include "TGLabel.h"
#include "TGListTree.h"
#include "TGMenu.h"
#include "TGNumberEntry.h"
#include "TGTab.h"
#include "TGTextView.h"
#include "TROOT.h"
#include "TSystem.h"

#include <algorithm>

namespace eventviewer {
    using namespace detail;
    void EventViewer::BuildUi() {
        SetWindowName("Geant4 Event Viewer");

        auto *top = new TGVerticalFrame(this);
        AddFrame(top, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *toolbar = new TGHorizontalFrame(top, 10, 10);
        top->AddFrame(toolbar, new TGLayoutHints(kLHintsExpandX, 6, 6, 6, 2));

        auto *openButton = new TGTextButton(toolbar, "Open");
        openButton->Connect("Clicked()", "eventviewer::EventViewer", this, "OpenFileDialog()");
        toolbar->AddFrame(openButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 6, 0, 0));

        auto *prevButton = new TGTextButton(toolbar, "Prev");
        prevButton->Connect("Clicked()", "eventviewer::EventViewer", this, "PreviousEvent()");
        toolbar->AddFrame(prevButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 4, 0, 0));

        auto *nextButton = new TGTextButton(toolbar, "Next");
        nextButton->Connect("Clicked()", "eventviewer::EventViewer", this, "NextEvent()");
        toolbar->AddFrame(nextButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 8, 0, 0));

        toolbar->AddFrame(new TGLabel(toolbar, "Event"),
                          new TGLayoutHints(kLHintsCenterY, 0, 4, 0, 0));
        fUi.eventEntry = new TGNumberEntry(toolbar, 0, 7, -1, TGNumberFormat::kNESInteger,
                                           TGNumberFormat::kNEANonNegative,
                                           TGNumberFormat::kNELLimitMinMax, 0, 0);
        fUi.eventEntry->Connect("ValueSet(Long_t)", "eventviewer::EventViewer", this,
                                "LoadEvent()");
        toolbar->AddFrame(fUi.eventEntry, new TGLayoutHints(kLHintsCenterY, 0, 10, 0, 0));

        fUi.showGeometry = new TGCheckButton(toolbar, "Geometry");
        fUi.showGeometry->SetState(kButtonDown);
        fUi.showGeometry->Connect("Clicked()", "eventviewer::EventViewer", this,
                                  "ToggleGeometry()");
        toolbar->AddFrame(fUi.showGeometry, new TGLayoutHints(kLHintsCenterY, 0, 6, 0, 0));

        fUi.showTracks = new TGCheckButton(toolbar, "Tracks");
        fUi.showTracks->SetState(kButtonDown);
        fUi.showTracks->Connect("Clicked()", "eventviewer::EventViewer", this, "ToggleTracks()");
        toolbar->AddFrame(fUi.showTracks, new TGLayoutHints(kLHintsCenterY, 0, 6, 0, 0));

        fUi.showSteps = new TGCheckButton(toolbar, "Steps");
        fUi.showSteps->SetState(kButtonUp);
        fUi.showSteps->Connect("Clicked()", "eventviewer::EventViewer", this, "ToggleSteps()");
        toolbar->AddFrame(fUi.showSteps, new TGLayoutHints(kLHintsCenterY, 0, 10, 0, 0));

        fUi.fileLabel = new TGLabel(toolbar, "");
        toolbar->AddFrame(fUi.fileLabel, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY));

        auto *main = new TGHorizontalFrame(top);
        top->AddFrame(main, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 6, 6, 0, 6));

        auto *left = new TGVerticalFrame(main, 280, 920);
        main->AddFrame(left, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 0, 8, 0, 0));

        fUi.summaryLabel = new TGLabel(left, "No file loaded");
        left->AddFrame(fUi.summaryLabel, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 6));

        auto *tabs = new TGTab(left, 280, 880);
        left->AddFrame(tabs, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *trackTab = tabs->AddTab("Tracks");
        fUi.trackText  = new TGTextView(trackTab, 260, 820);
        trackTab->AddFrame(fUi.trackText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *stepTab = tabs->AddTab("Steps");
        fUi.stepText  = new TGTextView(stepTab, 260, 820);
        stepTab->AddFrame(fUi.stepText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *metaTab    = tabs->AddTab("Metadata");
        fUi.metadataText = new TGTextView(metaTab, 260, 820);
        metaTab->AddFrame(fUi.metadataText, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        auto *geometryTab    = tabs->AddTab("Geometry");
        auto *geometryCanvas = new TGCanvas(geometryTab, 260, 820);
        fUi.geometryTree     = new GeometryListTree(geometryCanvas, kHorizontalFrame);
        fUi.geometryTree->SetCheckMode(TGListTree::kSimple);
        fUi.geometryTree->Connect("Checked(TObject*,Bool_t)", "eventviewer::EventViewer", this,
                                  "SetGeometryNodeVisibility(TObject*,Bool_t)");
        fUi.geometryTree->Connect("Clicked(TGListTreeItem*,Int_t,Int_t,Int_t)",
                                  "eventviewer::EventViewer", this,
                                  "ShowGeometryContextMenu(TGListTreeItem*,Int_t,Int_t,Int_t)");
        fUi.geometryContextMenu = new TGPopupMenu(gClient->GetRoot());
        fUi.geometryContextMenu->AddEntry("Hide this node", 1);
        fUi.geometryContextMenu->AddEntry("Hide node and children", 2);
        fUi.geometryContextMenu->AddSeparator();
        fUi.geometryContextMenu->AddEntry("Show this node", 3);
        fUi.geometryContextMenu->AddEntry("Show node and children", 4);
        fUi.geometryContextMenu->Connect("Activated(Int_t)", "eventviewer::EventViewer", this,
                                         "HandleGeometryContextMenu(Int_t)");
        geometryTab->AddFrame(geometryCanvas,
                              new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        fUi.geometryTree->AddItem(nullptr, "No geometry loaded");

        auto *viewTab = tabs->AddTab("View");
        viewTab->AddFrame(new TGLabel(viewTab, "Graphical verbosity"),
                          new TGLayoutHints(kLHintsLeft, 0, 0, 6, 2));
        fUi.graphicalVerbosityBox = new TGComboBox(viewTab);
        fUi.graphicalVerbosityBox->AddEntry("Overview", 0);
        fUi.graphicalVerbosityBox->AddEntry("Normal", 1);
        fUi.graphicalVerbosityBox->AddEntry("Detailed", 2);
        fUi.graphicalVerbosityBox->AddEntry("Full detail", 3);
        fUi.graphicalVerbosityBox->Select(fView.graphicalVerbosity, kFALSE);
        fUi.graphicalVerbosityBox->Connect("Selected(Int_t)", "eventviewer::EventViewer", this,
                                           "SetGraphicalVerbosity(Int_t)");
        viewTab->AddFrame(fUi.graphicalVerbosityBox,
                          new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 10));

        viewTab->AddFrame(new TGLabel(viewTab, "View vector"),
                          new TGLayoutHints(kLHintsLeft, 0, 0, 6, 2));
        fUi.viewXEntry = AddVectorEntry(viewTab, "X", -1.0);
        fUi.viewYEntry = AddVectorEntry(viewTab, "Y", 0.0);
        fUi.viewZEntry = AddVectorEntry(viewTab, "Z", 0.0);
        viewTab->AddFrame(new TGLabel(viewTab, "Up vector"),
                          new TGLayoutHints(kLHintsLeft, 0, 0, 10, 2));
        fUi.upXEntry = AddVectorEntry(viewTab, "X", 0.0);
        fUi.upYEntry = AddVectorEntry(viewTab, "Y", 1.0);
        fUi.upZEntry = AddVectorEntry(viewTab, "Z", 0.0);

        auto *backgroundRow = new TGHorizontalFrame(viewTab);
        viewTab->AddFrame(backgroundRow, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
        backgroundRow->AddFrame(new TGLabel(backgroundRow, "Background"),
                                new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 8, 0, 0));
        Pixel_t blackPixel = 0;
        gClient->GetColorByName("black", blackPixel);
        fUi.backgroundColorSelect = new TGColorSelect(backgroundRow, blackPixel, -1);
        fUi.backgroundColorSelect->Connect("ColorSelected(Pixel_t)", "eventviewer::EventViewer",
                                           this, "ApplyBackgroundColor(Pixel_t)");
        backgroundRow->AddFrame(fUi.backgroundColorSelect,
                                new TGLayoutHints(kLHintsLeft | kLHintsCenterY));

        auto *viewButtons = new TGHorizontalFrame(viewTab);
        viewTab->AddFrame(viewButtons, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
        auto *applyViewButton = new TGTextButton(viewButtons, "Apply");
        applyViewButton->Connect("Clicked()", "eventviewer::EventViewer", this,
                                 "ApplyCameraFromUi()");
        viewButtons->AddFrame(applyViewButton, new TGLayoutHints(kLHintsExpandX, 0, 4, 0, 0));
        auto *resetViewButton = new TGTextButton(viewButtons, "Reset");
        resetViewButton->Connect("Clicked()", "eventviewer::EventViewer", this,
                                 "ResetCameraControls()");
        viewButtons->AddFrame(resetViewButton, new TGLayoutHints(kLHintsExpandX, 4, 0, 0, 0));

        const bool wasBatch = gROOT->IsBatch();
        gROOT->SetBatch(kTRUE);
        fRender.canvas = new TCanvas("event_backing_canvas", "event_backing_canvas", 1200, 760);
        gROOT->SetBatch(wasBatch);
        fRender.glViewer = new TGLEmbeddedViewer(main, fRender.canvas);
        fRender.glViewer->GetFrame()->Resize(1200, 760);
        main->AddFrame(fRender.glViewer->GetFrame(),
                       new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        fRender.glViewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);
        fRender.glViewer->SetResetCamerasOnUpdate(false);
        fRender.glViewer->SetClearColor(kBlack);
        fRender.glHandler = new ShiftTruckEventHandler(nullptr, fRender.glViewer);
        fRender.glViewer->GetGLWidget()->SetEventHandler(fRender.glHandler);
    }

    void EventViewer::ApplyInitialWindowSize() {
        constexpr UInt_t targetWidth  = 1600;
        constexpr UInt_t targetHeight = 900;
        constexpr UInt_t minWidth     = 1100;
        constexpr UInt_t minHeight    = 720;

        const UInt_t displayWidth  = gClient ? gClient->GetDisplayWidth() : targetWidth;
        const UInt_t displayHeight = gClient ? gClient->GetDisplayHeight() : targetHeight;
        const UInt_t margin        = 80;

        const UInt_t maxWidth  = displayWidth > margin ? displayWidth - margin : displayWidth;
        const UInt_t maxHeight = displayHeight > margin ? displayHeight - margin : displayHeight;
        const UInt_t width =
            std::max(std::min(targetWidth, maxWidth), std::min(minWidth, maxWidth));
        const UInt_t height =
            std::max(std::min(targetHeight, maxHeight), std::min(minHeight, maxHeight));

        Resize(width, height);
    }

    void EventViewer::FlushInitialDisplay() {
        Layout();
        MapSubwindows();
        if (gClient) {
            gClient->NeedRedraw(this, kTRUE);
            if (fRender.glViewer && fRender.glViewer->GetFrame()) {
                gClient->NeedRedraw(fRender.glViewer->GetFrame(), kTRUE);
            }
        }
        if (fRender.glViewer) fRender.glViewer->RequestDraw();
        if (gSystem) {
            for (int i = 0; i < 3; ++i)
                gSystem->ProcessEvents();
        }
    }
    void EventViewer::ToggleGeometry() { RedrawEvent(); }
    void EventViewer::ToggleTracks() { RedrawEvent(); }
    void EventViewer::ToggleSteps() { RedrawEvent(); }

    void EventViewer::SetGraphicalVerbosity(Int_t level) {
        fView.graphicalVerbosity = std::clamp(static_cast<int>(level), 0, 3);
        RedrawEvent();
    }

    void EventViewer::ApplyCameraFromUi() {
        if (!fRender.glViewer) return;

        const double vx = fUi.viewXEntry->GetNumber();
        const double vy = fUi.viewYEntry->GetNumber();
        const double vz = fUi.viewZEntry->GetNumber();
        const double ux = fUi.upXEntry->GetNumber();
        const double uy = fUi.upYEntry->GetNumber();
        const double uz = fUi.upZEntry->GetNumber();

        const double viewNorm2  = vx * vx + vy * vy + vz * vz;
        const double upNorm2    = ux * ux + uy * uy + uz * uz;
        const double cx         = vy * uz - vz * uy;
        const double cy         = vz * ux - vx * uz;
        const double cz         = vx * uy - vy * ux;
        const double crossNorm2 = cx * cx + cy * cy + cz * cz;

        if (viewNorm2 < 1e-12 || upNorm2 < 1e-12 || crossNorm2 / (viewNorm2 * upNorm2) < 1e-8) {
            fUi.summaryLabel->SetText("Camera vectors are invalid or nearly parallel.");
            return;
        }

        fRender.glViewer->ReinitializeCurrentCamera(TGLVector3(vx, vy, vz), TGLVector3(ux, uy, uz));
        fRender.cameraInitialized = true;
        UpdateEventSummary();
    }

    void EventViewer::ResetCameraControls() {
        fUi.viewXEntry->SetNumber(-1.0);
        fUi.viewYEntry->SetNumber(0.0);
        fUi.viewZEntry->SetNumber(0.0);
        fUi.upXEntry->SetNumber(0.0);
        fUi.upYEntry->SetNumber(1.0);
        fUi.upZEntry->SetNumber(0.0);
        fRender.cameraInitialized = false;
        ApplyCameraFromUi();
    }

    void EventViewer::ApplyBackgroundColor(Pixel_t color) {
        if (!fRender.glViewer) return;
        fRender.glViewer->SetClearColor(TColor::GetColor(color));
        fRender.glViewer->RequestDraw();
    }
} // namespace eventviewer
