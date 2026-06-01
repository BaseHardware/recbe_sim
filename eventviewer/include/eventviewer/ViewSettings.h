#ifndef __eventviewer_ViewSettings_h__
#define __eventviewer_ViewSettings_h__

#include "TObject.h"
#include "TGeoVolume.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class TGeoMaterial;
class TGeoNode;
class TGeoVolume;

namespace eventviewer {
    struct GeometryAttributeState {
        bool visOverride   = false;
        bool visNone       = false;
        bool visThis       = false;
        bool visDaughters  = false;
        bool visOneLevel   = false;
        bool visStreamed   = false;
        bool visTouched    = false;
        bool visContainers = false;
        bool visOnly       = false;
        bool visBranch     = false;
        bool visRaytrace   = false;
    };

    struct GeometryVolumeDrawState {
        int transparency = 0;
        int lineColor    = 0;
        int lineStyle    = 0;
        int lineWidth    = 0;
        int fillColor    = 0;
        int fillStyle    = 0;
    };

    struct GeometryTreeEntry : public TObject {
        TGeoNode *node = nullptr;
        std::string path;
    };

    class ViewSettings {
      public:
        int graphicalVerbosity = 1;
        bool updatingGeometryTree = false;
        std::unordered_map<const TGeoNode *, GeometryAttributeState> geometryNodeAttributeDefaults;
        std::unordered_map<const TGeoNode *, TGeoVolume *> geometryNodeVolumeDefaults;
        std::unordered_map<const TGeoVolume *, GeometryAttributeState> geometryVolumeAttributeDefaults;
        std::unordered_map<const TGeoVolume *, GeometryVolumeDrawState> geometryVolumeDrawDefaults;
        std::unordered_map<const TGeoMaterial *, int> geometryMaterialTransparencyDefaults;
        std::unordered_set<std::string> hiddenGeometryPaths;
        mutable std::size_t hiddenGeometryProxySerial = 0;
        mutable std::vector<TGeoVolume *> hiddenGeometryVolumeClones;
        std::vector<std::unique_ptr<GeometryTreeEntry>> geometryTreeEntries;
        GeometryTreeEntry *geometryContextEntry = nullptr;
    };
} // namespace eventviewer

#endif
