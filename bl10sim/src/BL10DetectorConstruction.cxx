#include "bl10sim/BL10DetectorConstruction.h"

#include "simcore/MetadataManager.h"
#include "simcore/TouchTriggerSD.h"

#include "G4Box.hh"
#include "G4DisplacedSolid.hh"
#include "G4ExtrudedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"

#include <limits>
#include <map>
#include <vector>

static const G4double booleanSolidTolerance = 5 * cm;

G4VPhysicalVolume *FindDaughterPVWithName(G4LogicalVolume *lv, const G4String &name,
                                          G4bool perfect = false) {
    for (size_t i = 0; i < lv->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowDaug = lv->GetDaughter(i);
        if ((!perfect && nowDaug->GetName().find(name) != nowDaug->GetName().npos) ||
            (perfect && nowDaug->GetName() == name)) {
            return nowDaug;
        }
    }
    return nullptr;
}

G4ThreeVector GetTlateToFPGADie(G4LogicalVolume *target) {
    G4ThreeVector retval;
    G4LogicalVolume *nowBoardLV = nullptr;
    for (size_t i = 0; i < target->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowDaug = target->GetDaughter(i);
        if (nowDaug->GetName().find("_PV") != nowDaug->GetName().npos) {
            nowBoardLV = nowDaug->GetLogicalVolume();
            retval += nowDaug->GetTranslation();
            break;
        }
    }

    if (nowBoardLV == nullptr) {
        G4cout << "Finding Board PV failed" << G4endl;
        return {};
    }

    G4VPhysicalVolume *diePV = nullptr;
    for (size_t i = 0; i < nowBoardLV->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowDaug = nowBoardLV->GetDaughter(i);
        if (nowDaug->GetName().find("FPGADiePV") != nowDaug->GetName().npos) {
            retval += nowDaug->GetTranslation();
            diePV = nowDaug;
            break;
        }
    }
    if (diePV == nullptr) {
        G4cout << "Finding FPGADie PV failed" << G4endl;
        return {};
    }

    return retval;
}

void PrintBoardAlignmentParameters(G4LogicalVolume *target) {
    using namespace std;
    G4VPhysicalVolume *beamPV, *expSetupPV;
    beamPV = expSetupPV = nullptr;

    for (size_t i = 0; i < target->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowPV = target->GetDaughter(i);
        if (nowPV->GetName() == "BeamWindowPV")
            beamPV = nowPV;
        else if (nowPV->GetName() == "ExperimentalSetupPV")
            expSetupPV = nowPV;

        if (expSetupPV != nullptr && beamPV != nullptr) break;
    }
    if (expSetupPV == nullptr || beamPV == nullptr) {
        G4cout << "Failed: expSetupPV or beamPV is null" << G4endl;
        return;
    }

    G4ThreeVector beamPVTlate   = beamPV->GetTranslation();
    G4ThreeVector expSetupTlate = expSetupPV->GetTranslation();

    map<pair<string, int>, G4ThreeVector> fpgaTlate;

    G4LogicalVolume *expSetupLV = expSetupPV->GetLogicalVolume();
    for (size_t i = 0; i < expSetupLV->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowPV = expSetupLV->GetDaughter(i);
        if (nowPV->GetName().find("Frame_") != nowPV->GetName().npos) {
            fpgaTlate[{nowPV->GetName(), nowPV->GetCopyNo()}] =
                expSetupTlate + nowPV->GetTranslation() +
                GetTlateToFPGADie(nowPV->GetLogicalVolume());
        }
    }

    for (auto &i : fpgaTlate) {
        G4double horiOffset  = beamPVTlate.getX() - i.second.getX();
        G4double distFromTop = -(beamPVTlate.getY() - i.second.getY());
        G4cout << i.first.first << " #" << i.first.second
               << " | Horizontal offset (mm): " << horiOffset / mm << "  |  "
               << "Distance from top (mm): " << distFromTop / mm << G4endl;
    }
}

G4ThreeVector PrintLastFPGATlate(G4LogicalVolume *target, const G4String &lastBoardName,
                                 const G4int lastBoardCopyNo) {
    using namespace std;
    G4ThreeVector lastFPGADieTlate;
    const G4ThreeVector nanVector = {numeric_limits<double>::quiet_NaN(),
                                     numeric_limits<double>::quiet_NaN(),
                                     numeric_limits<double>::quiet_NaN()};

    G4LogicalVolume *lastComplexLV = nullptr;
    for (size_t i = 0; i < target->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowDaug = target->GetDaughter(i);
        if (nowDaug->GetName().find(lastBoardName) != nowDaug->GetName().npos &&
            nowDaug->GetCopyNo() == lastBoardCopyNo) {
            lastComplexLV = nowDaug->GetLogicalVolume();
            lastFPGADieTlate += nowDaug->GetTranslation();
            break;
        }
    }

    if (lastComplexLV == nullptr) {
        G4cout << "Finding last complex LV failed" << G4endl;
        return nanVector;
    }

    G4LogicalVolume *lastBoardLV = FindDaughterPVWithName(lastComplexLV, "_PV")->GetLogicalVolume();

    if (lastComplexLV == nullptr) {
        G4cout << "Finding last board envelope LV failed" << G4endl;
        return nanVector;
    }

    G4VPhysicalVolume *diePV = nullptr;
    for (size_t i = 0; i < lastBoardLV->GetNoDaughters(); i++) {
        G4VPhysicalVolume *nowDaug = lastBoardLV->GetDaughter(i);
        if (nowDaug->GetName().find("FPGADiePV") != nowDaug->GetName().npos) {
            lastFPGADieTlate += nowDaug->GetTranslation();
            diePV = nowDaug;
            break;
        }
    }
    if (diePV == nullptr) {
        G4cout << "Finding FPGADie PV failed" << G4endl;
        return nanVector;
    }

    G4cout << "lastFPGADieTlate @ taregetLV: " << lastFPGADieTlate << G4endl;
    return lastFPGADieTlate;
}

namespace bl10sim {
    BL10DetectorConstruction::BL10DetectorConstruction() : fSimpleGeometry(false) {
        SetGeometryParameters();
        CalculateGeometrySubparameters();

        ftSimpleRotMtxX90Deg = new G4RotationMatrix();
        ftSimpleRotMtxY90Deg = new G4RotationMatrix();

        ftSimpleRotMtxX90Deg->rotateX(90 * deg);
        ftSimpleRotMtxY90Deg->rotateY(90 * deg);
    }

    void BL10DetectorConstruction::DefineMaterials() {
        DetectorConstruction::DefineMaterials();
        G4NistManager *nist = G4NistManager::Instance();
        nist->FindOrBuildMaterial("G4_Fe");
        nist->FindOrBuildMaterial("G4_Si");
        nist->FindOrBuildMaterial("G4_BORON_CARBIDE");
        nist->FindOrBuildMaterial("G4_Galactic");

        G4Material *matCu = nist->FindOrBuildMaterial("G4_Cu");
        G4Material *matNi = nist->FindOrBuildMaterial("G4_Ni");

        // Elements
        G4Element *atomH  = nist->FindOrBuildElement("H");
        G4Element *atomC  = nist->FindOrBuildElement("C");
        G4Element *atomO  = nist->FindOrBuildElement("O");
        G4Element *atomSi = nist->FindOrBuildElement("Si");
        G4Element *atomAl = nist->FindOrBuildElement("Al");
        G4Element *atomCa = nist->FindOrBuildElement("Ca");
        G4Element *atomB  = nist->FindOrBuildElement("B");
        G4Element *atomMg = nist->FindOrBuildElement("Mg");

        G4Material *matDGEBA = new G4Material("DGEBA_Epoxy", 1.16 * g / cm3, 3);
        matDGEBA->AddElement(atomC, 21);
        matDGEBA->AddElement(atomH, 24);
        matDGEBA->AddElement(atomO, 4);

        G4Material *matEGlass = new G4Material("EGlass_Epoxy", 2.55 * g / cm3, 6);
        matEGlass->AddElement(atomO, 0.47424);
        matEGlass->AddElement(atomSi, 0.25241);
        matEGlass->AddElement(atomAl, 0.07939);
        matEGlass->AddElement(atomCa, 0.15723);
        matEGlass->AddElement(atomB, 0.01863);
        matEGlass->AddElement(atomMg, 0.01809);

        G4Material *FR4 = new G4Material("FR4", 1.90 * g / cm3, 2);
        FR4->AddMaterial(matEGlass, 60. * perCent);
        FR4->AddMaterial(matDGEBA, 40. * perCent);

        G4Material *matEffectivePCB = new G4Material("EffectivePCB", 2.6 * g / cm3, 2);
        matEffectivePCB->AddMaterial(matCu, 30. * perCent);
        matEffectivePCB->AddMaterial(FR4, 70. * perCent);

        G4Material *matFPGASubstrate = new G4Material("FPGASubstrate", 2.6 * g / cm3, 2);
        matFPGASubstrate->AddMaterial(matCu, 40. * perCent);
        matFPGASubstrate->AddMaterial(FR4, 60. * perCent);

        G4Material *matLid = new G4Material("FPGALid", 8.9 * g / cm3, 2);
        matLid->AddMaterial(matCu, 99.15 * perCent);
        matLid->AddMaterial(matNi, 0.85 * perCent);
    }

    void BL10DetectorConstruction::SetGeometryParameters() {
        SetBL10RoomParameters();
        SetBoardParameters();
        SetJigFrameParameters();
    }

    void BL10DetectorConstruction::SetBL10RoomParameters() {
        fBeamYDistanceFromFloor = 180 * cm;
        fBeamXDistanceFromWall  = 75 * cm;

        fBoronResinThickness = 20 * cm;
        fIronThickness       = 30 * cm;
        fFloorThickness      = 10 * cm;
        fFeFlooringThickness = 7 * cm;

        fLabHeight        = 3.0 * m;
        fLabZLength       = 3.5 * m;
        fLabWidthBeamside = 1.9 * m;
        fLabWidthDumpside = 3.1 * m;
        fLabFloorSpace    = 25 * cm;

        fStdSmplZPosFromBeamwall = 1.5 * m;

        fExitwallDistance  = 60 * cm;
        fExitwallThickness = 50 * cm;
        fExitwallWidth     = 1 * m;
        fExitwallBRDepth   = 10 * cm;

        fExitpathWidth = 60 * cm;

        fBeamductPipeORadius       = 16 * cm;
        fBeamductPipeIRadius       = 13.5 * cm;
        fBeamductProtrusionLength  = 12 * cm;
        fBeamductEndplateRadius    = 18 * cm;
        fBeamductEndplateThickness = 0.5 * cm;

        fWorkbenchPlateWidth     = 1000 * mm;
        fWorkbenchPlateLength    = 2000 * mm;
        fWorkbenchPlateThickness = 30 * mm;

        fWorkbenchPlateOuterMargin = 60 * mm;
        fWorkbenchPlateInnerMargin = 25 * mm;

        fWorkbenchSupportWidth     = 100 * mm;
        fWorkbenchSupportHeight    = 100 * mm;
        fWorkbenchSupportThickness = 10 * mm;
        fWorkbenchXSupportLength   = 850 * mm;
        fWorkbenchZSupportLength   = 2050 * mm;

        fWBPipeEndplateThickness = 20 * mm;
        fWBPipeEndplate1Width    = 220 * mm;
        fWBPipeEndplate2Width    = 220 * mm;
        fWBPipeEndplate3Width    = 115 * mm;
        fWBPipeEndplate1Length   = 220 * mm;
        fWBPipeEndplate2Length   = 320 * mm;
        fWBPipeEndplate3Length   = 155 * mm;

        fWBVertPipeLength     = 1000 * mm;
        fWBZDiagPipeLength    = 1000 * mm;
        fWBXDiagPipeLength    = 630 * mm;
        fWBVertPipeRadius     = 115 * mm / 2.;
        fWBZDiagPipeRadius    = 65 * mm / 2.;
        fWBXDiagPipeRadius    = 65 * mm / 2.;
        fWBVertPipeThickness  = 4.5 * mm;
        fWBZDiagPipeThickness = 3 * mm;
        fWBXDiagPipeThickness = 3 * mm;

        fWBZDiagPipeAngle     = 40 * deg;
        fWBXDiagPipeAngle     = 20 * deg;
        fWBZDiagPipeYDistance = 55 * mm;
        fWBXDiagPipeYDistance = 190 * mm;

        fWBLevelingBoltSize      = 60 * mm;
        fWBLevelingBoltThickness = 30 * mm;

        fWBZDistanceFromWall = 10 * cm;

        fBeamWindowWidth  = 47 * mm;
        fBeamWindowHeight = 10 * cm;

        fWindowThickness = 1 * nm;

        fSlitFrameZLength = 10 * cm;

        fESlitFrameWidth  = 52 * cm;
        fESlitFrameHeight = 52 * cm;
        fISlitFrameWidth  = 40 * cm;
        fISlitFrameHeight = 40 * cm;

        fESlitFrameThickness = 2 * cm;
        fISlitFrameThickness = 2 * cm;

        fSlitThickness     = 0.5 * cm;
        fSlitZDistFromWall = 17.5 * cm;

        fSlitHVZSpace = 4 * cm;
        fSlitVSpace   = 10 * cm;
        fSlitHSpace   = 10 * cm;

        fSlitStandHeight    = 20 * cm;
        fSlitStandWidth     = 56 * cm;
        fSlitStandZLength   = 25 * cm;
        fSlitStandThickness = 1 * mm;

        fSlitUStandHeight    = 10 * cm;
        fSlitUStandZLength   = 17.5 * cm;
        fSlitUStandWidth     = 75 * cm;
        fSlitUStandThickness = 3 * cm;
    }

