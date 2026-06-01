#include "eventviewer/EventViewer.h"

#include "EventViewerDetail.h"

#include "TGClient.h"
#include "TGListTree.h"
#include "TGMenu.h"
#include "TGeoAtt.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"

#include <sstream>
#include <string>

namespace eventviewer {
    namespace {
        enum GeometryContextCommand {
            kHideNode = 1,
            kHideNodeRecursive,
            kShowNode,
            kShowNodeRecursive
        };

        std::string GeometryNodeLabel(const TGeoNode &node) {
            std::ostringstream os;
            os << node.GetName();

            const auto *volume = node.GetVolume();
            if (volume) {
                os << "  [" << volume->GetName();
                if (const auto *material = volume->GetMaterial()) {
                    os << ", " << material->GetName();
                }
                os << ']';
            }
            os << "  copy=" << node.GetNumber();
            return os.str();
        }

        bool IsPathChecked(const ViewSettings &view, const std::string &path) {
            return view.hiddenGeometryPaths.find(path) == view.hiddenGeometryPaths.end();
        }

        GeometryAttributeState CaptureGeometryAttributes(const TGeoAtt &attributes) {
            GeometryAttributeState state;
            state.visOverride   = attributes.TestAttBit(TGeoAtt::kVisOverride);
            state.visNone       = attributes.TestAttBit(TGeoAtt::kVisNone);
            state.visThis       = attributes.TestAttBit(TGeoAtt::kVisThis);
            state.visDaughters  = attributes.TestAttBit(TGeoAtt::kVisDaughters);
            state.visOneLevel   = attributes.TestAttBit(TGeoAtt::kVisOneLevel);
            state.visStreamed   = attributes.TestAttBit(TGeoAtt::kVisStreamed);
            state.visTouched    = attributes.TestAttBit(TGeoAtt::kVisTouched);
            state.visContainers = attributes.TestAttBit(TGeoAtt::kVisContainers);
            state.visOnly       = attributes.TestAttBit(TGeoAtt::kVisOnly);
            state.visBranch     = attributes.TestAttBit(TGeoAtt::kVisBranch);
            state.visRaytrace   = attributes.TestAttBit(TGeoAtt::kVisRaytrace);
            return state;
        }

        GeometryVolumeDrawState CaptureVolumeDrawState(const TGeoVolume &volume) {
            GeometryVolumeDrawState state;
            state.transparency = volume.GetTransparency();
            state.lineColor    = volume.GetLineColor();
            state.lineStyle    = volume.GetLineStyle();
            state.lineWidth    = volume.GetLineWidth();
            state.fillColor    = volume.GetFillColor();
            state.fillStyle    = volume.GetFillStyle();
            return state;
        }

        void SnapshotGeometryAttributes(TGeoNode &node, ViewSettings &view) {
            view.geometryNodeAttributeDefaults[&node] = CaptureGeometryAttributes(node);
            view.geometryNodeVolumeDefaults[&node]    = node.GetVolume();
            if (auto *volume = node.GetVolume()) {
                view.geometryVolumeAttributeDefaults.try_emplace(
                    volume, CaptureGeometryAttributes(*volume));
                view.geometryVolumeDrawDefaults.try_emplace(volume, CaptureVolumeDrawState(*volume));
                if (auto *material = volume->GetMaterial()) {
                    view.geometryMaterialTransparencyDefaults.try_emplace(
                        material, material->GetTransparency());
                }
            }

            const int nDaughters = node.GetNdaughters();
            for (int i = 0; i < nDaughters; ++i) {
                auto *daughter = node.GetDaughter(i);
                if (daughter) SnapshotGeometryAttributes(*daughter, view);
            }
        }

        GeometryTreeEntry *MakeGeometryTreeEntry(ViewSettings &view, TGeoNode &node,
                                                 std::string path) {
            auto entry = std::make_unique<GeometryTreeEntry>();
            entry->node = &node;
            entry->path = std::move(path);
            auto *result = entry.get();
            view.geometryTreeEntries.push_back(std::move(entry));
            return result;
        }

        void AddGeometryNode(TGListTree &tree, TGListTreeItem *parent, TGeoNode &node,
                             ViewSettings &view, const std::string &path, int depth) {
            const std::string label = GeometryNodeLabel(node);
            auto *entry             = MakeGeometryTreeEntry(view, node, path);
            auto *item              = tree.AddItem(parent, label.c_str(), entry, nullptr, nullptr,
                                                   kTRUE);
            tree.CheckItem(item, IsPathChecked(view, path));
            if (depth < 2) tree.OpenItem(item);

            const int nDaughters = node.GetNdaughters();
            for (int i = 0; i < nDaughters; ++i) {
                auto *daughter = node.GetDaughter(i);
                if (!daughter) continue;
                AddGeometryNode(tree, item, *daughter, view, path + "/" + daughter->GetName(),
                                depth + 1);
            }
        }

