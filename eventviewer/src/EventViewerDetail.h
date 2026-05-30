#ifndef EVENTVIEWER_EVENTVIEWERDETAIL_H
#define EVENTVIEWER_EVENTVIEWERDETAIL_H

#include "simobj/Step.h"

#include "G4IonTable.hh"

#include "TGLEventHandler.h"
#include "TGListTree.h"
#include "TGLViewer.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGTextView.h"

#include <cmath>
#include <string>

namespace eventviewer::detail {
    constexpr int kUnknownCharge = 99;

    inline int ColorFromChargeSign(int chargeSign) {
        if (chargeSign == 0) return kGreen;
        if (chargeSign > 0) return kRed;
        return kBlue;
    }

    inline int IonChargeSignFromEncoding(int pdgCode) {
        const int encoding = pdgCode < 0 ? -pdgCode : pdgCode;
        if (encoding < 1000000000) return kUnknownCharge;

        G4int z             = 0;
        G4int a             = 0;
        G4int level         = 0;
        G4double excitation = 0.0;
        G4IonTable::GetNucleusByEncoding(encoding, z, a, excitation, level);

        // Geant4 nuclear codes are +-10LZZZAAAI; ZZZ is the ion charge number.
        if (z <= 0) z = (encoding / 10000) % 1000;
        if (z <= 0) return 0;
        return pdgCode < 0 ? -1 : 1;
    }

    inline int GeometryVisLevel(int verbosity) {
        switch (verbosity) {
            case 0:
                return 3;
            case 1:
                return 5;
            case 2:
                return 8;
            default:
                return 99;
        }
    }

    inline int GeometryMaxVisNodes(int verbosity) {
        switch (verbosity) {
            case 0:
                return 10000;
            case 1:
                return 50000;
            case 2:
                return 200000;
            default:
                return 1000000;
        }
    }

    inline int GeometrySegments(int verbosity) {
        switch (verbosity) {
            case 0:
                return 20;
            case 1:
                return 40;
            case 2:
                return 80;
            default:
                return 120;
        }
    }

    inline double StepDistance(const simobj::Step &a, const simobj::Step &b) {
        const double dx = a.GetX() - b.GetX();
        const double dy = a.GetY() - b.GetY();
        const double dz = a.GetZ() - b.GetZ();
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline double DisplayLength(double millimeter) { return 0.1 * millimeter; }

    inline std::string CompactPath(const std::string &path) {
        constexpr size_t maxLen = 86;
        if (path.size() <= maxLen) return path;
        return path.substr(0, 34) + "..." + path.substr(path.size() - 49);
    }

    inline void LoadText(TGTextView *view, const std::string &text) {
        view->Clear();
        view->AddLine(text.c_str());
        view->Update();
    }

    inline TGNumberEntry *AddVectorEntry(TGCompositeFrame *parent, const char *label,
                                         double value) {
        auto *row = new TGHorizontalFrame(parent);
        parent->AddFrame(row, new TGLayoutHints(kLHintsExpandX, 0, 0, 2, 2));
        row->AddFrame(new TGLabel(row, label),
                      new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 6, 0, 0));
        auto *entry = new TGNumberEntry(row, value, 8, -1, TGNumberFormat::kNESRealThree,
                                        TGNumberFormat::kNEAAnyNumber);
        row->AddFrame(entry, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY));
        return entry;
    }

    class GeometryListTree : public TGListTree {
      public:
        GeometryListTree(TGCanvas *parent, UInt_t options) : TGListTree(parent, options) {}

        Bool_t HandleButton(Event_t *event) override {
            if (event->fType == kButtonPress) {
                fLastButtonPressWasShift = (event->fState & kKeyShiftMask) != 0;
            }
            return TGListTree::HandleButton(event);
        }

        bool LastButtonPressWasShift() const { return fLastButtonPressWasShift; }

      private:
        bool fLastButtonPressWasShift = false;
    };

    class ShiftTruckEventHandler : public TGLEventHandler {
      public:
        ShiftTruckEventHandler(TGWindow *window, TObject *object)
            : TGLEventHandler(window, object), fShiftTruck(false), fOwnLeftDrag(false), fLastX(0),
              fLastY(0) {}

        Bool_t HandleButton(Event_t *event) override {
            if (event->fCode == kButton1) {
                if (event->fType == kButtonRelease && fOwnLeftDrag) {
                    EndShiftTruck(*event);
                    fOwnLeftDrag = false;
                    return kTRUE;
                }
                if (event->fType == kButtonPress && (event->fState & kKeyShiftMask)) {
                    BeginShiftTruck(*event);
                    return kTRUE;
                }
                if (event->fType == kButtonRelease && (event->fState & kKeyShiftMask)) {
                    EndShiftTruck(*event);
                    return kTRUE;
                }
            }
            if (event->fType == kButtonRelease) EndShiftTruck(*event);
            return TGLEventHandler::HandleButton(event);
        }

        Bool_t HandleMotion(Event_t *event) override {
            if ((event->fState & kKeyShiftMask) && (event->fState & kButton1Mask) && !fShiftTruck) {
                BeginShiftTruck(*event);
                return kTRUE;
            }

            if (fShiftTruck && fGLViewer) {
                if (!(event->fState & kKeyShiftMask)) {
                    EndShiftTruck(*event);
                    return kTRUE;
                }

                const int dx = event->fX - fLastX;
                const int dy = event->fY - fLastY;
                SyncMouseState(*event);
                if (dx != 0 || dy != 0) {
                    fGLViewer->SetResetCamerasOnUpdate(false);
                    fGLViewer->CurrentCamera().Truck(dx, -dy, kFALSE, kFALSE);
                    fGLViewer->RequestDraw();
                }
                return kTRUE;
            }
            return TGLEventHandler::HandleMotion(event);
        }

      private:
        void BeginShiftTruck(const Event_t &event) {
            fShiftTruck  = true;
            fOwnLeftDrag = true;
            StopMouseTimer();
            SyncMouseState(event);
        }

        void EndShiftTruck(const Event_t &event) {
            if (!fShiftTruck) return;
            fShiftTruck = false;
            SyncMouseState(event);
        }

        void SyncMouseState(const Event_t &event) {
            fLastX            = event.fX;
            fLastY            = event.fY;
            fLastPos.fX       = event.fX;
            fLastPos.fY       = event.fY;
            fLastGlobalPos.fX = event.fXRoot;
            fLastGlobalPos.fY = event.fYRoot;
            fLastEventState   = event.fState;
        }

        bool fShiftTruck;
        bool fOwnLeftDrag;
        int fLastX;
        int fLastY;
    };
} // namespace eventviewer::detail

#endif