    void BL10DetectorConstruction::SetBoardParameters() {
        fJigVHSize = 2 * cm;

        fJigCenterHoleRadius = 2 * mm;

        fJigSpaceStemBottomWidth = 6 * mm;
        fJigSpaceStemLength      = 2 * mm;
        fJigSpaceMiddleWidth     = 3 * mm;
        fJigSpaceMiddleLength    = 1 * mm;
        fJigSpaceHeight          = 6 * mm;

        fRECBETopWidth    = 20 * cm;
        fRECBEThickness   = 2 * mm;
        fRECBEMiddleWidth = 5.8 * cm;
        fRECBELongHeight  = 17 * cm;
        fRECBEShortHeight = 12 * cm;
        fRECBEFPGAVSpace  = 6.2 * cm;
        fRECBEFPGAHSpace  = 7.5 * cm;

        fRECBEFPGASubstrateVHSize    = 4.3 * cm;
        fRECBEFPGALidThickness       = 0.8 * mm;
        fRECBEFPGASubstrateThickness = 1.0 * mm;
        fRECBEFPGADieThickness       = 0.8 * mm;
        fRECBEFPGADieVHSize          = 3.5 * cm;

        fMkIITopWidth    = 20 * cm;
        fMkIIThickness   = 2 * mm;
        fMkIIMiddleWidth = 6 * cm;
        fMkIILongHeight  = 16.5 * cm;
        fMkIIShortHeight = 10.2 * cm;
        fMkIIFPGAVSpace  = 6. * cm;
        fMkIIFPGAHSpace  = 8.2 * cm;

        fMkIIFPGASubstrateVHSize    = 2.6 * cm;
        fMkIIFPGALidThickness       = 0.5 * mm;
        fMkIIFPGASubstrateThickness = 1.0 * mm;
        fMkIIFPGADieThickness       = 0.2 * mm;
        fMkIIFPGADieVHSize          = 1.0 * cm;

        fROESTIWidth      = 18.5 * cm;
        fROESTIThickness  = 2 * mm;
        fROESTIHeight     = 8 * cm;
        fROESTIFPGAVSpace = 2.9 * cm;
        fROESTIFPGAHSpace = 7.7 * cm;

        fROESTIFPGADieVSize           = 1.2 * cm;
        fROESTIFPGADieHSize           = 1.0 * cm;
        fROESTIFPGASubstrateThickness = 1 * mm;
        fROESTIFPGADieThickness       = 0.2 * mm;
        fROESTIFPGASubstrateVHSize    = 2.6 * cm;
    }

    void BL10DetectorConstruction::SetJigFrameParameters() {
        // TODO: Correct these values with the measured ones
        G4double jigToBoardZSpace = 15 * mm;

        G4double bracketROESTIPosXMargin = -0.2 * mm;
        G4double bracketROESTINegXMargin = 15 * mm;
        G4double bracketRECBEPosXMargin  = 12 * mm;
        G4double bracketRECBENegXMargin  = 12 * mm;
        G4double bracketMkIIPosXMargin   = 10 * mm;
        G4double bracketMkIINegXMargin   = 10 * mm;

        G4double auxBracketROESTIPosXYOffset = -7.0 * cm;
        G4double auxBracketROESTINegXYOffset = 0;
        G4double auxBracketRECBEPosXYOffset  = 0;
        G4double auxBracketRECBENegXYOffset  = -5.0 * cm;
        G4double auxBracketMkII1PosXYOffset  = -6.0 * cm;
        G4double auxBracketMkII1NegXYOffset  = 0;
        G4double auxBracketMkII2PosXYOffset  = 0;
        G4double auxBracketMkII2NegXYOffset  = -6.0 * cm;

        fFirstJigZOffset = 0 * mm;

        fVJigType1Length = 30 * cm;
        fVJigType2Length = 34 * cm;

        fFrameWidth      = 30 * cm;
        fFrameLength     = 80 * cm;
        fRearFrameLength = 75 * cm;

        fJackWidth     = 40 * cm;
        fJackZLength   = 75 * cm;
        fJackHeight    = 36 * cm;
        fJackThickness = 2 * mm;

        fTriangleBracketSize = 1.5 * cm;

        fLShapeBracketLength = 3.5 * cm; // jigToBoardZSpace + fJigVHSize
        fLShapeBracketHeight = 1.0 * cm;
        fLShapeBracketWidth  = 0.4 * cm;

        fLastFPGAZOffsetFromStdSmplPos = -3 * cm;

        fBoardZSpaces[0] = 47.1 * mm;
        fBoardZSpaces[1] = 47.8 * mm;
        fBoardZSpaces[2] = 51.4 * mm;
        fBoardZSpaces[3] = 51.1 * mm;
        fBoardZSpaces[4] = 46.0 * mm;
        fBoardZSpaces[5] = 52.0 * mm;
        fBoardZSpaces[6] = 51.0 * mm;
        fBoardZSpaces[7] = 47.5 * mm;

        fXNegVJigLengths[0] = fVJigType1Length;
        fXNegVJigLengths[1] = fVJigType1Length;
        fXNegVJigLengths[2] = fVJigType2Length;
        fXNegVJigLengths[3] = fVJigType1Length;
        fXNegVJigLengths[4] = fVJigType1Length;
        fXNegVJigLengths[5] = fVJigType2Length;
        fXNegVJigLengths[6] = fVJigType2Length;
        fXNegVJigLengths[7] = fVJigType1Length;
        fXNegVJigLengths[8] = fVJigType2Length;

        fXPosVJigLengths[0] = fVJigType1Length;
        fXPosVJigLengths[1] = fVJigType1Length;
        fXPosVJigLengths[2] = fVJigType2Length;
        fXPosVJigLengths[3] = fVJigType1Length;
        fXPosVJigLengths[4] = fVJigType1Length;
        fXPosVJigLengths[5] = fVJigType2Length;
        fXPosVJigLengths[6] = fVJigType2Length;
        fXPosVJigLengths[7] = fVJigType2Length;
        fXPosVJigLengths[8] = fVJigType2Length;

        fJigToBoardZSpaces[0] = jigToBoardZSpace;
        fJigToBoardZSpaces[1] = jigToBoardZSpace;
        fJigToBoardZSpaces[2] = jigToBoardZSpace;
        fJigToBoardZSpaces[3] = jigToBoardZSpace;
        fJigToBoardZSpaces[4] = jigToBoardZSpace;
        fJigToBoardZSpaces[5] = jigToBoardZSpace;
        fJigToBoardZSpaces[6] = jigToBoardZSpace;
        fJigToBoardZSpaces[7] = jigToBoardZSpace;
        fJigToBoardZSpaces[8] = jigToBoardZSpace;

        fBoardDistFromTop[0] = 98 * mm;
        fBoardDistFromTop[1] = 56.5 * mm;
        fBoardDistFromTop[2] = 107.0 * mm;
        fBoardDistFromTop[3] = 98 * mm;
        fBoardDistFromTop[4] = 56.5 * mm;
        fBoardDistFromTop[5] = 107.0 * mm;
        fBoardDistFromTop[6] = 138 * mm;
        fBoardDistFromTop[7] = 56.5 * mm;
        fBoardDistFromTop[8] = 107.0 * mm;

        fBoardHoriOffsets[0] = -2.5 * mm;
        fBoardHoriOffsets[1] = -3.5 * mm;
        fBoardHoriOffsets[2] = -5.0 * mm;
        fBoardHoriOffsets[3] = -2.5 * mm;
        fBoardHoriOffsets[4] = -3.5 * mm;
        fBoardHoriOffsets[5] = -5.0 * mm;
        fBoardHoriOffsets[6] = -2.5 * mm;
        fBoardHoriOffsets[7] = -3.5 * mm;
        fBoardHoriOffsets[8] = -5.0 * mm;

        fBracketBoardPosXMargins[0] = bracketROESTIPosXMargin;
        fBracketBoardPosXMargins[1] = bracketRECBEPosXMargin;
        fBracketBoardPosXMargins[2] = bracketMkIIPosXMargin;
        fBracketBoardPosXMargins[3] = bracketROESTIPosXMargin;
        fBracketBoardPosXMargins[4] = bracketRECBEPosXMargin;
        fBracketBoardPosXMargins[5] = bracketMkIIPosXMargin;
        fBracketBoardPosXMargins[6] = bracketROESTIPosXMargin;
        fBracketBoardPosXMargins[7] = bracketRECBEPosXMargin;
        fBracketBoardPosXMargins[8] = bracketMkIIPosXMargin;

        fBracketBoardNegXMargins[0] = bracketROESTINegXMargin;
        fBracketBoardNegXMargins[1] = bracketRECBENegXMargin;
        fBracketBoardNegXMargins[2] = bracketMkIINegXMargin;
        fBracketBoardNegXMargins[3] = bracketROESTINegXMargin;
        fBracketBoardNegXMargins[4] = bracketRECBENegXMargin;
        fBracketBoardNegXMargins[5] = bracketMkIINegXMargin;
        fBracketBoardNegXMargins[6] = bracketROESTINegXMargin;
        fBracketBoardNegXMargins[7] = bracketRECBENegXMargin;
        fBracketBoardNegXMargins[8] = bracketMkIINegXMargin;

        fPosXBracketYOffset[0] = auxBracketROESTIPosXYOffset;
        fPosXBracketYOffset[1] = auxBracketRECBEPosXYOffset;
        fPosXBracketYOffset[2] = auxBracketMkII1PosXYOffset;
        fPosXBracketYOffset[3] = auxBracketROESTIPosXYOffset;
        fPosXBracketYOffset[4] = auxBracketRECBEPosXYOffset;
        fPosXBracketYOffset[5] = auxBracketMkII1PosXYOffset;
        fPosXBracketYOffset[6] = auxBracketROESTIPosXYOffset;
        fPosXBracketYOffset[7] = auxBracketRECBEPosXYOffset;
        fPosXBracketYOffset[8] = auxBracketMkII2PosXYOffset;

        fNegXBracketYOffset[0] = auxBracketROESTINegXYOffset;
        fNegXBracketYOffset[1] = auxBracketRECBENegXYOffset;
        fNegXBracketYOffset[2] = auxBracketMkII1NegXYOffset;
        fNegXBracketYOffset[3] = auxBracketROESTINegXYOffset;
        fNegXBracketYOffset[4] = auxBracketRECBENegXYOffset;
        fNegXBracketYOffset[5] = auxBracketMkII1NegXYOffset;
        fNegXBracketYOffset[6] = auxBracketROESTINegXYOffset;
        fNegXBracketYOffset[7] = auxBracketRECBENegXYOffset;
        fNegXBracketYOffset[8] = auxBracketMkII2NegXYOffset;
    }

