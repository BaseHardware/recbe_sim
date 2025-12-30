#ifndef __kobetdsim_aDetectorConstruction_h__
#define __kobetdsim_aDetectorConstruction_h__

#include "G4VUserDetectorConstruction.hh"
#include "simcore/DetectorConstruction.h"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class G4UserLimits;

namespace kobetdsim {
    class DetectorMessenger;
    class DetectorConstruction : public simcore::DetectorConstruction {
      public:
        DetectorConstruction();
        ~DetectorConstruction() override;

      public:
        G4VPhysicalVolume *Construct() override;
        void ConstructSDandField() override;

        void SetBoratedPE(G4bool a) { fUseBoratedPE = a; }
        G4bool GetBoratedPE() const { return fUseBoratedPE; }

        void SetGeometryType(G4String s) { fGeometryType = s; }
        G4String GetGeometryType() const { return fGeometryType; }

        void SetRealisticBeampipe(G4bool a) { fRealisticBeampipe = a; }
        G4bool GetRealisticBeampipe() const { return fRealisticBeampipe; }

      private:
        G4VPhysicalVolume *DefineVolumes() override;

        DetectorMessenger *fMessenger;

        G4bool fUseBoratedPE;
        G4bool fRealisticBeampipe;

        G4String fGeometryType;
    };
} // namespace kobetdsim
#endif
