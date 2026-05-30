#ifndef __eventviewer_ViewSettings_h__
#define __eventviewer_ViewSettings_h__

#include <unordered_map>
#include <unordered_set>

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

    class ViewSettings {
      public:
        int graphicalVerbosity = 1;
        bool updatingGeometryTree = false;
        std::unordered_map<const TGeoNode *, GeometryAttributeState> geometryNodeAttributeDefaults;
        std::unordered_map<const TGeoVolume *, GeometryAttributeState> geometryVolumeAttributeDefaults;
        std::unordered_map<const TGeoVolume *, GeometryVolumeDrawState> geometryVolumeDrawDefaults;
        std::unordered_map<const TGeoMaterial *, int> geometryMaterialTransparencyDefaults;
        std::unordered_set<const TGeoNode *> hiddenGeometryNodes;
    };
} // namespace eventviewer

#endif