    void BL10DetectorConstruction::CalculateGeometrySubparameters() {
        ftLabWidthSlope = (fLabWidthDumpside - fLabWidthBeamside) / fLabZLength;

        ftWBEnvelopeWidth =
            fWorkbenchXSupportLength + fWorkbenchSupportWidth * 2 + fWorkbenchPlateOuterMargin * 2;
        ftWBEnvelopeHeight = fWBPipeEndplateThickness + fWBVertPipeLength +
                             fWBPipeEndplateThickness + fWorkbenchSupportHeight +
                             fWorkbenchPlateThickness;
        ftWBEnvelopeZLength = fWorkbenchPlateOuterMargin * 2 + fWorkbenchZSupportLength;

        G4TwoVector nowHBeamPoint;

        G4double hbXSize     = fWorkbenchSupportWidth;
        G4double hbYSize     = fWorkbenchSupportHeight;
        G4double hbThickness = fWorkbenchSupportThickness;

        ftHBeamPoints.clear();

        nowHBeamPoint = {-hbThickness / 2., -hbYSize / 2. + hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbThickness / 2. - hbXSize / 2., 0};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, -hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbXSize, 0};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbXSize / 2. + hbThickness / 2., 0};
        ftHBeamPoints.push_back(nowHBeamPoint);

        nowHBeamPoint = {hbThickness / 2., hbYSize / 2. - hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbThickness / 2. + hbXSize / 2., 0};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {-hbXSize, 0};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {0, -hbThickness};
        ftHBeamPoints.push_back(nowHBeamPoint);
        nowHBeamPoint += {hbXSize / 2. - hbThickness / 2., 0};
        ftHBeamPoints.push_back(nowHBeamPoint);

        ftLevelingBoltPoints.clear();

        G4TwoVector nowLBPoint = {0, fWBLevelingBoltSize / 2.};
        ftLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        ftLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        ftLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        ftLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        ftLevelingBoltPoints.push_back(nowLBPoint);
        nowLBPoint.rotate(60 * deg);
        ftLevelingBoltPoints.push_back(nowLBPoint);

        G4TwoVector nowJigSpacePoint = {0, 0};

        ftJigSpacePoints.clear();

        std::vector<G4TwoVector> jigSpaceRPoints, jigSpaceLPoints;

        nowJigSpacePoint += {fJigSpaceStemBottomWidth / 2., 0};
        jigSpaceRPoints.push_back(nowJigSpacePoint);
        jigSpaceLPoints.push_back({-nowJigSpacePoint.x(), nowJigSpacePoint.y()});

        nowJigSpacePoint += {0, fJigSpaceStemLength};
        jigSpaceRPoints.push_back(nowJigSpacePoint);
        jigSpaceLPoints.push_back({-nowJigSpacePoint.x(), nowJigSpacePoint.y()});

        nowJigSpacePoint += {fJigSpaceMiddleWidth, 0};
        jigSpaceRPoints.push_back(nowJigSpacePoint);
        jigSpaceLPoints.push_back({-nowJigSpacePoint.x(), nowJigSpacePoint.y()});

        nowJigSpacePoint += {0, fJigSpaceMiddleLength};
        jigSpaceRPoints.push_back(nowJigSpacePoint);
        jigSpaceLPoints.push_back({-nowJigSpacePoint.x(), nowJigSpacePoint.y()});

        nowJigSpacePoint = {fJigSpaceStemBottomWidth / 2., fJigSpaceHeight};
        jigSpaceRPoints.push_back(nowJigSpacePoint);
        jigSpaceLPoints.push_back({-nowJigSpacePoint.x(), nowJigSpacePoint.y()});

        for (auto i = jigSpaceRPoints.begin(); i != jigSpaceRPoints.end(); ++i)
            ftJigSpacePoints.push_back(*i);
        for (auto i = jigSpaceLPoints.rbegin(); i != jigSpaceLPoints.rend(); ++i)
            ftJigSpacePoints.push_back(*i);

        G4TwoVector nowRECBEPoint = {0, 0};

        ftRECBEBoardPoints.clear();

        nowRECBEPoint += {fRECBETopWidth / 2., 0};
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        nowRECBEPoint += {0, -fRECBEShortHeight};
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        nowRECBEPoint += {-fRECBEMiddleWidth, 0};
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        nowRECBEPoint.setY(-fRECBELongHeight);
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        nowRECBEPoint.setX(-fRECBETopWidth / 2.);
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        nowRECBEPoint.setY(0);
        ftRECBEBoardPoints.push_back(nowRECBEPoint);

        G4TwoVector nowMkIIPoint = {0, 0};

        ftMkIIBoardPoints.clear();

        nowMkIIPoint += {fMkIITopWidth / 2., 0};
        ftMkIIBoardPoints.push_back(nowMkIIPoint);

        nowMkIIPoint += {0, -fMkIIShortHeight};
        ftMkIIBoardPoints.push_back(nowMkIIPoint);

        nowMkIIPoint += {-fMkIIMiddleWidth, 0};
        ftMkIIBoardPoints.push_back(nowMkIIPoint);

        nowMkIIPoint.setY(-fMkIILongHeight);
        ftMkIIBoardPoints.push_back(nowMkIIPoint);

        nowMkIIPoint.setX(-fMkIITopWidth / 2.);
        ftMkIIBoardPoints.push_back(nowMkIIPoint);

        nowMkIIPoint.setY(0);
        ftMkIIBoardPoints.push_back(nowMkIIPoint);
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildIroncase() const {
        // Making a logical volume for the world (Iron case)
        G4Material *matIron = G4Material::GetMaterial("G4_Fe");

        G4double worldZLength = fLabZLength + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldHeight = fLabHeight + fBoronResinThickness + fIronThickness + fFloorThickness;

        // Calculating the width of the iron case at the beam/dump side
        // (assuming beamside < dumpside)
        G4double worldWidthAtLabBeamBoundary =
            fLabWidthBeamside + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldWidthAtLabDumpBoundary =
            fLabWidthDumpside + 2 * (fBoronResinThickness + fIronThickness);
        G4double worldWidthBeamside =
            worldWidthAtLabBeamBoundary - ftLabWidthSlope * (fBoronResinThickness + fIronThickness);
        G4double worldWidthDumpside =
            worldWidthAtLabDumpBoundary + ftLabWidthSlope * (fBoronResinThickness + fIronThickness);

        G4Trd *worldTrd = new G4Trd("IroncaseTrd", worldWidthBeamside / 2., worldWidthDumpside / 2.,
                                    worldHeight / 2., worldHeight / 2., worldZLength / 2.);
        G4LogicalVolume *worldLV = new G4LogicalVolume(worldTrd, matIron, "IroncaseLV");

        G4Material *matConcrete = G4Material::GetMaterial("G4_CONCRETE");
        G4Trd *floorTrd = new G4Trd("FloorTrd", worldWidthBeamside / 2., worldWidthDumpside / 2.,
                                    fFloorThickness / 2., fFloorThickness / 2., worldZLength / 2.);
        G4LogicalVolume *floorLV = new G4LogicalVolume(floorTrd, matConcrete, "FloorLV");

        G4ThreeVector floorTlate = {0, -worldHeight / 2. + fFloorThickness / 2., 0};
        new G4PVPlacement(nullptr, floorTlate, floorLV, "FloorPV", worldLV, false, 0,
                          fCheckOverlaps);

        return worldLV;
    }

    G4VSolid *BL10DetectorConstruction::BuildBoronResincaseSolid(G4bool simple) const {
        G4double boronResinZLength = fLabZLength + 2 * fBoronResinThickness + fIronThickness;
        G4double boronResinHeight  = fLabHeight + fBoronResinThickness;

        // Calculating the width of the boron-resin case at the beam/dump side
        // (assuming beamside < dumpside)
        G4double boronResinWidthAtLabBeamBoundary = fLabWidthBeamside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtLabDumpBoundary = fLabWidthDumpside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtOrigBeamBoundary =
            boronResinWidthAtLabBeamBoundary - ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthAtOrigDumpBoundary =
            boronResinWidthAtLabDumpBoundary + ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthBeamside = boronResinWidthAtOrigBeamBoundary;
        G4double boronResinWidthDumpside =
            boronResinWidthAtOrigDumpBoundary + ftLabWidthSlope * fIronThickness;

        // Build the Boron-Resin case (simple version)
        G4Trd *boronResinTrd = new G4Trd("BoronResinCaseTrd", boronResinWidthBeamside / 2.,
                                         boronResinWidthDumpside / 2., boronResinHeight / 2.,
                                         boronResinHeight / 2., boronResinZLength / 2.);

        if (simple) return boronResinTrd;

        G4DisplacedSolid *displacedBRTrd = new G4DisplacedSolid(
            "BoronResinCaseDisplacedTrd", boronResinTrd, nullptr, {0, 0, fIronThickness / 2.});

        G4double ewCarverBoxWidth = (fExitwallWidth - ftLabWidthSlope / 2. * fExitwallBRDepth) +
                                    fBoronResinThickness - fExitwallBRDepth +
                                    booleanSolidTolerance * 2;
        G4double ewCarverBoxHeight    = fLabHeight + booleanSolidTolerance;
        G4double ewCarverBoxThickness = fExitwallThickness - 2 * fExitwallBRDepth;

        G4Box *exitwallCarverBox =
            new G4Box("BoronResinCaseExitwallCarverBox", ewCarverBoxWidth / 2.,
                      ewCarverBoxHeight / 2., ewCarverBoxThickness / 2.);

        // Tlanslation for the carver
        G4ThreeVector ewCarverTlate = {0, 0, 0};
        // Apply a bias of the center of G4Trd
        // -z length: fLabZLength/2+fBoronResinThickness
        // +z length: fLabZLength/2+fBoronResinThickness + fIronThickness
        ewCarverTlate += {0, 0, fIronThickness / 2.};
        // Move the carver center to the +z end
        ewCarverTlate += {0, 0, boronResinZLength / 2.};
        // Compenstae the thickness of the iron
        ewCarverTlate += {0, 0, -fIronThickness};
        // Move the center to the +z edge of exitwall iron shield (inside the boron-resin shield)
        ewCarverTlate += {0, 0, -fExitwallDistance - fExitwallBRDepth - fBoronResinThickness};
        // Move the center to the x-border of exitwall
        ewCarverTlate += {-boronResinWidthAtOrigDumpBoundary / 2. +
                              ftLabWidthSlope / 2. *
                                  (fExitwallBRDepth + fExitwallDistance + fBoronResinThickness),
                          0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {ewCarverBoxWidth / 2. - 2 * booleanSolidTolerance, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2. + fExitwallBRDepth};
        // Make the carver cling to the bottom
        ewCarverTlate += {0, -fBoronResinThickness / 2. - booleanSolidTolerance / 2., 0};

        G4SubtractionSolid *carvedBRCase =
            new G4SubtractionSolid("BoronResinCaseWExitwallSSolid", displacedBRTrd,
                                   exitwallCarverBox, nullptr, ewCarverTlate);

        G4double exitpathCarverHeight  = boronResinHeight + 2 * booleanSolidTolerance;
        G4double exitpathCarverZLength = fIronThickness + booleanSolidTolerance;
        G4double exitpathCarverMZWidth = boronResinWidthAtOrigDumpBoundary - fExitpathWidth -
                                         2 * fBoronResinThickness + booleanSolidTolerance;
        G4double exitpathCarverPZWidth =
            boronResinWidthDumpside - fExitpathWidth - 2 * fBoronResinThickness +
            ftLabWidthSlope * booleanSolidTolerance + booleanSolidTolerance;

        G4Trd *exitpathCarver = new G4Trd("BRCaseExitCarverTrd", exitpathCarverMZWidth / 2.,
                                          exitpathCarverPZWidth / 2., exitpathCarverHeight / 2.,
                                          exitpathCarverHeight / 2., exitpathCarverZLength / 2.);

        G4ThreeVector eCarverTlate = {0, 0, 0};
        // Compensate the displacement of G4DisplacedSolid
        eCarverTlate += {0, 0, fIronThickness / 2.};
        // Move +z to the +z-end of the Boron-resin solid with consideration of boolean tolerance
        eCarverTlate += {0, 0, boronResinZLength / 2. + booleanSolidTolerance / 2.};
        // Insert carver to -z to make the boron-resin+iron shield region
        eCarverTlate += {0, 0, -fIronThickness / 2.};
        // Move +x to make the exit path with consideration of boolean tolerance
        eCarverTlate += {fExitpathWidth / 2. + booleanSolidTolerance / 2., 0, 0};
        // Adding addition +x for the thickness of boron-resin layer
        eCarverTlate += {fBoronResinThickness, 0, 0};
        // Move +y to make the spacing of lab at the floor with consideration of boolean tolerance
        eCarverTlate += {0, fLabFloorSpace + booleanSolidTolerance, 0};

        G4SubtractionSolid *brCaseWithExit = new G4SubtractionSolid(
            "BoronResinCaseSolid", carvedBRCase, exitpathCarver, nullptr, eCarverTlate);

        return brCaseWithExit;
    }

    G4VSolid *BL10DetectorConstruction::BuildLabSolids(G4bool simple,
                                                       G4VSolid *&feFlooringSolid) const {
        G4double labTrdZLength       = fLabZLength + fBoronResinThickness + fIronThickness;
        G4double labTrdHeight        = fLabHeight - booleanSolidTolerance;
        G4double labTrdWidthBeamside = fLabWidthBeamside;
        G4double labTrdWidthDumpside =
            fLabWidthDumpside + ftLabWidthSlope * (fBoronResinThickness + fIronThickness);

        if (booleanSolidTolerance >= fLabFloorSpace / 2.) {
            G4ExceptionDescription msg;
            msg << "The tolerance of boolean solids is too large (> fLabFloorSpace[="
                << fLabFloorSpace / cm << " cm])!! The resultant solid may be inaccurate.";
            G4Exception("BL10DetectorConstruction::BuildLabSolids", "BL10GeometryE0001",
                        JustWarning, msg);
        }

        G4Trd *labTrd = new G4Trd("LabTrd", labTrdWidthBeamside / 2., labTrdWidthDumpside / 2.,
                                  labTrdHeight / 2., labTrdHeight / 2., labTrdZLength / 2.);

        G4Trd *labFeFlooringTrd =
            new G4Trd("LabFeFlooringTrd", labTrdWidthBeamside / 2., labTrdWidthDumpside / 2.,
                      fFeFlooringThickness / 2., fFeFlooringThickness / 2., labTrdZLength / 2.);

        // Copied from BuildBoronResincaseSolid()
        // (assuming beamside < dumpside)
        G4double boronResinZLength = fLabZLength + 2 * fBoronResinThickness + fIronThickness;

        G4double boronResinWidthAtLabBeamBoundary = fLabWidthBeamside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtLabDumpBoundary = fLabWidthDumpside + 2 * fBoronResinThickness;
        G4double boronResinWidthAtOrigBeamBoundary =
            boronResinWidthAtLabBeamBoundary - ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthAtOrigDumpBoundary =
            boronResinWidthAtLabDumpBoundary + ftLabWidthSlope * fBoronResinThickness;
        G4double boronResinWidthBeamside = boronResinWidthAtOrigBeamBoundary;
        G4double boronResinWidthDumpside =
            boronResinWidthAtOrigDumpBoundary + ftLabWidthSlope * fIronThickness;

        G4DisplacedSolid *displacedLabTrd = new G4DisplacedSolid(
            "DisplacedLabTrd", labTrd, nullptr, {0, booleanSolidTolerance / 2., 0});

        G4Trd *labFloorTrd =
            new G4Trd("LabFloorTrd", boronResinWidthBeamside / 2., boronResinWidthDumpside / 2.,
                      fLabFloorSpace / 2., fLabFloorSpace / 2., boronResinZLength / 2.);

        G4ThreeVector floorSpaceTlate = {0, 0, 0};
        // Moving the center of floor spaing to the bottom of lab
        floorSpaceTlate += {0, -fLabHeight / 2., 0};
        // Insert the floor spacing to the lab
        floorSpaceTlate += {0, fLabFloorSpace / 2., 0};

        G4UnionSolid *labTrdWithFloor;
        if (simple) {
            labTrdWithFloor =
                new G4UnionSolid("LabFloorSolid", labTrd, labFloorTrd, nullptr, floorSpaceTlate);
            feFlooringSolid = labFeFlooringTrd;
            return labTrdWithFloor;
        } else {
            // Compensating a tlanslation for the the iron case
            floorSpaceTlate += {0, 0, labTrdZLength / 2. - boronResinZLength / 2.};

            labTrdWithFloor = new G4UnionSolid("LabFloorSolid", displacedLabTrd, labFloorTrd,
                                               nullptr, floorSpaceTlate);
        }

        G4double ewCarverBoxWidth =
            fExitwallWidth + fBoronResinThickness + booleanSolidTolerance * 2;
        G4double ewCarverBoxHeight  = fLabHeight + booleanSolidTolerance * 2;
        G4double ewCarverBoxZLength = fExitwallThickness;

        G4Box *exitwallCarverBox = new G4Box("LabExitwallCarverBox", ewCarverBoxWidth / 2.,
                                             ewCarverBoxHeight / 2., ewCarverBoxZLength / 2.);

        // Tlanslation for the carver
        G4ThreeVector ewCarverTlate = {0, 0, 0};
        // Move the carver center to the +z end
        ewCarverTlate += {0, 0, fLabZLength / 2.};
        // Move the center to the +z edge of exitwall Boron-resin shield
        ewCarverTlate += {0, 0, -fExitwallDistance};
        // Move the center to the x-border of exitwall
        ewCarverTlate += {-fLabWidthDumpside / 2. + ftLabWidthSlope * fExitwallDistance / 2., 0, 0};
        // Put the carver box to the innerside of room with consideration of tolerance.
        ewCarverTlate += {-ewCarverBoxWidth / 2., 0, 0};
        ewCarverTlate += {fExitwallWidth, 0, 0};
        // Move the box to the center of exitwall
        ewCarverTlate += {0, 0, -fExitwallThickness / 2.};

        G4ThreeVector ltwfDisplacement = {0, 0, (fBoronResinThickness + fIronThickness) / 2.};

        G4DisplacedSolid *displacedLTWF = new G4DisplacedSolid(
            "DisplacedLabFloorSolid", labTrdWithFloor, nullptr, ltwfDisplacement);
        G4DisplacedSolid *displacedLFFT = new G4DisplacedSolid(
            "DisplacedLabFeFlooringTrd", labFeFlooringTrd, nullptr, ltwfDisplacement);

        G4SubtractionSolid *carvedLab = new G4SubtractionSolid(
            "LabWExitwallSSolid", displacedLTWF, exitwallCarverBox, nullptr, ewCarverTlate);
        G4SubtractionSolid *carvedFeFlooringSolid =
            new G4SubtractionSolid("LabFeFlooringWExitwallSSolid", displacedLFFT, exitwallCarverBox,
                                   nullptr, ewCarverTlate);

        G4double exitpathCarverHeight = fLabHeight + 2 * booleanSolidTolerance;
        G4double exitpathCarverZLength =
            fBoronResinThickness + fIronThickness + booleanSolidTolerance;
        G4double exitpathCarverMZWidth = boronResinWidthDumpside -
                                         ftLabWidthSlope * (fBoronResinThickness + fIronThickness) +
                                         booleanSolidTolerance - fExitpathWidth;
        G4double exitpathCarverPZWidth = boronResinWidthDumpside +
                                         ftLabWidthSlope * booleanSolidTolerance +
                                         booleanSolidTolerance - fExitpathWidth;

        G4Trd *exitpathCarver = new G4Trd("LabExitCarverTrd", exitpathCarverMZWidth / 2.,
                                          exitpathCarverPZWidth / 2., exitpathCarverHeight / 2.,
                                          exitpathCarverHeight / 2., exitpathCarverZLength / 2.);

        G4ThreeVector eCarverTlate = {0, 0, 0};
        // Compensate the displacement of the previous instances of G4DisplacedSolid
        eCarverTlate += {0, 0, (fBoronResinThickness + fIronThickness) / 2.};
        // Move +z to the +z-end of the labTrd solid with consideration of boolean tolerance
        eCarverTlate += {0, 0, labTrdZLength / 2. + booleanSolidTolerance / 2.};
        // Insert carver to -z to make the boron-resin+iron shield region
        eCarverTlate += {0, 0, -(fBoronResinThickness + fIronThickness) / 2.};
        // Move +x to make the exit path with consideration of boolean tolerance
        eCarverTlate += {fExitpathWidth / 2. + booleanSolidTolerance / 2., 0, 0};
        // Adding addition +x for the thickness of boron-resin layer
        eCarverTlate += {fBoronResinThickness, 0, 0};
        // Move +y to make the spacing of lab at the floor with consideration of boolean tolerance
        eCarverTlate += {0, fLabFloorSpace + booleanSolidTolerance, 0};

        G4SubtractionSolid *labWithExit = new G4SubtractionSolid(
            "LabWExitSolid", carvedLab, exitpathCarver, nullptr, eCarverTlate);
        feFlooringSolid = new G4SubtractionSolid("LabFeFlooringWExitSolid", carvedFeFlooringSolid,
                                                 exitpathCarver, nullptr, eCarverTlate);

        return labWithExit;
    }

    G4LogicalVolume *
        BL10DetectorConstruction::FillExperimentalRoom(G4LogicalVolume *ironcaseLV,
                                                       G4ThreeVector &workbenchCenter,
                                                       G4ThreeVector &wbCenterOnBeamAxis) const {
        G4Material *matB4C = G4Material::GetMaterial("G4_BORON_CARBIDE");
        G4Material *matAir = G4Material::GetMaterial("G4_AIR");
        G4Material *matFe  = G4Material::GetMaterial("G4_Fe");
        G4Material *matSS  = G4Material::GetMaterial("Stainless_Steel");

        G4LogicalVolume *boronResinLV = new G4LogicalVolume(
            BuildBoronResincaseSolid(fSimpleGeometry), matB4C, "BoronResinCaseLV");
        G4ThreeVector boronResinTlate = {
            0, -(fFloorThickness + fIronThickness) / 2. + fFloorThickness, 0};
        new G4PVPlacement(nullptr, boronResinTlate, boronResinLV, "BoronResinCasePV", ironcaseLV,
                          false, 0, fCheckOverlaps);

        G4VSolid *feFlooringSolid;
        G4LogicalVolume *labLV =
            new G4LogicalVolume(BuildLabSolids(fSimpleGeometry, feFlooringSolid), matAir, "LabLV");
        G4ThreeVector labTlate = {0, -fBoronResinThickness / 2., 0};
        new G4PVPlacement(nullptr, labTlate, labLV, "LabPV", boronResinLV, false, 0,
                          fCheckOverlaps);

        G4ThreeVector feFlooringTlate = {0, -fLabHeight / 2. + fFeFlooringThickness / 2., 0};
        G4LogicalVolume *feFlooringLV =
            new G4LogicalVolume(feFlooringSolid, matFe, "LabFlooringLV");
        new G4PVPlacement(nullptr, feFlooringTlate, feFlooringLV, "LabFlooringPV", labLV, false, 0,
                          fCheckOverlaps);

        G4Box *windowBox = new G4Box("BeamWindowBox", fBeamWindowWidth / 2., fBeamWindowHeight / 2.,
                                     fWindowThickness / 2.);
        G4LogicalVolume *windowLV = new G4LogicalVolume(windowBox, matAir, "BeamWindowLV");

        G4ThreeVector windowTlate = {0, 0, 0};
        // Move the beam window solid to the z-end of the lab
        windowTlate += {0, 0, -fLabZLength / 2. + fWindowThickness / 2.};
        // Move the beam window to the fBeamYDistanceFromFloor on the y-axis
        windowTlate += {0, -fLabHeight / 2. + fBeamYDistanceFromFloor, 0};
        // Move the beam window to the center of beamline
        windowTlate += {fLabWidthBeamside / 2. - fBeamXDistanceFromWall, 0, 0};

        new G4PVPlacement(nullptr, windowTlate, windowLV, "BeamWindowPV", labLV, false, 0,
                          fCheckOverlaps);

        G4ThreeVector iDuctTlate = windowTlate + labTlate + boronResinTlate;
        iDuctTlate.setZ(fLabZLength / 2. + fBoronResinThickness + fIronThickness / 2.);
        G4LogicalVolume *iDuctLV = BuildBeamductPipe("Ironcase", fIronThickness);
        new G4PVPlacement(nullptr, iDuctTlate, iDuctLV, "IroncaseBeamductPipePV", ironcaseLV, false,
                          0, fCheckOverlaps);

        G4ThreeVector brDuctTlate = windowTlate + labTlate;
        brDuctTlate.setZ(fLabZLength / 2. + fBoronResinThickness / 2.);
        G4LogicalVolume *brDuctLV = BuildBeamductPipe("BoronResin", fBoronResinThickness);
        new G4PVPlacement(nullptr, brDuctTlate, brDuctLV, "BoronResinBeamductPV", boronResinLV,
                          false, 0, fCheckOverlaps);

        G4ThreeVector lDuctTlate = windowTlate;
        lDuctTlate.setZ(fLabZLength / 2. - fBeamductProtrusionLength / 2.);
        G4LogicalVolume *lDuctLV = BuildBeamductPipe("Lab", fBeamductProtrusionLength);
        new G4PVPlacement(nullptr, lDuctTlate, lDuctLV, "LabBeamductPV", labLV, false, 0,
                          fCheckOverlaps);

        G4ThreeVector lDuctEndplateTlate = lDuctTlate;
        lDuctEndplateTlate +=
            {0, 0, -fBeamductProtrusionLength / 2. - fBeamductEndplateThickness / 2.};
        G4Tubs *lDuctEndplateTubs = new G4Tubs("LabBeamductEndplateTub", 0, fBeamductEndplateRadius,
                                               fBeamductEndplateThickness / 2., 0, 360 * deg);
        G4LogicalVolume *lDuctEndplateLV =
            new G4LogicalVolume(lDuctEndplateTubs, matSS, "LabBeamductEndplateLV");
        new G4PVPlacement(nullptr, lDuctEndplateTlate, lDuctEndplateLV, "LabBeamductEndplatePV",
                          labLV, false, 0, fCheckOverlaps);

        workbenchCenter = PlaceWorkbench(labLV);
        PlaceRoomSlit(labLV, workbenchCenter);
        wbCenterOnBeamAxis = workbenchCenter;
        wbCenterOnBeamAxis.setX(windowTlate.getX());
        wbCenterOnBeamAxis.setY(windowTlate.getY());

        return labLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildBeamductPipe(const G4String &namePrefix,
                                                                 G4double length) const {
        G4LogicalVolume *ductLV =
            G4LogicalVolumeStore::GetInstance()->GetVolume(namePrefix + "BeamductPipeLV", false);
        if (ductLV != nullptr) return ductLV;

        G4Material *matSS  = G4Material::GetMaterial("Stainless_Steel");
        G4Material *matVac = G4Material::GetMaterial("G4_Galactic");

        G4Tubs *ductPipeTub = new G4Tubs(namePrefix + "BeamductPipeTub", 0, fBeamductPipeORadius,
                                         length / 2., 0, 360 * deg);
        ductLV = new G4LogicalVolume(ductPipeTub, matSS, namePrefix + "BeamductPipeLV");

        G4Tubs *ductVacuumTub = new G4Tubs(namePrefix + "BeamductVacuumTub", 0,
                                           fBeamductPipeIRadius, length / 2., 0, 360 * deg);
        G4LogicalVolume *ductVacuumLV =
            new G4LogicalVolume(ductVacuumTub, matVac, namePrefix + "BeamductVacuumLV");
        new G4PVPlacement(nullptr, {}, ductVacuumLV, namePrefix + "BeamductVacuumPV", ductLV, false,
                          0, fCheckOverlaps);

        return ductLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildRoomSlit(G4Material *aroundMaterial) const {
        G4Material *frameMaterial = G4Material::GetMaterial("Stainless_Steel");
        G4Material *slitMaterial  = G4Material::GetMaterial("G4_BORON_CARBIDE");

        G4Box *externalFrameBox = new G4Box("RoomSlitExternalFrameBox", fESlitFrameWidth / 2.,
                                            fESlitFrameHeight / 2., fSlitFrameZLength / 2.);
        G4LogicalVolume *externalFrameLV =
            new G4LogicalVolume(externalFrameBox, frameMaterial, "RoomSlitExternalFrameLV");

        G4Box *externalSpaceBox =
            new G4Box("RoomSlitExternalSpaceBox", fESlitFrameWidth / 2 - fESlitFrameThickness,
                      fESlitFrameHeight / 2. - fESlitFrameThickness, fSlitFrameZLength / 2.);
        G4LogicalVolume *externalSpaceLV =
            new G4LogicalVolume(externalSpaceBox, aroundMaterial, "RoomSlitExternalSpaceLV");

        new G4PVPlacement(nullptr, {}, externalSpaceLV, "RoomSlitExternalSpacePV", externalFrameLV,
                          false, 0, fCheckOverlaps);

        G4Box *internalFrameBox = new G4Box("RoomSlitInternalFrameBox", fISlitFrameWidth / 2.,
                                            fISlitFrameHeight / 2., fSlitFrameZLength / 2.);
        G4LogicalVolume *internalFrameLV =
            new G4LogicalVolume(internalFrameBox, frameMaterial, "RoomSlitInternalFrameLV");
        new G4PVPlacement(nullptr, {}, internalFrameLV, "RoomSlitInternalFramePV", externalSpaceLV,
                          false, 0, fCheckOverlaps);

        G4Box *internalSpaceBox =
            new G4Box("RoomSlitInternalSpaceBox", fISlitFrameWidth / 2. - fISlitFrameThickness,
                      fISlitFrameHeight / 2. - fISlitFrameThickness, fSlitFrameZLength / 2.);
        G4LogicalVolume *internalSpaceLV =
            new G4LogicalVolume(internalSpaceBox, aroundMaterial, "RoomSlitInternalSpaceLV");
        new G4PVPlacement(nullptr, {}, internalSpaceLV, "RoomSlitInternalSpacePV", internalFrameLV,
                          false, 0, fCheckOverlaps);

        G4double slitVBoxWidth  = fISlitFrameWidth - 2 * fISlitFrameThickness - fSlitVSpace;
        G4double slitHBoxHeight = fISlitFrameHeight - 2 * fISlitFrameThickness - fSlitHSpace;

        G4Box *slitVerticalBox =
            new G4Box("RoomSlitVerticalBox", slitVBoxWidth / 4.,
                      fISlitFrameHeight / 2. - fISlitFrameThickness, fSlitThickness / 2.);
        G4LogicalVolume *slitVerticalLV =
            new G4LogicalVolume(slitVerticalBox, slitMaterial, "RoomSlitVerticalLV");

        G4ThreeVector slitVTlatePosX = {fISlitFrameWidth / 2. - fISlitFrameThickness -
                                            slitVBoxWidth / 4.,
                                        0, fSlitThickness / 2. + fSlitHVZSpace / 2.};
        G4ThreeVector slitVTlateNegX = {-fISlitFrameWidth / 2. + fISlitFrameThickness +
                                            slitVBoxWidth / 4.,
                                        0, fSlitThickness / 2. + fSlitHVZSpace / 2.};

        new G4PVPlacement(nullptr, slitVTlatePosX, slitVerticalLV, "RoomSlitVerticalPV",
                          internalSpaceLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, slitVTlateNegX, slitVerticalLV, "RoomSlitVerticalPV",
                          internalSpaceLV, true, 1, fCheckOverlaps);

        G4Box *slitHorizontalBox =
            new G4Box("RoomSlitHorizontalBox", fISlitFrameWidth / 2. - fISlitFrameThickness,
                      slitHBoxHeight / 4., fSlitThickness / 2.);

        G4LogicalVolume *slitHorizontalLV =
            new G4LogicalVolume(slitHorizontalBox, slitMaterial, "RoomSlitHorizontalLV");

        G4ThreeVector slitHTlatePosX = {
            0, fISlitFrameHeight / 2. - fISlitFrameThickness - slitHBoxHeight / 4,
            -fSlitThickness / 2. - fSlitHVZSpace / 2.};
        G4ThreeVector slitHTlateNegX = {
            0, -fISlitFrameHeight / 2. + fISlitFrameThickness + slitHBoxHeight / 4,
            -fSlitThickness / 2. - fSlitHVZSpace / 2.};

        new G4PVPlacement(nullptr, slitHTlatePosX, slitHorizontalLV, "RoomSlitHorizontalPV",
                          internalSpaceLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, slitHTlateNegX, slitHorizontalLV, "RoomSlitHorizontalPV",
                          internalSpaceLV, true, 1, fCheckOverlaps);

        return externalFrameLV;
    }

    G4LogicalVolume *
        BL10DetectorConstruction::BuildRoomSlitStand(G4Material *aroundMaterial) const {
        G4Material *matFe = G4Material::GetMaterial("G4_Fe");

        G4Box *slitStandBox          = new G4Box("RoomSlitStandBox", fSlitStandWidth / 2.,
                                                 fSlitStandHeight / 2., fSlitStandZLength / 2.);
        G4LogicalVolume *slitStandLV = new G4LogicalVolume(slitStandBox, matFe, "RoomSlitStandLV");

        G4Box *slitStandSpaceBox =
            new G4Box("RoomSlitStandSpaceBox", fSlitStandWidth / 2. - fSlitStandThickness,
                      fSlitStandHeight / 2. - fSlitStandThickness,
                      fSlitStandZLength / 2. - fSlitStandThickness);

        G4LogicalVolume *slitStandSpaceLV =
            new G4LogicalVolume(slitStandSpaceBox, aroundMaterial, "RoomSlitStandSpaceLV");
        new G4PVPlacement(nullptr, {}, slitStandSpaceLV, "RoomSlitStandSpacePV", slitStandLV, false,
                          0, fCheckOverlaps);

        return slitStandLV;
    }
    G4LogicalVolume *
        BL10DetectorConstruction::BuildRoomSlitUpperStand(G4Material *aroundMaterial) const {
        G4Material *matSS = G4Material::GetMaterial("Stainless_Steel");

        G4Box *slitUpperStandBox = new G4Box("RoomSlitUpperStandBox", fSlitUStandWidth / 2.,
                                             fSlitUStandHeight / 2., fSlitUStandZLength / 2.);
        G4LogicalVolume *slitUpperStandLV =
            new G4LogicalVolume(slitUpperStandBox, matSS, "RoomSlitUpperStandLV");

        G4Box *slitUpperStandSpaceBox =
            new G4Box("RoomSlitUpperStandBox", fSlitUStandWidth / 2. - fSlitUStandThickness,
                      fSlitUStandHeight / 2. - fSlitUStandThickness,
                      fSlitUStandZLength / 2. - fSlitUStandThickness);

        G4LogicalVolume *slitUpperStandSpaceLV = new G4LogicalVolume(
            slitUpperStandSpaceBox, aroundMaterial, "RoomSlitUpperStandSpaceLV");
        new G4PVPlacement(nullptr, {}, slitUpperStandSpaceLV, "RoomSlitUpperStandPV",
                          slitUpperStandLV, false, 0, fCheckOverlaps);
        return slitUpperStandLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildWorkbench(G4Material *aroundMaterial) const {
        G4Material *matSS = G4Material::GetMaterial("Stainless_Steel");
        G4Material *matFe = G4Material::GetMaterial("G4_Fe");

        G4Box *envelopeBox = new G4Box("WorkbenchEnvelopeBox", ftWBEnvelopeWidth / 2.,
                                       ftWBEnvelopeHeight / 2., ftWBEnvelopeZLength / 2.);
        G4LogicalVolume *envelopeLV =
            new G4LogicalVolume(envelopeBox, aroundMaterial, "WorkbenchEnvelopeLV");

        G4Box *plateBox          = new G4Box("WBPlateBox", fWorkbenchPlateWidth / 2.,
                                             fWorkbenchPlateThickness / 2., fWorkbenchPlateLength / 2.);
        G4LogicalVolume *plateLV = new G4LogicalVolume(plateBox, matSS, "WBPlateLV");

        G4ThreeVector plateTlate = {0, 0, 0};
        // Move the plate to the +y-end of the envelope
        plateTlate += {0, ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness / 2., 0};
        // Move the plate in the -z-end of the envelope
        plateTlate += {0, 0, -ftWBEnvelopeZLength / 2. + fWorkbenchPlateLength / 2.};
        // Add +z margin for the plate
        plateTlate += {0, 0, fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin};
        // Move the plate in the -x-end of the envelope
        plateTlate += {-ftWBEnvelopeWidth / 2. + fWorkbenchPlateWidth / 2., 0, 0};
        // Add +x margin for the plate
        plateTlate += {fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin, 0, 0};
        new G4PVPlacement(nullptr, plateTlate, plateLV, "WBPlatePV", envelopeLV, false, 0,
                          fCheckOverlaps);

        G4ExtrudedSolid *xSupportHBeamSolid = new G4ExtrudedSolid(
            "WBXSupportHBeamSolid", ftHBeamPoints, fWorkbenchXSupportLength / 2.);
        G4ExtrudedSolid *zSupportHBeamSolid = new G4ExtrudedSolid(
            "WBZSupportHBeamSolid", ftHBeamPoints, fWorkbenchZSupportLength / 2.);

        G4LogicalVolume *xSupportHBeamLV =
            new G4LogicalVolume(xSupportHBeamSolid, matFe, "WBXSupportHBeamLV");
        G4LogicalVolume *zSupportHBeamLV =
            new G4LogicalVolume(zSupportHBeamSolid, matFe, "WBZSupportHBeamLV");

        G4ThreeVector beamsideYHBeamTlate = {
            0, ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness - fWorkbenchSupportHeight / 2.,
            -ftWBEnvelopeZLength / 2. + fWorkbenchSupportWidth / 2. + fWorkbenchPlateOuterMargin};
        G4ThreeVector dumpsideYHBeamTlate = beamsideYHBeamTlate;
        dumpsideYHBeamTlate[G4ThreeVector::Z] *= -1;

        G4ThreeVector xPlussideZHBeamTlate = {
            fWorkbenchXSupportLength / 2. + fWorkbenchSupportWidth / 2.,
            ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness - fWorkbenchSupportHeight / 2.,
            -ftWBEnvelopeZLength / 2. + fWorkbenchZSupportLength / 2. + fWorkbenchPlateOuterMargin};
        G4ThreeVector xMinussideZHBeamTlate = xPlussideZHBeamTlate;
        xMinussideZHBeamTlate[G4ThreeVector::X] *= -1;

        new G4PVPlacement(ftSimpleRotMtxY90Deg, beamsideYHBeamTlate, xSupportHBeamLV,
                          "WBXSupportHBeamPV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxY90Deg, dumpsideYHBeamTlate, xSupportHBeamLV,
                          "WBXSupportHBeamPV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(nullptr, xPlussideZHBeamTlate, zSupportHBeamLV, "WBZSuppoerHBeamPV",
                          envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, xMinussideZHBeamTlate, zSupportHBeamLV, "WBZSuppoerHBeamPV",
                          envelopeLV, true, 1, fCheckOverlaps);

        G4Box *wbPipeEndplate1Box =
            new G4Box("WBPipeEndplate1Box", fWBPipeEndplate1Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate1Length / 2.);
        G4Box *wbPipeEndplate2Box =
            new G4Box("WBPipeEndplate2Box", fWBPipeEndplate2Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate2Length / 2.);
        G4Box *wbPipeEndplate3Box =
            new G4Box("WBPipeEndplate3Box", fWBPipeEndplate3Width / 2.,
                      fWBPipeEndplateThickness / 2., fWBPipeEndplate3Length / 2.);

        G4LogicalVolume *wbPipeEndplate1LV =
            new G4LogicalVolume(wbPipeEndplate1Box, matFe, "WBEndplate1LV");
        G4LogicalVolume *wbPipeEndplate2LV =
            new G4LogicalVolume(wbPipeEndplate2Box, matFe, "WBEndplate2LV");
        G4LogicalVolume *wbPipeEndplate3LV =
            new G4LogicalVolume(wbPipeEndplate3Box, matFe, "WBEndplate3LV");

        G4ThreeVector vpTopEndplateXTlate = {-ftWBEnvelopeWidth / 2. + fWBPipeEndplate2Width / 2.,
                                             0, 0};
        G4ThreeVector vpTopEndplateYTlate = {0,
                                             ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness -
                                                 fWorkbenchSupportHeight -
                                                 fWBPipeEndplateThickness / 2.,
                                             0};
        G4ThreeVector vpTopEndplateBeamsideTlate = {
            0, 0, -ftWBEnvelopeZLength / 2. + fWBPipeEndplate2Length / 2.};
        G4ThreeVector vpTopEndplateDumpsideTlate = {
            0, 0, ftWBEnvelopeZLength / 2. - fWBPipeEndplate1Length / 2.};

        new G4PVPlacement(
            nullptr, vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateBeamsideTlate,
            wbPipeEndplate2LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateBeamsideTlate,
            wbPipeEndplate2LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);

        new G4PVPlacement(
            nullptr, vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -vpTopEndplateXTlate + vpTopEndplateYTlate + vpTopEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4Tubs *wbVertPipeTub =
            new G4Tubs("WBVertPipeTub", fWBVertPipeRadius - fWBVertPipeThickness, fWBVertPipeRadius,
                       fWBVertPipeLength / 2., 0, 360 * deg);
        G4LogicalVolume *wbVertPipeLV = new G4LogicalVolume(wbVertPipeTub, matFe, "WBVertPipeLV");

        G4ThreeVector vpXTlate = vpTopEndplateXTlate;
        G4ThreeVector vpYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplateThickness + fWBVertPipeLength / 2., 0};
        G4ThreeVector vpBeamsideTlate = {
            0, 0, -ftWBEnvelopeZLength / 2. + fWBPipeEndplate2Length - fWBPipeEndplate1Length / 2.};
        G4ThreeVector vpDumpsideTlate = {0, 0,
                                         ftWBEnvelopeZLength / 2. - fWBPipeEndplate1Length / 2.};

        new G4PVPlacement(ftSimpleRotMtxX90Deg, vpXTlate + vpYTlate + vpBeamsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg, -vpXTlate + vpYTlate + vpBeamsideTlate,
                          wbVertPipeLV, "WBVertPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg, vpXTlate + vpYTlate + vpDumpsideTlate, wbVertPipeLV,
                          "WBVertPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg, -vpXTlate + vpYTlate + vpDumpsideTlate,
                          wbVertPipeLV, "WBVertPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector bottomEndplateXTlate = vpTopEndplateXTlate;
        G4ThreeVector bottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplateThickness / 2., 0};
        G4ThreeVector bottomEndplateBeamsideTlate = vpBeamsideTlate;
        G4ThreeVector bottomEndplateDumpsideTlate = vpDumpsideTlate;

        new G4PVPlacement(
            nullptr, bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateBeamsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateBeamsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -bottomEndplateXTlate + bottomEndplateYTlate + bottomEndplateDumpsideTlate,
            wbPipeEndplate1LV, "WBVertPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4Tubs *wbZDiagPipeTub =
            new G4Tubs("WBZDiagPipeTub", fWBZDiagPipeRadius - fWBZDiagPipeThickness,
                       fWBZDiagPipeRadius, fWBZDiagPipeLength / 2., 0, 360 * deg);
        G4Tubs *wbXDiagPipeTub =
            new G4Tubs("WBXDiagPipeTub", fWBXDiagPipeRadius - fWBXDiagPipeThickness,
                       fWBXDiagPipeRadius, fWBXDiagPipeLength / 2., 0, 360 * deg);

        G4LogicalVolume *wbZDiagPipeLV =
            new G4LogicalVolume(wbZDiagPipeTub, matFe, "WBZDiagPipeLV");
        G4LogicalVolume *wbXDiagPipeLV =
            new G4LogicalVolume(wbXDiagPipeTub, matFe, "WBXDiagPipeLV");

        G4ThreeVector zdpBottomEndplateXTlate = vpTopEndplateXTlate;
        G4ThreeVector zdpBottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplate3Width / 2. + fWBZDiagPipeYDistance, 0};
        G4ThreeVector zdpBottomEndplateBeamsideTlate = vpBeamsideTlate;
        zdpBottomEndplateBeamsideTlate += {0, 0, fWBVertPipeRadius + fWBPipeEndplateThickness / 2.};
        G4ThreeVector zdpBottomEndplateDumpsideTlate = vpDumpsideTlate;
        zdpBottomEndplateDumpsideTlate +=
            {0, 0, -fWBVertPipeRadius - fWBPipeEndplateThickness / 2.};

        new G4PVPlacement(
            ftSimpleRotMtxX90Deg,
            zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            ftSimpleRotMtxX90Deg,
            -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            ftSimpleRotMtxX90Deg,
            zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            ftSimpleRotMtxX90Deg,
            -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate + zdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector zdpBeamsideTlate = {0, fWBZDiagPipeLength / 2. + fWBZDiagPipeRadius * 3, 0};
        G4ThreeVector zdpDumpsideTlate = {0, fWBZDiagPipeLength / 2. + fWBZDiagPipeRadius * 3, 0};
        zdpBeamsideTlate.rotateX(fWBZDiagPipeAngle);
        zdpDumpsideTlate.rotateX(-fWBZDiagPipeAngle);

        G4RotationMatrix *zdpBeamsideRotMtx = new G4RotationMatrix();
        zdpBeamsideRotMtx->rotateX(90 * deg - fWBZDiagPipeAngle);
        G4RotationMatrix *zdpDumpsideRotMtx = new G4RotationMatrix();
        zdpDumpsideRotMtx->rotateX(-(90 * deg - fWBZDiagPipeAngle));

        new G4PVPlacement(zdpBeamsideRotMtx,
                          zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateBeamsideTlate + zdpBeamsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(zdpBeamsideRotMtx,
                          -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateBeamsideTlate + zdpBeamsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(zdpDumpsideRotMtx,
                          zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateDumpsideTlate + zdpDumpsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(zdpDumpsideRotMtx,
                          -zdpBottomEndplateXTlate + zdpBottomEndplateYTlate +
                              zdpBottomEndplateDumpsideTlate + zdpDumpsideTlate,
                          wbZDiagPipeLV, "WBZDiagPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector zdpTopEndplateXTlate = zdpBottomEndplateXTlate;
        G4ThreeVector zdpTopEndplateYTlate = {0,
                                              ftWBEnvelopeHeight / 2. - fWorkbenchPlateThickness -
                                                  fWorkbenchSupportWidth -
                                                  fWBPipeEndplateThickness / 2.,
                                              0};

        G4double zdpBotEPDistFromVertTop =
            fWBVertPipeLength -
            (fWBZDiagPipeYDistance - fWBPipeEndplateThickness + fWBPipeEndplate3Width / 2.);

        G4ThreeVector zdpTopEndplateBeamsideTlate = vpBeamsideTlate;
        zdpTopEndplateBeamsideTlate += {0, 0, zdpBotEPDistFromVertTop * tan(fWBZDiagPipeAngle)};
        G4ThreeVector zdpTopEndplateDumpsideTlate = vpDumpsideTlate;
        zdpTopEndplateDumpsideTlate += {0, 0, -zdpBotEPDistFromVertTop * tan(fWBZDiagPipeAngle)};

        new G4PVPlacement(
            nullptr, zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -zdpTopEndplateXTlate + zdpTopEndplateYTlate + zdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBZDiagPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector xdpBottomEndplateXTlate = vpTopEndplateXTlate;
        xdpBottomEndplateXTlate += {fWBVertPipeRadius + fWBPipeEndplateThickness / 2., 0, 0};
        G4ThreeVector xdpBottomEndplateYTlate = {
            0, -ftWBEnvelopeHeight / 2. + fWBPipeEndplate3Width / 2. + fWBXDiagPipeYDistance, 0};
        G4ThreeVector xdpBottomEndplateBeamsideTlate = vpBeamsideTlate;
        G4ThreeVector xdpBottomEndplateDumpsideTlate = vpDumpsideTlate;

        G4RotationMatrix *xdpBottomRotMtx = new G4RotationMatrix();
        xdpBottomRotMtx->rotateZ(90 * deg);
        xdpBottomRotMtx->rotateY(90 * deg);

        new G4PVPlacement(
            xdpBottomRotMtx,
            xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            xdpBottomRotMtx,
            -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate + xdpBottomEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeBotEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        G4ThreeVector xdpXPlussideTlate = {0, fWBXDiagPipeLength / 2. + fWBXDiagPipeRadius * 3.5,
                                           0};
        xdpXPlussideTlate.rotateZ(-fWBXDiagPipeAngle);
        G4ThreeVector xdpXMinussideTlate = {0, fWBXDiagPipeLength / 2. + fWBXDiagPipeRadius * 3.5,
                                            0};
        xdpXMinussideTlate.rotateZ(fWBXDiagPipeAngle);

        G4RotationMatrix *xdpXPlussideRotMtx = new G4RotationMatrix();
        xdpXPlussideRotMtx->rotateX(90 * deg);
        xdpXPlussideRotMtx->rotateY(-fWBXDiagPipeAngle);
        G4RotationMatrix *xdpXMinussideRotMtx = new G4RotationMatrix();
        xdpXMinussideRotMtx->rotateX(90 * deg);
        xdpXMinussideRotMtx->rotateY(fWBXDiagPipeAngle);

        new G4PVPlacement(xdpXPlussideRotMtx,
                          xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateBeamsideTlate + xdpXPlussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(xdpXMinussideRotMtx,
                          -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateBeamsideTlate + xdpXMinussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(xdpXPlussideRotMtx,
                          xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateDumpsideTlate + xdpXPlussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(xdpXMinussideRotMtx,
                          -xdpBottomEndplateXTlate + xdpBottomEndplateYTlate +
                              xdpBottomEndplateDumpsideTlate + xdpXMinussideTlate,
                          wbXDiagPipeLV, "WBXDiagPipePV", envelopeLV, true, 3, fCheckOverlaps);

        G4double xdpBotEPDistFromVertTop =
            fWBVertPipeLength -
            (fWBXDiagPipeYDistance - fWBPipeEndplateThickness + fWBPipeEndplate3Width / 2.);

        G4ThreeVector xdpTopEndplateXTlate = vpXTlate;
        xdpTopEndplateXTlate += {xdpBotEPDistFromVertTop * tan(fWBXDiagPipeAngle), 0, 0};
        G4ThreeVector xdpTopEndplateYTlate = zdpTopEndplateYTlate;

        G4ThreeVector xdpTopEndplateBeamsideTlate = xdpBottomEndplateBeamsideTlate;
        G4ThreeVector xdpTopEndplateDumpsideTlate = xdpBottomEndplateDumpsideTlate;

        new G4PVPlacement(
            nullptr, xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateBeamsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(
            nullptr, -xdpTopEndplateXTlate + xdpTopEndplateYTlate + xdpTopEndplateDumpsideTlate,
            wbPipeEndplate3LV, "WBXDiagPipeTopEndplatePV", envelopeLV, true, 3, fCheckOverlaps);

        return envelopeLV;
    }

    G4ThreeVector BL10DetectorConstruction::PlaceWorkbench(G4LogicalVolume *labLV) const {
        G4Material *matFe = G4Material::GetMaterial("G4_Fe");

        G4LogicalVolume *wbLV = BuildWorkbench(labLV->GetMaterial());

        G4ThreeVector wbTlate = {0, 0, 0};

        // Move the workbench to the center of beamline
        wbTlate += {+fLabWidthBeamside / 2. - fBeamXDistanceFromWall, 0, 0};
        // Move the workbench to the earth
        wbTlate += {0, -fLabHeight / 2. + fFeFlooringThickness + ftWBEnvelopeHeight / 2., 0};
        // Move the workbench to the beamside wall of lab
        wbTlate += {0, 0, -fLabZLength / 2. + ftWBEnvelopeZLength / 2};
        // Move the workbench to match the distance from wall
        wbTlate += {0, 0, fWBZDistanceFromWall};

        // Move wb
        new G4PVPlacement(nullptr, wbTlate, wbLV, "WorkbenchPV", labLV, false, 0, fCheckOverlaps);

        G4ExtrudedSolid *wbBoltSolid = new G4ExtrudedSolid(
            "WBLevelingBoltSolid", ftLevelingBoltPoints, fWBLevelingBoltThickness / 2.);
        G4LogicalVolume *wbBoltLV = new G4LogicalVolume(wbBoltSolid, matFe, "WBLevelingBoltLV");

        G4ThreeVector wbBoltXTlate = {-ftWBEnvelopeWidth / 2. + fWBLevelingBoltSize / 2. +
                                          fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin,
                                      0, 0};
        G4ThreeVector wbBoltYTlate = {0, ftWBEnvelopeHeight / 2. + fWBLevelingBoltThickness / 2.,
                                      0};
        G4ThreeVector wbBoltZTlate = {0, 0,
                                      -ftWBEnvelopeZLength / 2. + fWBLevelingBoltSize / 2. +
                                          fWorkbenchPlateOuterMargin + fWorkbenchPlateInnerMargin};

        new G4PVPlacement(ftSimpleRotMtxX90Deg,
                          wbTlate + wbBoltXTlate + wbBoltYTlate + wbBoltZTlate, wbBoltLV,
                          "WBLevelingBoltPV", labLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg,
                          wbTlate - wbBoltXTlate + wbBoltYTlate + wbBoltZTlate, wbBoltLV,
                          "WBLevelingBoltPV", labLV, true, 1, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg,
                          wbTlate + wbBoltXTlate + wbBoltYTlate - wbBoltZTlate, wbBoltLV,
                          "WBLevelingBoltPV", labLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg,
                          wbTlate - wbBoltXTlate + wbBoltYTlate - wbBoltZTlate, wbBoltLV,
                          "WBLevelingBoltPV", labLV, true, 3, fCheckOverlaps);

        G4ThreeVector workbenchCenter{0, ftWBEnvelopeHeight / 2., 0};
        workbenchCenter += wbTlate;
        return workbenchCenter;
    }

    void BL10DetectorConstruction::PlaceRoomSlit(G4LogicalVolume *labLV,
                                                 const G4ThreeVector &smplPos) const {
        G4Material *labMaterial = labLV->GetMaterial();

        G4LogicalVolume *roomSlitLV       = BuildRoomSlit(labMaterial);
        G4LogicalVolume *slitStandLV      = BuildRoomSlitStand(labMaterial);
        G4LogicalVolume *slitUpperStandLV = BuildRoomSlitUpperStand(labMaterial);

        G4ThreeVector slitStandTlate = smplPos;
        slitStandTlate.setX(0);

        slitStandTlate += {fLabWidthBeamside / 2. - fBeamXDistanceFromWall, fSlitStandHeight / 2.,
                           -fWorkbenchPlateLength / 2. + fSlitStandZLength / 2.};
        new G4PVPlacement(nullptr, slitStandTlate, slitStandLV, "RoomSlitStandPV", labLV, false, 0,
                          fCheckOverlaps);

        G4ThreeVector roomSlitUpperStandTlate = slitStandTlate;
        roomSlitUpperStandTlate += {0, fSlitStandHeight / 2. + fSlitUStandHeight / 2., 0};
        new G4PVPlacement(nullptr, roomSlitUpperStandTlate, slitUpperStandLV,
                          "RoomSlitUpperStandPV", labLV, false, 0, fCheckOverlaps);

        G4ThreeVector roomSlitTlate = roomSlitUpperStandTlate;
        roomSlitTlate += {0, fSlitUStandHeight / 2. + fESlitFrameHeight / 2., 0};
        new G4PVPlacement(nullptr, roomSlitTlate, roomSlitLV, "RoomSlitPV", labLV, false, 0,
                          fCheckOverlaps);
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildJig(G4double jigLength, G4Material *jigMaterial,
                                                        G4Material *aroundMaterial) const {
        struct JigInformation {
            G4double fLength;
            G4Material *fMaterial;
            G4Material *fEnvelopeMaterial;
            bool operator<(const JigInformation &rhs) const {
                if (fLength != rhs.fLength)
                    return fLength < rhs.fLength;
                else if (fMaterial != rhs.fMaterial)
                    return fMaterial < rhs.fMaterial;
                else
                    return fEnvelopeMaterial < rhs.fEnvelopeMaterial;
            }
        };

        JigInformation input = {jigLength, jigMaterial, aroundMaterial};

        static std::map<JigInformation, G4LogicalVolume *> sJigLVCache;

        auto findres = sJigLVCache.find(input);

        if (findres != sJigLVCache.end()) return findres->second;

        const G4int sJigCount = sJigLVCache.size();

        G4String envSolidName   = "JigBox_";
        G4String spaceSolidName = "JigSpaceSolid_";
        G4String holeSolidName  = "JigHoleSolid_";
        G4String envLVName      = "JigLV_";
        G4String spaceLVName    = "JigSpaceLV_";
        G4String holeLVName     = "JigHoleLV_";

        envSolidName += std::to_string(sJigCount);
        envLVName += std::to_string(sJigCount);

        spaceSolidName += std::to_string(sJigCount);
        spaceLVName += std::to_string(sJigCount);

        holeSolidName += std::to_string(sJigCount);
        holeLVName += std::to_string(sJigCount);

        G4Box *jigBox = new G4Box(envSolidName, fJigVHSize / 2., fJigVHSize / 2., jigLength / 2.);
        G4Tubs *jigHole =
            new G4Tubs(holeSolidName, 0, fJigCenterHoleRadius, jigLength / 2., 0, 360 * deg);

        G4LogicalVolume *jigLV = new G4LogicalVolume(jigBox, input.fEnvelopeMaterial, envLVName);
        G4LogicalVolume *jigHoleLV =
            new G4LogicalVolume(jigHole, input.fEnvelopeMaterial, holeLVName);

        new G4PVPlacement(nullptr, {}, jigHoleLV, "JigHolePV", jigLV, false, 0, fCheckOverlaps);

        G4ExtrudedSolid *spaceSolid =
            new G4ExtrudedSolid(spaceSolidName, ftJigSpacePoints, jigLength / 2.);
        G4LogicalVolume *spaceLV = new G4LogicalVolume(spaceSolid, input.fMaterial, spaceLVName);

        G4ThreeVector spaceTlate = {0, -fJigVHSize / 2., 0};

        G4RotationMatrix *rotMtx1 = new G4RotationMatrix();
        G4RotationMatrix *rotMtx2 = new G4RotationMatrix();
        G4RotationMatrix *rotMtx3 = new G4RotationMatrix();
        rotMtx1->rotateZ(90 * deg);
        rotMtx2->rotateZ(180 * deg);
        rotMtx3->rotateZ(270 * deg);

        new G4PVPlacement(nullptr, spaceTlate, spaceLV, "JigSpacePV", jigLV, true, 0,
                          fCheckOverlaps);
        spaceTlate.rotateZ(-90 * deg);
        new G4PVPlacement(rotMtx1, spaceTlate, spaceLV, "JigSpacePV", jigLV, true, 1,
                          fCheckOverlaps);
        spaceTlate.rotateZ(-90 * deg);
        new G4PVPlacement(rotMtx2, spaceTlate, spaceLV, "JigSpacePV", jigLV, true, 2,
                          fCheckOverlaps);
        spaceTlate.rotateZ(-90 * deg);
        new G4PVPlacement(rotMtx3, spaceTlate, spaceLV, "JigSpacePV", jigLV, true, 3,
                          fCheckOverlaps);

        sJigLVCache[input] = jigLV;
        return jigLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildMkII(G4Material *aroundMaterial) const {
        G4LogicalVolume *envelopeLV =
            G4LogicalVolumeStore::GetInstance()->GetVolume("MkIIEnvelopeLV", false);

        if (envelopeLV != nullptr) {
            return envelopeLV;
        }

        G4Material *matPCB = G4Material::GetMaterial("EffectivePCB");
        G4Material *matFR4 = G4Material::GetMaterial("FR4");
        G4Material *matSi  = G4Material::GetMaterial("G4_Si");
        G4Material *matLid = G4Material::GetMaterial("FPGALid");

        G4double envelopeZLength = (fMkIIThickness + fMkIIFPGASubstrateThickness +
                                    fMkIIFPGADieThickness + fMkIIFPGALidThickness);

        G4Box *envelopeBox = new G4Box("MkIIEnvelopeBox", fMkIITopWidth / 2., fMkIILongHeight / 2.,
                                       envelopeZLength / 2.);
        envelopeLV         = new G4LogicalVolume(envelopeBox, aroundMaterial, "MkIIEnvelopeLV");
        envelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4ExtrudedSolid *pcbSolid =
            new G4ExtrudedSolid("MkIIPCBSolid", ftMkIIBoardPoints, fMkIIThickness / 2.);
        G4LogicalVolume *pcbLV = new G4LogicalVolume(pcbSolid, matPCB, "MkIIPCBLV");

        new G4PVPlacement(nullptr,
                          {0, +fMkIILongHeight / 2., -envelopeZLength / 2. + fMkIIThickness / 2.},
                          pcbLV, "MkIIPCBPV", envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaSubstrateBox =
            new G4Box("MkIIFPGASubstrateBox", fMkIIFPGASubstrateVHSize / 2.,
                      fMkIIFPGASubstrateVHSize / 2., fMkIIFPGASubstrateThickness / 2.);
        G4LogicalVolume *fpgaSubstrateLV =
            new G4LogicalVolume(fpgaSubstrateBox, matFR4, "MkIIFPGASubstrateLV");

        G4ThreeVector fpgaSubstrateTlate;
        fpgaSubstrateTlate = {fMkIITopWidth / 2. - fMkIIFPGASubstrateVHSize / 2.,
                              fMkIILongHeight / 2. - fMkIIFPGASubstrateVHSize / 2.,
                              -envelopeZLength / 2. + fMkIIThickness +
                                  fMkIIFPGASubstrateThickness / 2.};
        fpgaSubstrateTlate += {-fMkIIFPGAHSpace, -fMkIIFPGAVSpace, 0};
        new G4PVPlacement(nullptr, fpgaSubstrateTlate, fpgaSubstrateLV, "MkIIFPGASubstratePV",
                          envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaDieBox       = new G4Box("MkIIFPGADieBox", fMkIIFPGADieVHSize / 2.,
                                            fMkIIFPGADieVHSize / 2., fMkIIFPGADieThickness / 2.);
        G4LogicalVolume *fpgaLV = new G4LogicalVolume(fpgaDieBox, matSi, "MkIIFPGADieLV");

        G4ThreeVector fpgaDieTlate = fpgaSubstrateTlate;
        fpgaDieTlate += {0, 0, fMkIIFPGASubstrateThickness / 2. + fMkIIFPGADieThickness / 2.};
        new G4PVPlacement(nullptr, fpgaDieTlate, fpgaLV, "MkIIFPGADiePV", envelopeLV, false, 0,
                          fCheckOverlaps);

        G4Box *fpgaLidBox          = new G4Box("MkIIFPGALidBox", fMkIIFPGASubstrateVHSize / 2.,
                                               fMkIIFPGASubstrateVHSize / 2., fMkIIFPGALidThickness / 2.);
        G4LogicalVolume *fpgaLidLV = new G4LogicalVolume(fpgaLidBox, matLid, "MkIIFPGALidLV");

        G4ThreeVector fpgaLidTlate = fpgaDieTlate;
        fpgaLidTlate += {0, 0, fMkIIFPGADieThickness / 2. + fMkIIFPGALidThickness / 2.};
        new G4PVPlacement(nullptr, fpgaLidTlate, fpgaLidLV, "MkIIFPGALidPV", envelopeLV, false, 0,
                          fCheckOverlaps);

        return envelopeLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildRECBE(G4Material *aroundMaterial) const {
        G4LogicalVolume *envelopeLV =
            G4LogicalVolumeStore::GetInstance()->GetVolume("RECBEEnvelopeLV", false);

        if (envelopeLV != nullptr) {
            return envelopeLV;
        }

        G4Material *matPCB = G4Material::GetMaterial("EffectivePCB");
        G4Material *matFR4 = G4Material::GetMaterial("FR4");
        G4Material *matSi  = G4Material::GetMaterial("G4_Si");
        G4Material *matLid = G4Material::GetMaterial("FPGALid");

        G4double envelopeZLength = (fRECBEThickness + fRECBEFPGASubstrateThickness +
                                    fRECBEFPGADieThickness + fMkIIFPGALidThickness);

        G4Box *envelopeBox = new G4Box("RECBEEnvelopeBox", fRECBETopWidth / 2.,
                                       fRECBELongHeight / 2., envelopeZLength / 2.);
        envelopeLV         = new G4LogicalVolume(envelopeBox, aroundMaterial, "RECBEEnvelopeLV");
        envelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4ExtrudedSolid *pcbSolid =
            new G4ExtrudedSolid("RECBEPCBSolid", ftRECBEBoardPoints, fRECBEThickness / 2.);
        G4LogicalVolume *pcbLV = new G4LogicalVolume(pcbSolid, matPCB, "RECBEPCBLV");

        new G4PVPlacement(nullptr,
                          {0, +fRECBELongHeight / 2., -envelopeZLength / 2. + fRECBEThickness / 2.},
                          pcbLV, "RECBEPCBPV", envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaSubstrateBox =
            new G4Box("RECBEFPGASubstrateBox", fRECBEFPGASubstrateVHSize / 2.,
                      fRECBEFPGASubstrateVHSize / 2., fRECBEFPGASubstrateThickness / 2.);
        G4LogicalVolume *fpgaSubstrateLV =
            new G4LogicalVolume(fpgaSubstrateBox, matFR4, "RECBEFPGASubstrateLV");

        G4ThreeVector fpgaSubstrateTlate;
        fpgaSubstrateTlate = {fRECBETopWidth / 2. - fRECBEFPGASubstrateVHSize / 2.,
                              fRECBELongHeight / 2. - fRECBEFPGASubstrateVHSize / 2.,
                              -envelopeZLength / 2. + fRECBEThickness +
                                  fRECBEFPGASubstrateThickness / 2.};
        fpgaSubstrateTlate += {-fRECBEFPGAHSpace, -fRECBEFPGAVSpace, 0};
        new G4PVPlacement(nullptr, fpgaSubstrateTlate, fpgaSubstrateLV, "RECBEFPGASubstratePV",
                          envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaDieBox       = new G4Box("RECBEFPGADieBox", fRECBEFPGADieVHSize / 2.,
                                            fRECBEFPGADieVHSize / 2., fRECBEFPGADieThickness / 2.);
        G4LogicalVolume *fpgaLV = new G4LogicalVolume(fpgaDieBox, matSi, "RECBEFPGADieLV");

        G4ThreeVector fpgaDieTlate = fpgaSubstrateTlate;
        fpgaDieTlate += {0, 0, fRECBEFPGASubstrateThickness / 2. + fRECBEFPGADieThickness / 2.};
        new G4PVPlacement(nullptr, fpgaDieTlate, fpgaLV, "RECBEFPGADiePV", envelopeLV, false, 0,
                          fCheckOverlaps);

        G4Box *fpgaLidBox          = new G4Box("RECBEFPGALidBox", fRECBEFPGASubstrateVHSize / 2.,
                                               fRECBEFPGASubstrateVHSize / 2., fMkIIFPGALidThickness / 2.);
        G4LogicalVolume *fpgaLidLV = new G4LogicalVolume(fpgaLidBox, matLid, "RECBEFPGALidLV");

        G4ThreeVector fpgaLidTlate = fpgaDieTlate;
        fpgaLidTlate += {0, 0, fRECBEFPGADieThickness / 2. + fMkIIFPGALidThickness / 2.};
        new G4PVPlacement(nullptr, fpgaLidTlate, fpgaLidLV, "RECBEFPGALidPV", envelopeLV, false, 0,
                          fCheckOverlaps);
        return envelopeLV;
    }

    G4LogicalVolume *BL10DetectorConstruction::BuildROESTI(G4Material *aroundMaterial) const {
        G4LogicalVolume *envelopeLV =
            G4LogicalVolumeStore::GetInstance()->GetVolume("ROESTIEnvelopeLV", false);

        if (envelopeLV != nullptr) {
            return envelopeLV;
        }

        G4Material *matPCB = G4Material::GetMaterial("EffectivePCB");
        G4Material *matFR4 = G4Material::GetMaterial("FR4");
        G4Material *matSi  = G4Material::GetMaterial("G4_Si");

        G4double envelopeZLength = (fROESTIThickness + fROESTIFPGASubstrateThickness +
                                    fROESTIFPGADieThickness + fMkIIFPGALidThickness);

        G4Box *envelopeBox = new G4Box("ROESTIEnvelopeBox", fROESTIWidth / 2., fROESTIHeight / 2.,
                                       envelopeZLength / 2.);
        envelopeLV         = new G4LogicalVolume(envelopeBox, aroundMaterial, "ROESTIEnvelopeLV");
        envelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4Box *pcbBox =
            new G4Box("ROESTIPCBBox", fROESTIWidth / 2., fROESTIHeight / 2., fROESTIThickness / 2.);
        G4LogicalVolume *pcbLV = new G4LogicalVolume(pcbBox, matPCB, "ROESTIPCBLV");

        new G4PVPlacement(nullptr, {0, 0, -envelopeZLength / 2. + fROESTIThickness / 2.}, pcbLV,
                          "ROESTIPCBPV", envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaSubstrateBox =
            new G4Box("ROESTIFPGASubstrateBox", fROESTIFPGASubstrateVHSize / 2.,
                      fROESTIFPGASubstrateVHSize / 2., fROESTIFPGASubstrateThickness / 2.);
        G4LogicalVolume *fpgaSubstrateLV =
            new G4LogicalVolume(fpgaSubstrateBox, matFR4, "ROESTIFPGASubstrateLV");

        G4ThreeVector fpgaSubstrateTlate;
        fpgaSubstrateTlate = {fROESTIWidth / 2. - fROESTIFPGASubstrateVHSize / 2.,
                              fROESTIHeight / 2. - fROESTIFPGASubstrateVHSize / 2.,
                              -envelopeZLength / 2. + fROESTIThickness +
                                  fROESTIFPGASubstrateThickness / 2.};
        fpgaSubstrateTlate += {-fROESTIFPGAHSpace, -fROESTIFPGAVSpace, 0};
        new G4PVPlacement(nullptr, fpgaSubstrateTlate, fpgaSubstrateLV, "ROESTIFPGASubstratePV",
                          envelopeLV, false, 0, fCheckOverlaps);

        G4Box *fpgaDieBox       = new G4Box("ROESTIFPGADieBox", fROESTIFPGADieHSize / 2.,
                                            fROESTIFPGADieVSize / 2., fROESTIFPGADieThickness / 2.);
        G4LogicalVolume *fpgaLV = new G4LogicalVolume(fpgaDieBox, matSi, "ROESTIFPGADieLV");

        G4ThreeVector fpgaDieTlate = fpgaSubstrateTlate;
        fpgaDieTlate += {0, 0, fROESTIFPGASubstrateThickness / 2. + fROESTIFPGADieThickness / 2.};
        new G4PVPlacement(nullptr, fpgaDieTlate, fpgaLV, "ROESTIFPGADiePV", envelopeLV, false, 0,
                          fCheckOverlaps);

        return envelopeLV;
    }

    G4LogicalVolume *
        BL10DetectorConstruction::BuildFrameAndBoards(G4Material *aroundMaterial) const {
        G4LogicalVolume *fbeLV =
            G4LogicalVolumeStore::GetInstance()->GetVolume("FrameBoardsEnvelopeLV", false);
        if (fbeLV != nullptr) return fbeLV;

        G4Material *jigMaterial = G4Material::GetMaterial("Stainless_Steel");

        G4double envelopeHeight  = std::max(fVJigType1Length, fVJigType2Length) + fJigVHSize * 2;
        G4double envelopeWidth   = fFrameWidth;
        G4double envelopeZLength = fFrameLength;

        G4Box *envelopeBox = new G4Box("FrameBoardsEnvelopeBox", envelopeWidth / 2.,
                                       envelopeHeight / 2., envelopeZLength / 2.);
        G4LogicalVolume *envelopeLV =
            new G4LogicalVolume(envelopeBox, aroundMaterial, "FrameBoardsEnvelopeLV");
        envelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4LogicalVolume *baseJigLV = BuildJig(fFrameLength, jigMaterial, aroundMaterial);

        G4ThreeVector baseJig1Tlate{envelopeWidth / 2. - fJigVHSize / 2.,
                                    -envelopeHeight / 2. + fJigVHSize / 2., 0};
        G4ThreeVector baseJig2Tlate{-envelopeWidth / 2. + fJigVHSize / 2.,
                                    -envelopeHeight / 2. + fJigVHSize / 2., 0};

        new G4PVPlacement(nullptr, baseJig1Tlate, baseJigLV, "BaseJigPV", envelopeLV, true, 0,
                          fCheckOverlaps);
        new G4PVPlacement(nullptr, baseJig2Tlate, baseJigLV, "BaseJigPV", envelopeLV, true, 1,
                          fCheckOverlaps);

        G4double firstJigZPos = -envelopeZLength / 2. + fFirstJigZOffset;

        G4double zPosSum = 0;

        for (size_t i = 0; i < fBoardZSpaces.size() + 1; i++) {
            if (i != 0) zPosSum += fBoardZSpaces[i - 1];
            G4ThreeVector nowDisplace = {0, 0, zPosSum};
            FrameBoardComplexInfo nowFBCplx;
            switch (i % 3) {
                case 0:
                    nowFBCplx.fBoardName = "ROESTI";
                    nowFBCplx.fBoardLV   = BuildROESTI(aroundMaterial);
                    break;
                case 1:
                    nowFBCplx.fBoardName = "RECBE";
                    nowFBCplx.fBoardLV   = BuildRECBE(aroundMaterial);
                    break;
                case 2:
                    nowFBCplx.fBoardName = "MkII";
                    nowFBCplx.fBoardLV   = BuildMkII(aroundMaterial);
                    break;
            }
            nowFBCplx.fEnvelopeMaterial    = aroundMaterial;
            nowFBCplx.fNegativeXVJigLength = fXNegVJigLengths[i];
            nowFBCplx.fPositiveXVJigLength = fXPosVJigLengths[i];
            nowFBCplx.fBoardDistFromTop    = fBoardDistFromTop[i];
            nowFBCplx.fBoardHoriOffset     = fBoardHoriOffsets[i];
            nowFBCplx.fJigToBoardZSpace    = fJigToBoardZSpaces[i];
            nowFBCplx.fJigToBoardPosXSpace = fBracketBoardPosXMargins[i];
            nowFBCplx.fJigToBoardNegXSpace = fBracketBoardNegXMargins[i];
            nowFBCplx.fPosXAuxJigYOffset   = fPosXBracketYOffset[i];
            nowFBCplx.fNegXAuxJigYOffset   = fNegXBracketYOffset[i];
            nowFBCplx.fBoardCopyNo         = i / 3;

            G4LogicalVolume *nowFBCLV = BuildFrameBoardComplex(nowFBCplx);
            G4Box *nowFBCBox          = dynamic_cast<G4Box *>(nowFBCLV->GetSolid());
            if (nowFBCBox == nullptr) {
                G4ExceptionDescription msg;
                msg << "The type of envelope for the given geometry is not G4Box, which is not "
                       "supported.";
                G4Exception("BL10DetectorConstruction::BuildFrameAndBoards", "BL10GeometryE0010",
                            FatalException, msg);
                return nullptr;
            }
            G4double nowFBCHeight  = nowFBCBox->GetYHalfLength() * 2.;
            G4double nowFBCZLength = nowFBCBox->GetZHalfLength() * 2.;

            G4ThreeVector nowFBCTlate = {0, -envelopeHeight / 2. + fJigVHSize + nowFBCHeight / 2.,
                                         firstJigZPos + nowFBCZLength / 2. + zPosSum};

            new G4PVPlacement(nullptr, nowFBCTlate, nowFBCLV, nowFBCplx.GetComplexName() + "_PV",
                              envelopeLV, true, i / 3, fCheckOverlaps);
        }

        return envelopeLV;
    }

    G4LogicalVolume *
        BL10DetectorConstruction::BuildFrameBoardComplex(const FrameBoardComplexInfo &input) const {
        static std::map<FrameBoardComplexInfo, G4LogicalVolume *> sFBCplxLVCache;
        static G4LogicalVolume *sTriangleBracketLV = nullptr;
        static G4LogicalVolume *sLShapeBracketLV   = nullptr;

        auto findres = sFBCplxLVCache.find(input);
        if (findres != sFBCplxLVCache.end()) return findres->second;

        G4Box *boardEnvelopeBox = dynamic_cast<G4Box *>(input.fBoardLV->GetSolid());
        if (boardEnvelopeBox == nullptr) {
            G4ExceptionDescription msg;
            msg << "The type of envelope for the given geometry is not G4Box, which is not "
                   "supported.";
            G4Exception("BL10DetectorConstruction::BuildFrameBoardComplex", "BL10GeometryE0010",
                        FatalException, msg);
            return nullptr;
        }

        G4Material *jigMaterial = G4Material::GetMaterial("Stainless_Steel");

        if (sTriangleBracketLV == nullptr) {
            G4Box *virtBracketBox = new G4Box("TriangleBracketBox", fJigVHSize / 2.,
                                              fTriangleBracketSize / 2., fTriangleBracketSize / 2.);
            sTriangleBracketLV =
                new G4LogicalVolume(virtBracketBox, jigMaterial, "TriangleBracketLV");

            G4Box *sLShapeBracketBox =
                new G4Box("LShapeBracketBox", fLShapeBracketWidth / 2., fLShapeBracketHeight / 2.,
                          fLShapeBracketLength / 2.);
            sLShapeBracketLV =
                new G4LogicalVolume(sLShapeBracketBox, jigMaterial, "LShapceBracketLV");
        }

        G4int nowcnt            = 0;
        G4LogicalVolume *prevLV = nullptr;

        G4String cplxName = input.GetComplexName();
        while (true) {
            G4String nowLVName = cplxName;
            nowLVName += "_EnvelopeLV_";
            nowLVName += std::to_string(nowcnt);
            prevLV = G4LogicalVolumeStore::GetInstance()->GetVolume(nowLVName, false);
            if (prevLV == nullptr) {
                break;
            } else {
                nowcnt++;
            }
        }
        G4String postfix = "_";
        postfix += std::to_string(nowcnt);

        G4double boardWidth   = boardEnvelopeBox->GetXHalfLength() * 2;
        G4double boardHeight  = boardEnvelopeBox->GetYHalfLength() * 2.;
        G4double boardZLength = boardEnvelopeBox->GetZHalfLength() * 2.;

        G4double envelopeWidth = fFrameWidth;
        G4double envelopeHeight =
            std::max(input.fNegativeXVJigLength, input.fPositiveXVJigLength) + fJigVHSize;
        G4double envelopeZLength = fJigVHSize + input.fJigToBoardZSpace + boardZLength;

        G4Box *envelopeBox = new G4Box(cplxName + "_EnvelopeBox" + postfix, envelopeWidth / 2.,
                                       envelopeHeight / 2., envelopeZLength / 2.);
        G4LogicalVolume *envelopeLV = new G4LogicalVolume(envelopeBox, input.fEnvelopeMaterial,
                                                          cplxName + "_EnvelopeLV" + postfix);
        envelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        G4LogicalVolume *hJigLV = BuildJig(fFrameWidth, jigMaterial, input.fEnvelopeMaterial);

        G4ThreeVector hJigTlate = {0, -envelopeHeight / 2. + fJigVHSize / 2.,
                                   -envelopeZLength / 2. + fJigVHSize / 2.};

        new G4PVPlacement(ftSimpleRotMtxY90Deg, hJigTlate, hJigLV, "HorizontalJigPV", envelopeLV,
                          false, 0, fCheckOverlaps);

        G4ThreeVector hJigVBXDisplace         = {envelopeWidth / 2. - fJigVHSize / 2., 0, 0};
        G4ThreeVector hJigTriBracketBaseTlate = hJigTlate;

        hJigTriBracketBaseTlate += {0, -fJigVHSize / 2. + fTriangleBracketSize / 2.,
                                    fJigVHSize / 2. + fTriangleBracketSize / 2.};

        new G4PVPlacement(nullptr, hJigTriBracketBaseTlate + hJigVBXDisplace, sTriangleBracketLV,
                          "TriangleBracketPV", envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, hJigTriBracketBaseTlate - hJigVBXDisplace, sTriangleBracketLV,
                          "TriangleBracketPV", envelopeLV, true, 1, fCheckOverlaps);

        G4double shortJigTopYPos = -envelopeHeight / 2. + fJigVHSize;

        if (input.fPositiveXVJigLength < input.fNegativeXVJigLength) {
            shortJigTopYPos += input.fPositiveXVJigLength;
        } else {
            shortJigTopYPos += input.fNegativeXVJigLength;
        }

        G4ThreeVector boardTlate = {
            input.fBoardHoriOffset, shortJigTopYPos - boardHeight / 2. - input.fBoardDistFromTop,
            -envelopeZLength / 2. + fJigVHSize + input.fJigToBoardZSpace + boardZLength / 2.};

        new G4PVPlacement(nullptr, boardTlate, input.fBoardLV, input.fBoardName + "_PV", envelopeLV,
                          true, input.fBoardCopyNo, fCheckOverlaps);

        G4ThreeVector posLBracketTlate = boardTlate;
        G4ThreeVector negLBracketTlate = boardTlate;

        posLBracketTlate +=
            {boardWidth / 2. + input.fJigToBoardPosXSpace - fLShapeBracketWidth / 2.,
             boardHeight / 2. - fLShapeBracketHeight / 2.,
             -boardZLength / 2. - fLShapeBracketLength / 2.};
        negLBracketTlate +=
            {-boardWidth / 2. - input.fJigToBoardNegXSpace + fLShapeBracketWidth / 2.,
             boardHeight / 2. - fLShapeBracketHeight / 2.,
             -boardZLength / 2. - fLShapeBracketLength / 2.};

        new G4PVPlacement(nullptr, posLBracketTlate, sLShapeBracketLV, "LShapeBracketPV",
                          envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(nullptr, negLBracketTlate, sLShapeBracketLV, "LShapeBracketPV",
                          envelopeLV, true, 1, fCheckOverlaps);

        int nowLBCopyNum = 2;
        if (input.fPosXAuxJigYOffset != 0) {
            G4ThreeVector auxLBracketTlate = posLBracketTlate;
            auxLBracketTlate += {0, input.fPosXAuxJigYOffset, 0};
            new G4PVPlacement(nullptr, auxLBracketTlate, sLShapeBracketLV, "LShapeBracketPV",
                              envelopeLV, true, nowLBCopyNum++, fCheckOverlaps);
        }
        if (input.fNegXAuxJigYOffset != 0) {
            G4ThreeVector auxLBracketTlate = negLBracketTlate;
            auxLBracketTlate += {0, input.fNegXAuxJigYOffset, 0};
            new G4PVPlacement(nullptr, auxLBracketTlate, sLShapeBracketLV, "LShapeBracketPV",
                              envelopeLV, true, nowLBCopyNum++, fCheckOverlaps);
        }

        G4ThreeVector posXVJigTlate = boardTlate;
        G4ThreeVector negXVJigTlate = boardTlate;

        posXVJigTlate.setY(0);
        posXVJigTlate += {boardWidth / 2. + input.fJigToBoardPosXSpace + fJigVHSize / 2.,
                          -envelopeHeight / 2. + fJigVHSize + input.fPositiveXVJigLength / 2.,
                          -boardZLength / 2. - input.fJigToBoardZSpace - fJigVHSize / 2.};

        negXVJigTlate.setY(0);
        negXVJigTlate += {-boardWidth / 2. - input.fJigToBoardNegXSpace - fJigVHSize / 2.,
                          -envelopeHeight / 2. + fJigVHSize + input.fNegativeXVJigLength / 2.,
                          -boardZLength / 2. - input.fJigToBoardZSpace - fJigVHSize / 2.};

        G4LogicalVolume *posXVJigLV =
            BuildJig(input.fPositiveXVJigLength, jigMaterial, input.fEnvelopeMaterial);
        G4LogicalVolume *negXVJigLV =
            BuildJig(input.fNegativeXVJigLength, jigMaterial, input.fEnvelopeMaterial);

        new G4PVPlacement(ftSimpleRotMtxX90Deg, posXVJigTlate, posXVJigLV, "VerticalJigPV",
                          envelopeLV, true, 0, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxX90Deg, negXVJigTlate, negXVJigLV, "VerticalJigPV",
                          envelopeLV, true, 1, fCheckOverlaps);

        G4ThreeVector posXTriBracketTlate = posXVJigTlate;
        G4ThreeVector negXTriBracketTlate = negXVJigTlate;

        posXTriBracketTlate += {-fJigVHSize / 2. - fTriangleBracketSize / 2., 0, 0};
        posXTriBracketTlate += {0, -input.fPositiveXVJigLength / 2. + fTriangleBracketSize / 2., 0};

        negXTriBracketTlate += {fJigVHSize / 2. + fTriangleBracketSize / 2., 0, 0};
        negXTriBracketTlate += {0, -input.fNegativeXVJigLength / 2. + fTriangleBracketSize / 2., 0};

        new G4PVPlacement(ftSimpleRotMtxY90Deg, posXTriBracketTlate, sTriangleBracketLV,
                          "TriangleBracketPV", envelopeLV, true, 2, fCheckOverlaps);
        new G4PVPlacement(ftSimpleRotMtxY90Deg, negXTriBracketTlate, sTriangleBracketLV,
                          "TriangleBracketPV", envelopeLV, true, 3, fCheckOverlaps);

        sFBCplxLVCache[input] = envelopeLV;
        return envelopeLV;
    }

    G4VPhysicalVolume *BL10DetectorConstruction::DefineVolumes() {
        std::string geomType = "bl10_";
        if (fSimpleGeometry) {
            geomType += "simple";
        } else {
            geomType += "detailed";
        }
        simcore::MetadataManager::GetInstance().SetGeometryType(geomType);

        G4ThreeVector workbenchCenter, wbCenterOnBeamAxis;

        G4LogicalVolume *ironcaseLV   = BuildIroncase();
        G4VPhysicalVolume *ironcasePV = new G4PVPlacement(nullptr, {}, ironcaseLV, "WorldPV",
                                                          nullptr, false, 0, fCheckOverlaps);

        G4LogicalVolume *labLV =
            FillExperimentalRoom(ironcaseLV, workbenchCenter, wbCenterOnBeamAxis);

        G4Material *matFe       = G4Material::GetMaterial("G4_Fe");
        G4Material *jigMaterial = G4Material::GetMaterial("Stainless_Steel");
        G4Material *labMaterial = labLV->GetMaterial();

        G4LogicalVolume *expSetup = BuildFrameAndBoards(labMaterial);
        G4Box *expEnvBox          = dynamic_cast<G4Box *>(expSetup->GetSolid());
        if (expEnvBox == nullptr) {
            G4ExceptionDescription msg;
            msg << "The type of envelope for the given geometry is not G4Box, which is not "
                   "supported.";
            G4Exception("BL10DetectorConstruction::DefineVolumes", "BL10GeometryE0010",
                        FatalException, msg);
            return nullptr;
        }

        G4double setupEnvHeight  = expEnvBox->GetYHalfLength() * 2.;
        G4double setupEnvZLength = expEnvBox->GetZHalfLength() * 2.;

        G4ThreeVector lastFPGADieTlate = PrintLastFPGATlate(expSetup, "MkII", 2);

        G4Box *jackBox = new G4Box("JackBox", fJackWidth / 2., fJackHeight / 2., fJackZLength / 2.);
        G4LogicalVolume *jackLV = new G4LogicalVolume(jackBox, matFe, "JackLV");

        G4Box *jackISpaceBox =
            new G4Box("JackInnerSpaceBox", fJackWidth / 2. - fJackThickness,
                      fJackHeight / 2. - fJackThickness, fJackZLength / 2. - fJackThickness);
        G4LogicalVolume *jackISpaceLV =
            new G4LogicalVolume(jackISpaceBox, labMaterial, "JackInnerSpaceLV");
        new G4PVPlacement(nullptr, {}, jackISpaceLV, "JackInnerSpacePV", jackLV, false, 0,
                          fCheckOverlaps);

        G4ThreeVector setupTlate = workbenchCenter;
        setupTlate += {0, fJackHeight + setupEnvHeight / 2.,
                       -fLabZLength / 2. + setupEnvZLength / 2. + fStdSmplZPosFromBeamwall -
                           lastFPGADieTlate.getZ() + fLastFPGAZOffsetFromStdSmplPos};
        new G4PVPlacement(nullptr, setupTlate, expSetup, "ExperimentalSetupPV", labLV, false, 0,
                          fCheckOverlaps);

        G4LogicalVolume *baseRareJigLV = BuildJig(fRearFrameLength, jigMaterial, labMaterial);
        G4ThreeVector baseRareJigTlate = setupTlate;
        baseRareJigTlate +=
            {0, -setupEnvHeight / 2. + fJigVHSize / 2., setupEnvZLength / 2. + fJigVHSize / 2.};
        new G4PVPlacement(ftSimpleRotMtxY90Deg, baseRareJigTlate, baseRareJigLV, "BaseJigPV", labLV,
                          true, 2, fCheckOverlaps);

        G4ThreeVector jackTlate = setupTlate;
        jackTlate += {0, -setupEnvHeight / 2. - fJackHeight / 2., 0};
        jackTlate += {0, 0, setupEnvZLength / 2. + fJigVHSize - fJackZLength / 2.};
        new G4PVPlacement(nullptr, jackTlate, jackLV, "JackPV", labLV, false, 0, fCheckOverlaps);

        // PrintBoardAlignmentParameters(labLV);

        return ironcasePV;
    }

    void BL10DetectorConstruction::PlaceSimpleNeutronFluxDetectors(G4LogicalVolume *labLV) const {
        G4Material *labMaterial = labLV->GetMaterial();

        G4double detectorWidth     = 30 * cm;
        G4double detectorHeight    = 30 * cm;
        G4double detectorThickness = 1 * nm;
        G4double detectorSpacing   = 50 * cm;

        G4int detectorNum = 5;

        G4double envelopeWidth  = detectorWidth;
        G4double envelopeHeight = detectorHeight;
        G4double envelopeLength =
            detectorSpacing * (detectorNum - 1) + detectorThickness * detectorNum;

        G4Box *detectorEnvelopeBox = new G4Box("DetectorEnvelopeBox", envelopeWidth / 2.,
                                               envelopeHeight / 2., envelopeLength / 2.);
        G4LogicalVolume *detectorEnvelopeLV =
            new G4LogicalVolume(detectorEnvelopeBox, labMaterial, "DetectorEnvelopeLV");

        G4Box *detectorBox = new G4Box("DetectorBox", detectorWidth / 2., detectorHeight / 2.,
                                       detectorThickness / 2.);
        G4LogicalVolume *detectorLV = new G4LogicalVolume(detectorBox, labMaterial, "DetectorLV");
        for (int i = 0; i < detectorNum; i++) {
            new G4PVPlacement(
                nullptr,
                {0, 0, -envelopeLength / 2. + detectorThickness / 2. + detectorSpacing * i},
                detectorLV, "DetectorPV", detectorEnvelopeLV, true, i, fCheckOverlaps);
        }

        G4ThreeVector envelopeTlate;
        // Move the envelope solid next to the beam window (z-direction)
        envelopeTlate += {0, 0, -fLabZLength / 2. + fWindowThickness + envelopeLength / 2.};
        // Move the envelope solid to align to the center of beam window
        envelopeTlate +=
            {0, -fLabHeight / 2. + fBeamYDistanceFromFloor + fBeamWindowHeight / 2, 0.};
        envelopeTlate +=
            {fLabWidthBeamside / 2. - fBeamXDistanceFromWall - fBeamWindowWidth / 2., 0, 0};
        // Add the distance between the beam window and envelope
        envelopeTlate += {0, 0, detectorSpacing};

        new G4PVPlacement(nullptr, envelopeTlate, detectorEnvelopeLV, "DetectorEnvelopePV", labLV,
                          false, 0, fCheckOverlaps);
    }

    void BL10DetectorConstruction::ConstructSDandField() {
        G4String detectorSDName = "/FPGASD";

        simcore::TouchTriggerSD *ttsd = new simcore::TouchTriggerSD(detectorSDName);
        ttsd->SetRequireNonzeroEdep(true);

        G4SDManager::GetSDMpointer()->AddNewDetector(ttsd);
        FindLVAndAddSD("MkIIFPGADieLV", ttsd);
        FindLVAndAddSD("RECBEFPGADieLV", ttsd);
        FindLVAndAddSD("ROESTIFPGADieLV", ttsd);
    }

    bool BL10DetectorConstruction::FindLVAndAddSD(const G4String &name, G4VSensitiveDetector *sd) {
        G4LogicalVolume *testres = G4LogicalVolumeStore::GetInstance()->GetVolume(name);
        if (testres != nullptr) {
            SetSensitiveDetector(testres, sd);
            return true;
        } else {
            return false;
        }
    }
} // namespace bl10sim
