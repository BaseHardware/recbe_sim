#ifndef __bl10sim_BL10DetectorConstruction_h__
#define __bl10sim_BL10DetectorConstruction_h__

#include "simcore/DetectorConstruction.h"

#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4TwoVector.hh"
#include "G4VUserDetectorConstruction.hh"

#include <array>
#include <vector>

class G4VPhysicalVolume;
class G4GlobalMagFieldMessenger;
class G4VSolid;
class G4Material;

namespace bl10sim {
    class BL10DetectorConstruction : public simcore::DetectorConstruction {
        struct FrameBoardComplexInfo {
            G4String fBoardName;
            G4LogicalVolume *fBoardLV;
            G4Material *fEnvelopeMaterial;
            G4double fNegativeXVJigLength;
            G4double fPositiveXVJigLength;
            G4double fBoardDistFromTop;
            G4double fBoardHoriOffset;
            G4double fJigToBoardZSpace;
            G4double fJigToBoardPosXSpace;
            G4double fJigToBoardNegXSpace;
            G4int fBoardCopyNo;

            bool operator<(const FrameBoardComplexInfo &rhs) const {
                if (fBoardName != rhs.fBoardName)
                    return fBoardName < rhs.fBoardName;
                else if (fBoardLV != rhs.fBoardLV)
                    return fBoardLV < rhs.fBoardLV;
                else if (fEnvelopeMaterial != rhs.fEnvelopeMaterial)
                    return fEnvelopeMaterial < rhs.fEnvelopeMaterial;
                else if (fNegativeXVJigLength != rhs.fNegativeXVJigLength)
                    return fNegativeXVJigLength < rhs.fNegativeXVJigLength;
                else if (fPositiveXVJigLength < rhs.fPositiveXVJigLength)
                    return fPositiveXVJigLength < rhs.fPositiveXVJigLength;
                else if (fBoardDistFromTop < rhs.fBoardDistFromTop)
                    return fBoardDistFromTop < rhs.fBoardDistFromTop;
                else if (fBoardHoriOffset < rhs.fBoardHoriOffset)
                    return fBoardHoriOffset < rhs.fBoardHoriOffset;
                else if (fJigToBoardZSpace != rhs.fJigToBoardZSpace)
                    return fJigToBoardZSpace < rhs.fJigToBoardZSpace;
                else if (fJigToBoardPosXSpace != rhs.fJigToBoardPosXSpace)
                    return fJigToBoardPosXSpace < rhs.fJigToBoardPosXSpace;
                else if (fJigToBoardNegXSpace != rhs.fJigToBoardNegXSpace)
                    return fJigToBoardNegXSpace < rhs.fJigToBoardNegXSpace;
                else
                    return fBoardCopyNo < rhs.fBoardCopyNo;
            }

            G4String GetComplexName() const { return "Frame_" + fBoardName; }
        };

      public:
        BL10DetectorConstruction();
        virtual ~BL10DetectorConstruction() override {
            delete fmHoriJigRotMtx;
            delete fmVertJigRotMtx;
        };

      public:
        void ConstructSDandField() override;

      protected:
        virtual void DefineMaterials() override;

        virtual void SetGeometryParameters();
        virtual void CalculateGeometrySubparameters();

        void SetBL10RoomParameters();
        void SetBoardParameters();
        void SetJigFrameParameters();

        G4VSolid *BuildBoronResincaseSolid(G4bool) const;
        G4VSolid *BuildLabSolid(G4bool) const;

        G4LogicalVolume *BuildIroncase() const;
        G4LogicalVolume *FillIroncase(G4LogicalVolume *) const;

        G4LogicalVolume *BuildWorkbench() const;
        G4ThreeVector PlaceWorkbench(G4LogicalVolume *, G4LogicalVolume *) const;

        G4LogicalVolume *BuildJig(G4double, G4Material *, G4Material *) const;

        G4LogicalVolume *BuildMkII(G4Material *) const;
        G4LogicalVolume *BuildRECBE(G4Material *) const;
        G4LogicalVolume *BuildROESTI(G4Material *) const;

        G4LogicalVolume *BuildFrameBoardComplex(const FrameBoardComplexInfo &) const;

        G4LogicalVolume *BuildFrameBoards(G4Material *) const;

        void PlaceBeamWindow(G4LogicalVolume *labLV) const;

        void PlaceSimpleNeutronFluxDetectors(G4LogicalVolume *labLV) const;

        G4VPhysicalVolume *DefineVolumes() override;

      private:
        G4bool fSimpleGeometry;

        G4double ftLabWidthSlope;
        G4double ftWBEnvelopeWidth;
        G4double ftWBEnvelopeHeight;
        G4double ftWBEnvelopeZLength;
        std::vector<G4TwoVector> ftHBeamPoints;
        std::vector<G4TwoVector> ftLevelingBoltPoints;
        std::vector<G4TwoVector> ftJigSpacePoints;
        std::vector<G4TwoVector> ftMkIIBoardPoints;
        std::vector<G4TwoVector> ftRECBEBoardPoints;

        G4double fBeamXDistanceFromWall;
        G4double fBeamYDistanceFromFloor;

        G4double fBoronResinThickness;
        G4double fIronThickness;
        G4double fFloorThickness;