        void ClearGeometryTree(TGListTree &tree) {
            while (auto *item = tree.GetFirstItem())
                tree.DeleteItem(item);
        }

        void SetPathHidden(ViewSettings &view, const std::string &path, Bool_t hidden) {
            if (hidden) {
                view.hiddenGeometryPaths.insert(path);
            } else {
                view.hiddenGeometryPaths.erase(path);
            }
        }

        void SetItemHiddenRecursive(TGListTreeItem &item, ViewSettings &view, Bool_t hidden) {
            if (auto *entry = static_cast<GeometryTreeEntry *>(item.GetUserData())) {
                SetPathHidden(view, entry->path, hidden);
                item.CheckItem(!hidden);
            }

            for (auto *child = item.GetFirstChild(); child; child = child->GetNextSibling())
                SetItemHiddenRecursive(*child, view, hidden);
        }
    } // namespace

    void EventViewer::UpdateGeometryTree() {
        if (!fUi.geometryTree) return;

        fView.updatingGeometryTree = true;
        fView.geometryNodeAttributeDefaults.clear();
        fView.geometryNodeVolumeDefaults.clear();
        fView.geometryVolumeAttributeDefaults.clear();
        fView.geometryVolumeDrawDefaults.clear();
        fView.geometryMaterialTransparencyDefaults.clear();
        fView.hiddenGeometryPaths.clear();
        fView.hiddenGeometryProxySerial = 0;
        fView.hiddenGeometryVolumeClones.clear();
        fView.geometryTreeEntries.clear();
        fView.geometryContextEntry = nullptr;
        ClearGeometryTree(*fUi.geometryTree);

        if (!gGeoManager || !gGeoManager->GetTopNode()) {
            fUi.geometryTree->AddItem(nullptr, "No geometry loaded");
        } else {
            auto *top = gGeoManager->GetTopNode();
            SnapshotGeometryAttributes(*top, fView);
            AddGeometryNode(*fUi.geometryTree, nullptr, *top, fView,
                            std::string("/") + top->GetName(), 0);
        }
        fView.updatingGeometryTree = false;

        fUi.geometryTree->MapSubwindows();
        fUi.geometryTree->AdjustPosition();
        if (gClient) gClient->NeedRedraw(fUi.geometryTree, kTRUE);
    }

    void EventViewer::SetGeometryNodeVisibility(TObject *object, Bool_t visible) {
        if (fView.updatingGeometryTree) return;

        auto *entry = dynamic_cast<GeometryTreeEntry *>(object);
        if (!entry) return;

        const auto *geometryTree = dynamic_cast<detail::GeometryListTree *>(fUi.geometryTree);
        const bool recursive     = geometryTree && geometryTree->LastButtonPressWasShift();

        if (recursive) {
            if (auto *item = fUi.geometryTree->FindItemByObj(fUi.geometryTree->GetFirstItem(), entry))
                SetItemHiddenRecursive(*item, fView, !visible);
            if (gClient) gClient->NeedRedraw(fUi.geometryTree, kTRUE);
        } else {
            SetPathHidden(fView, entry->path, !visible);
        }
        RedrawEvent();
    }

    void EventViewer::ShowGeometryContextMenu(TGListTreeItem *item, Int_t button, Int_t, Int_t) {
        if (!item) return;

        if (button != kButton3 || !fUi.geometryContextMenu) return;

        fView.geometryContextEntry = static_cast<GeometryTreeEntry *>(item->GetUserData());
        if (!fView.geometryContextEntry) return;

        const auto *geometryTree = dynamic_cast<detail::GeometryListTree *>(fUi.geometryTree);
        const int x              = geometryTree ? geometryTree->LastRootX() : 0;
        const int y              = geometryTree ? geometryTree->LastRootY() : 0;
        fUi.geometryContextMenu->PlaceMenu(x, y, kTRUE, kTRUE);
    }

    void EventViewer::HandleGeometryContextMenu(Int_t command) {
        auto *entry = fView.geometryContextEntry;
        if (!entry || !fUi.geometryTree) return;

        const bool hide = command == kHideNode || command == kHideNodeRecursive;
        const bool recursive =
            command == kHideNodeRecursive || command == kShowNodeRecursive;

        auto *item = fUi.geometryTree->FindItemByObj(fUi.geometryTree->GetFirstItem(), entry);
        if (!item) return;

        if (recursive) {
            SetItemHiddenRecursive(*item, fView, hide);
        } else {
            SetPathHidden(fView, entry->path, hide);
            item->CheckItem(!hide);
        }

        if (gClient) gClient->NeedRedraw(fUi.geometryTree, kTRUE);
        RedrawEvent();
    }
} // namespace eventviewer
