#ifndef __simcore_MetadataManager_h__
#define __simcore_MetadataManager_h__

#include <string>

namespace simobj {
    class Metadata;
}

namespace simcore {
    class MetadataManager {
      public:
        MetadataManager(const MetadataManager &)            = delete;
        MetadataManager &operator=(const MetadataManager &) = delete;
        MetadataManager(MetadataManager &&)                 = delete;
        MetadataManager &operator=(MetadataManager &&)      = delete;

        static MetadataManager &GetInstance() {
            static MetadataManager fgInstance;
            return fgInstance;
        }

        void SetSimulationName(const std::string &a) { fSimName = a; }
        std::string GetSimulationName() const { return fSimName; }

        void SetGeometryType(const std::string &a) { fGeomTypename = a; }
        std::string GetGeometryType() const { return fGeomTypename; }

        void SetNumberOfRequestedEvents(int a) { fNRequestedEvents = a; }
        int GetNumberOfRequestedEvents() const { return fNRequestedEvents; }

        void SetNumberOfProcessedEvents(int a) { fNEvents = a; }
        int GetNumberOfProcessedEvents() const { return fNEvents; }

        void AutoSetRandomSeed(bool a) { fAutoRandomSeed = a; }
        bool AutoSetRandomSeed() const { return fAutoRandomSeed; }

        void SetRandomSeed(long a, bool aux);
        long GetRandomSeed(bool aux) const;

        void FillMetadata(simobj::Metadata *target) const;

      private:
        MetadataManager() : fAutoRandomSeed(false), fSimName(""), fGeomTypename("") {};
        virtual ~MetadataManager() {};

        long fRandomSeeds[2];
        bool fAutoRandomSeed;

        int fNRequestedEvents;
        int fNEvents;

        std::string fSimName;
        std::string fGeomTypename;
    };
} // namespace simcore

#endif