        G4double fLabHeight;
        G4double fLabZLength;
        G4double fLabWidthBeamside;
        G4double fLabWidthDumpside;
        G4double fLabFloorSpace;

        G4double fExitwallDistance;
        G4double fExitwallThickness;
        G4double fExitwallWidth;
        G4double fExitwallBRDepth;

        G4double fExitpathWidth;

        G4double fWorkbenchPlateWidth;
        G4double fWorkbenchPlateLength;
        G4double fWorkbenchPlateThickness;

        G4double fWorkbenchPlateOuterMargin;
        G4double fWorkbenchPlateInnerMargin;

        G4double fWorkbenchSupportWidth;
        G4double fWorkbenchSupportHeight;
        G4double fWorkbenchSupportThickness;
        G4double fWorkbenchXSupportLength;
        G4double fWorkbenchZSupportLength;

        G4double fWBPipeEndplateThickness;
        G4double fWBPipeEndplate1Width;
        G4double fWBPipeEndplate2Width;
        G4double fWBPipeEndplate3Width;
        G4double fWBPipeEndplate1Length;
        G4double fWBPipeEndplate2Length;
        G4double fWBPipeEndplate3Length;

        G4double fWBVertPipeLength;
        G4double fWBZDiagPipeLength;
        G4double fWBXDiagPipeLength;
        G4double fWBVertPipeRadius;
        G4double fWBZDiagPipeRadius;
        G4double fWBXDiagPipeRadius;
        G4double fWBVertPipeThickness;
        G4double fWBZDiagPipeThickness;
        G4double fWBXDiagPipeThickness;

        G4double fWBZDiagPipeAngle;
        G4double fWBXDiagPipeAngle;
        G4double fWBZDiagPipeYDistance;
        G4double fWBXDiagPipeYDistance;

        G4double fWBLevelingBoltSize;
        G4double fWBLevelingBoltThickness;

        G4double fWBZDistanceFromWall;

        G4double fSampleZPosFromWBCenter;

        G4double fBeamWindowWidth;
        G4double fBeamWindowHeight;

        G4double fWindowThickness;

        G4double fJigVHSize;
        G4double fVJigType1Length;
        G4double fVJigType2Length;

        G4double fJigCenterHoleRadius;

        G4double fJigSpaceStemBottomWidth;
        G4double fJigSpaceStemLength;
        G4double fJigSpaceMiddleWidth;
        G4double fJigSpaceMiddleLength;
        G4double fJigSpaceHeight;

        G4double fRECBEThickness;
        G4double fRECBETopWidth;
        G4double fRECBEMiddleWidth;
        G4double fRECBELongHeight;
        G4double fRECBEShortHeight;
        G4double fRECBEFPGAVSpace;
        G4double fRECBEFPGAHSpace;

        G4double fRECBEFPGASubstrateVHSize;
        G4double fRECBEFPGALidThickness;
        G4double fRECBEFPGASubstrateThickness;
        G4double fRECBEFPGADieThickness;
        G4double fRECBEFPGADieVHSize;

        G4double fRECBEBoardTopHeight;

        G4double fMkIIThickness;
        G4double fMkIITopWidth;
        G4double fMkIIMiddleWidth;
        G4double fMkIILongHeight;
        G4double fMkIIShortHeight;
        G4double fMkIIFPGAVSpace;
        G4double fMkIIFPGAHSpace;

        G4double fMkIIFPGASubstrateVHSize;
        G4double fMkIIFPGALidThickness;
        G4double fMkIIFPGASubstrateThickness;
        G4double fMkIIFPGADieThickness;
        G4double fMkIIFPGADieVHSize;

        G4double fMkIIBoardTopHeight;

        G4double fROESTIThickness;
        G4double fROESTIWidth;
        G4double fROESTIHeight;
        G4double fROESTIFPGAVSpace;
        G4double fROESTIFPGAHSpace;

        G4double fROESTIFPGADieVSize;
        G4double fROESTIFPGADieHSize;
        G4double fROESTIFPGASubstrateThickness;
        G4double fROESTIFPGADieThickness;
        G4double fROESTIFPGASubstrateVHSize;

        G4double fROESTIBoardTopHeight;

        G4double fFrameWidth;
        G4double fFrameLength;
        G4double fFirstJigZOffset;

        G4double fTriangleBracketSize;

        G4double fLShapeBracketWidth;
        G4double fLShapeBracketHeight;
        G4double fLShapeBracketLength;

        std::array<G4double, 8> fBoardZSpaces;
        std::array<G4double, 9> fXPosVJigLengths;
        std::array<G4double, 9> fXNegVJigLengths;
        std::array<G4double, 9> fJigToBoardZSpaces;
        std::array<G4double, 9> fJigToBoardWidthSpaces;
        std::array<G4double, 9> fBoardDistFromTop;
        std::array<G4double, 9> fBoardHoriOffsets;
        std::array<G4double, 9> fBracketBoardPosXMargins;
        std::array<G4double, 9> fBracketBoardNegXMargins;

        mutable G4bool fMkIIBuilt;
        mutable G4bool fRECBEBuilt;
        mutable G4bool fROESTIBuilt;

        mutable G4RotationMatrix *fmHoriJigRotMtx = nullptr;
        mutable G4RotationMatrix *fmVertJigRotMtx = nullptr;
    };
} // namespace bl10sim
#endif
