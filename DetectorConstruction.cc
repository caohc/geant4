#include "../include/DetectorConstruction.hh"
#include "../../../common/include/CalorimeterSD.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Torus.hh"
#include "G4Tubs.hh"
#include "G4Trap.hh"
#include "G4Sphere.hh"
#include "G4EllipticalTube.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"

#include "G4SDManager.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "math.h"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal 
G4GlobalMagFieldMessenger* DetectorConstruction::fMagFieldMessenger = 0; 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ 
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{

  // Define materials 
  DefineMaterials();
  
  // Define volumes
  return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{ 
  // Lead material defined using NIST Manager
  auto nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_Pb");
  nistManager->FindOrBuildMaterial("G4_POLYETHYLENE");
  
  // Liquid argon material
  G4double a;  // mass of a mole;
  G4double z;  // z=mean number of protons;  
  G4double density; 

  new G4Material("Galactic", z=1., a=1.01*g/mole,density= universe_mean_density,
                  kStateGas, 2.73*kelvin, 3.e-18*pascal);
  new G4Material("Rock", z=11., a= 22*g/mole, density= 2.85*g/cm3);


  /////////////////////////////////////////////////////////////////////////
  G4double ratio_Ar=1*0.05;
  G4double Argon_density = 6.7594*kg/m3;
  G4Element* Ar = new G4Element("Argon"  , "Ar", 18 , 39.948*g/mole);
  G4Material* ArgonGas = new G4Material("ArgonGas", Argon_density* ratio_Ar, 1,
                        kStateGas, 288.15*kelvin, 4*atmosphere*ratio_Ar);
  ArgonGas->AddElement(Ar, 1);

  /////////////////////////////////////////////////////////////////////////
  G4Isotope* He3 = new G4Isotope("He3", 2, 3);
  G4Element* Helium3  = new G4Element("Helium3", "Helium3", 1);
  Helium3->AddIsotope(He3, 100*perCent);

  G4double ratio_He = 1 - ratio_Ar;

  G4double He3_density   = 0.5132*kg/m3;
  G4Material* Helium3Gas = new G4Material("Helium3Gas", He3_density*ratio_He, 1,
                        kStateGas, 288.15*kelvin, 4*atmosphere*ratio_He);
  Helium3Gas->AddElement(Helium3, 1);

  /////////////////////////////////////////////////////////////////////////
  G4double Mix_density = He3_density*ratio_He + Argon_density*ratio_Ar;
  G4Material* MixGas = new G4Material("MixGas", Mix_density, 2);
  MixGas -> AddMaterial(Helium3Gas, ratio_He);
  MixGas -> AddMaterial(ArgonGas,   ratio_Ar);

  /////////////////////////////////////////////////////////////////////////

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4VPhysicalVolume* DetectorConstruction::DefineVolumes()
{

  G4Material* WorldMaterial = G4Material::GetMaterial("Galactic");
  G4Material* LeadMaterial  = G4Material::GetMaterial("G4_Pb");
  G4Material* PolyMaterial  = G4Material::GetMaterial("G4_POLYETHYLENE");
  //G4Material* RockMaterial = G4Material::GetMaterial("Sodium");
  G4Material* RockMaterial  = G4Material::GetMaterial("Rock");
  G4Material* MixMaterial   = G4Material::GetMaterial("MixGas");

////////////////////////////////////////////////////////////////////////

  //     
  // World
  //

  G4Box* worldS = new G4Box("World", worldSizeX/2, worldSizeY/2, worldSizeZ/2);
  G4LogicalVolume* worldLV = new G4LogicalVolume(worldS, WorldMaterial, "World");
 
  G4PVPlacement* worldPV
    = new G4PVPlacement(
                 0,
                 G4ThreeVector(0, 0, 0),
                 worldLV,
                 "WorldPV",
                 0,
                 false,
                 0,
                 true);

////////////////////////////////////////////////////////////////////////

  //     
  // Rock
  //

  G4Box* RockBlock = new G4Box("RockBlock", rockSizeX/2, rockSizeY/2, rockSizeZ/2);
  G4Box* Room = new G4Box("Room", roomSizeX/2, roomSizeY/2, roomSizeZ/2);
  G4SubtractionSolid* Rock = new G4SubtractionSolid("Rock", RockBlock, Room, 0, G4ThreeVector(0,0,0));


  G4LogicalVolume* rockLV = new G4LogicalVolume(Rock, RockMaterial, "RockLV");

  rockPV
    = new G4PVPlacement(
                 0,
                 G4ThreeVector(0, 0, 0),
                 rockLV,
                 "rockPV",
                 worldLV,
                 false,
                 0,
                 false);

////////////////////////////////////////////////////////////////////////

 G4double He_R=1.55*cm/2;
 G4double He_L=30*cm;
 G4EllipticalTube* HeS = new G4EllipticalTube("HeS", He_R, He_R, He_L/2);

 G4LogicalVolume* HeCounter_LV = new G4LogicalVolume(HeS, MixMaterial, "HeCounter_LV");


////////////////////////////////////////////////////////////////////////

  G4double PolyA = 60*cm;
  G4double PolyT = 15*cm;

  G4Box* PolySide_Box = new G4Box("PolySide_Box", PolyT/2, LeadL/2, LeadL/2);
  G4SubtractionSolid* PolySide_Box1 = new G4SubtractionSolid("PolySide_Box1", PolySide_Box, HeS, 0, G4ThreeVector(-5*cm, 12.5*cm,0));
  G4SubtractionSolid* PolySide_Box2 = new G4SubtractionSolid("PolySide_Box2", PolySide_Box1, HeS, 0, G4ThreeVector(0, 12.5*cm,0));
  G4SubtractionSolid* PolySide_Box3 = new G4SubtractionSolid("PolySide_Box3", PolySide_Box2, HeS, 0, G4ThreeVector(0, 7.5*cm,0));
  G4SubtractionSolid* PolySide_Box4 = new G4SubtractionSolid("PolySide_Box4", PolySide_Box3, HeS, 0, G4ThreeVector(0, 2.5*cm,0));
  G4SubtractionSolid* PolySide_Box5 = new G4SubtractionSolid("PolySide_Box5", PolySide_Box4, HeS, 0, G4ThreeVector(0, -2.5*cm,0));
  G4SubtractionSolid* PolySide_Box6 = new G4SubtractionSolid("PolySide_Box6", PolySide_Box5, HeS, 0, G4ThreeVector(0, -7.5*cm,0));
  G4SubtractionSolid* PolySide_Box7 = new G4SubtractionSolid("PolySide_Box7", PolySide_Box6, HeS, 0, G4ThreeVector(0, -12.5*cm,0));
  G4SubtractionSolid* PolyLR_S = new G4SubtractionSolid("PolyLR_S", PolySide_Box7, HeS, 0, G4ThreeVector(-5*cm, -12.5*cm,0));

  G4LogicalVolume* PolySide_LV = new G4LogicalVolume(PolyLR_S, PolyMaterial, "PolySide_LV");

  G4Box* PolyUD_Box = new G4Box("PolyUD_Box", PolyA/2, PolyT/2, PolyA/2);
  G4SubtractionSolid* PolyUD_Box1 = new G4SubtractionSolid("PolyUD_Box1", PolyUD_Box, HeS, 0, G4ThreeVector(-22.5*cm, -5*cm, 0));
  G4SubtractionSolid* PolyUD_Box2 = new G4SubtractionSolid("PolyUD_Box2", PolyUD_Box1, HeS, 0, G4ThreeVector(-17.5*cm, -5*cm, 0));
  G4SubtractionSolid* PolyUD_Box3 = new G4SubtractionSolid("PolyUD_Box3", PolyUD_Box2, HeS, 0, G4ThreeVector(-17.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box4 = new G4SubtractionSolid("PolyUD_Box4", PolyUD_Box3, HeS, 0, G4ThreeVector(-12.5*cm, -5*cm, 0));
  G4SubtractionSolid* PolyUD_Box5 = new G4SubtractionSolid("PolyUD_Box5", PolyUD_Box4, HeS, 0, G4ThreeVector(-12.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box6 = new G4SubtractionSolid("PolyUD_Box6", PolyUD_Box5, HeS, 0, G4ThreeVector(-7.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box7 = new G4SubtractionSolid("PolyUD_Box7", PolyUD_Box6, HeS, 0, G4ThreeVector(-2.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box8 = new G4SubtractionSolid("PolyUD_Box8", PolyUD_Box7, HeS, 0, G4ThreeVector(2.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box9 = new G4SubtractionSolid("PolyUD_Box9", PolyUD_Box8, HeS, 0, G4ThreeVector(7.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box10 = new G4SubtractionSolid("PolyUD_Box10", PolyUD_Box9, HeS, 0, G4ThreeVector(12.5*cm, -5*cm, 0));
  G4SubtractionSolid* PolyUD_Box11 = new G4SubtractionSolid("PolyUD_Box11", PolyUD_Box10, HeS, 0, G4ThreeVector(12.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_Box12 = new G4SubtractionSolid("PolyUD_Box12", PolyUD_Box11, HeS, 0, G4ThreeVector(17.5*cm, -5*cm, 0));
  G4SubtractionSolid* PolyUD_Box13 = new G4SubtractionSolid("PolyUD_Box13", PolyUD_Box12, HeS, 0, G4ThreeVector(17.5*cm, 0, 0));
  G4SubtractionSolid* PolyUD_S = new G4SubtractionSolid("PolyUD_S", PolyUD_Box13, HeS, 0, G4ThreeVector(22.5*cm, -5*cm, 0));

  G4LogicalVolume* PolyUD_LV = new G4LogicalVolume(PolyUD_S, PolyMaterial, "PolySide_LV");


////////////////////////////////////////////////////////////////////////

  //     
  // Target
  //

  G4Box* Target = new G4Box("Target", LeadL/2, LeadL/2, LeadL/2);
  G4LogicalVolume* TargetLV = new G4LogicalVolume(Target, LeadMaterial, "TargetLV");

  VD[21] =
  new G4PVPlacement(
                 0,
                 G4ThreeVector(0., 0., Lead_TargetZ),
                 TargetLV,
                 "TargetPV",
                 worldLV,
                 false,
                 0,
                 false);



/////////////////////////////////////////////////////////////////
  G4RotationMatrix* rot1 = new G4RotationMatrix();
  rot1->rotateY(0*deg);

  PolyR_PV = new G4PVPlacement
                (rot1,
                 G4ThreeVector( (LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ),
                 PolySide_LV,
                 "PolyR_PV",
                 worldLV,
                 false,
                 0,
                 false);


/////////////////////////////////////////////////////////////////
  G4RotationMatrix* rot2 = new G4RotationMatrix();
  rot2->rotateY(90*deg);

  PolyB_PV = new G4PVPlacement
                (rot2,
                 G4ThreeVector( 0, 0, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 PolySide_LV,
                 "PolyB_PV",
                 worldLV,
                 false,
                 0,
                 false);

/////////////////////////////////////////////////////////////////

  G4RotationMatrix* rot3 = new G4RotationMatrix();
  rot3->rotateY(180*deg);

  PolyL_PV = new G4PVPlacement
                (rot3,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ),
                 PolySide_LV,
                 "PolyL_PV",
                 worldLV,
                 false,
                 0,
                 false);

/////////////////////////////////////////////////////////////////

  G4RotationMatrix* rot4 = new G4RotationMatrix();
  rot4->rotateY(270*deg);

  PolyF_PV = new G4PVPlacement
                (rot4,
                 G4ThreeVector(0, 0, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 PolySide_LV,
                 "PolyF_PV",
                 worldLV,
                 false,
                 0,
                 false);

/////////////////////////////////////////////////////////////////

  G4RotationMatrix* rot5 = new G4RotationMatrix();
  rot5->rotateX(0*deg);
  PolyU_PV = new G4PVPlacement
                (rot5,
                 G4ThreeVector(0, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 PolyUD_LV,
                 "PolyU_PV",
                 worldLV,
                 false,
                 0,
                 false);

/////////////////////////////////////////////////////////////////

  G4RotationMatrix* rot6 = new G4RotationMatrix();
  rot6->rotateX(180*deg);
  PolyD_PV = new G4PVPlacement
                 (rot6,
                 G4ThreeVector(0, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 PolyUD_LV,
                 "PolyU_PV",
                 worldLV,
                 false,
                 0,
                 false);


/////////////////////////////////////////////////////////////////

  G4Box* PolyCornerS = new G4Box("PolyCornerS", PolyT/2, LeadL/2, PolyT/2);
  G4LogicalVolume* PolyCornerLV = new G4LogicalVolume(PolyCornerS, PolyMaterial, "PolyCornerLV");

  PolyCornerPV1 = new G4PVPlacement
                (0,
                 G4ThreeVector( -(LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 PolyCornerLV,
                 "PolyCornerPV1",
                 worldLV,
                 false,
                 0,
                 false);

  PolyCornerPV2 = new G4PVPlacement
                (0,
                 G4ThreeVector( (LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 PolyCornerLV,
                 "PolyCornerPV2",
                 worldLV,
                 false,
                 0,
                 false);

  PolyCornerPV3 = new G4PVPlacement
                (0,
                 G4ThreeVector( (LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 PolyCornerLV,
                 "PolyCornerPV3",
                 worldLV,
                 false,
                 0,
                 false);

  PolyCornerPV4 = new G4PVPlacement
                (0,
                 G4ThreeVector( -(LeadL+PolyT+2*VDt)/2, 0, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 PolyCornerLV,
                 "PolyCornerPV4",
                 worldLV,
                 false,
                 0,
                 false);

/////////////////////////////////////////////////////////////////

  HeCounter_PV[0] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2 - 5*cm, 12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV0",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[1] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, 12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV1",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[2] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, 7.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV2",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[3] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, 2.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV3",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[4] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, -2.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV4",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[5] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, -7.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV5",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[6] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2, -12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV6",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[7] = new G4PVPlacement
                (0,
                 G4ThreeVector((LeadL+PolyT+2*VDt)/2 - 5*cm, -12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV7",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[8] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 12.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2 -5*cm),
                 HeCounter_LV,
                 "HeCounter_PV8",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[9] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 12.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV9",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[10] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 7.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV10",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[11] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 2.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV11",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[12] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -2.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV12",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[13] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -7.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV13",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[14] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -12.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV14",
                 worldLV,
                 false,
                 0,
                 false);
                                
  HeCounter_PV[15] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -12.5*cm, Lead_TargetZ + (LeadL+PolyT+2*VDt)/2 - 5*cm),
                 HeCounter_LV,
                 "HeCounter_PV15",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[16] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2 + 5*cm, 12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV16",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[17] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, 12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV17",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[18] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, 7.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV18",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[19] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, 2.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV19",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[20] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, -2.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV20",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[21] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, -7.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV21",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[22] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2, -12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV22",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[23] = new G4PVPlacement
                (0,
                 G4ThreeVector(-(LeadL+PolyT+2*VDt)/2 + 5*cm, -12.5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV23",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[24] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 12.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2 +5*cm),
                 HeCounter_LV,
                 "HeCounter_PV24",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[25] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 12.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV25",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[26] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 7.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV26",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[27] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, 2.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV27",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[28] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -2.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV28",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[29] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -7.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV29",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[30] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -12.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2),
                 HeCounter_LV,
                 "HeCounter_PV30",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[31] = new G4PVPlacement
                (rot2,
                 G4ThreeVector(0, -12.5*cm, Lead_TargetZ - (LeadL+PolyT+2*VDt)/2 + 5*cm),
                 HeCounter_LV,
                 "HeCounter_PV31",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[32] = new G4PVPlacement
                (0,
                 G4ThreeVector(-22.5*cm, (LeadL+PolyT+2*VDt)/2 -5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV32",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[33] = new G4PVPlacement
                (0,
                 G4ThreeVector(-17.5*cm, (LeadL+PolyT+2*VDt)/2 -5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV33",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[34] = new G4PVPlacement
                (0,
                 G4ThreeVector(-17.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV34",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[35] = new G4PVPlacement
                (0,
                 G4ThreeVector(-12.5*cm, (LeadL+PolyT+2*VDt)/2 -5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV35",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[36] = new G4PVPlacement
                (0,
                 G4ThreeVector(-12.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV36",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[37] = new G4PVPlacement
                (0,
                 G4ThreeVector(-7.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV37",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[38] = new G4PVPlacement
                (0,
                 G4ThreeVector(-2.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV38",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[39] = new G4PVPlacement
                (0,
                 G4ThreeVector(2.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV39",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[40] = new G4PVPlacement
                (0,
                 G4ThreeVector(7.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV40",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[41] = new G4PVPlacement
                (0,
                 G4ThreeVector(12.5*cm, (LeadL+PolyT+2*VDt)/2 - 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV41",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[42] = new G4PVPlacement
                (0,
                 G4ThreeVector(12.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV42",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[43] = new G4PVPlacement
                (0,
                 G4ThreeVector(17.5*cm, (LeadL+PolyT+2*VDt)/2 - 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV43",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[44] = new G4PVPlacement
                (0,
                 G4ThreeVector(17.5*cm, (LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV44",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[45] = new G4PVPlacement
                (0,
                 G4ThreeVector(22.5*cm, (LeadL+PolyT+2*VDt)/2 - 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV45",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[46] = new G4PVPlacement
                (0,
                 G4ThreeVector(-22.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV46",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[47] = new G4PVPlacement
                (0,
                 G4ThreeVector(-17.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV47",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[48] = new G4PVPlacement
                (0,
                 G4ThreeVector(-17.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV48",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[49] = new G4PVPlacement
                (0,
                 G4ThreeVector(-12.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV49",
                 worldLV,
                 false,
                 0,
                 false);
                                 
  HeCounter_PV[50] = new G4PVPlacement
                (0,
                 G4ThreeVector(-12.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV50",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[51] = new G4PVPlacement
                (0,
                 G4ThreeVector(-7.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV51",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[52] = new G4PVPlacement
                (0,
                 G4ThreeVector(-2.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV52",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[53] = new G4PVPlacement
                (0,
                 G4ThreeVector(2.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV53",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[54] = new G4PVPlacement
                (0,
                 G4ThreeVector(7.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV54",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[55] = new G4PVPlacement
                (0,
                 G4ThreeVector(12.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV55",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[56] = new G4PVPlacement
                (0,
                 G4ThreeVector(12.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV56",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[57] = new G4PVPlacement
                (0,
                 G4ThreeVector(17.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV57",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[58] = new G4PVPlacement
                (0,
                 G4ThreeVector(17.5*cm, -(LeadL+PolyT+2*VDt)/2, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV58",
                 worldLV,
                 false,
                 0,
                 false);

  HeCounter_PV[59] = new G4PVPlacement
                (0,
                 G4ThreeVector(22.5*cm, -(LeadL+PolyT+2*VDt)/2 + 5*cm, Lead_TargetZ),
                 HeCounter_LV,
                 "HeCounter_PV59",
                 worldLV,
                 false,
                 0,
                 false);
                             

////////////////////////////////////////////////////////////////////////

  //     
  // VD0
  //


  G4Box* VD0 = new G4Box("VD0", worldSizeX/2, worldSizeY/2, VDt/2);
  G4LogicalVolume* VD0LV = new G4LogicalVolume(VD0, WorldMaterial, "VD0LV");

   VD[1] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, -rockSizeZ/2 - VDt/2),
                 VD0LV,
                 "VD0PV",
                 worldLV,
                 false,
                 0,
                 false);


////////////////////////////////////////////////////////////////////////

  //     
  // VD1
  //

  G4Box* VD1 = new G4Box("VD1", roomSizeX/4, roomSizeY/4, VDt/2);
  G4LogicalVolume* VD1LV = new G4LogicalVolume(VD1, WorldMaterial, "VD1LV");

   VD[2] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, Lead_TargetZ - Plastic_t - LeadL - VDt/2),
                 VD1LV,
                 "VD1PV",
                 worldLV,
                 false,
                 0,
                 false);



//////////////////////////////////////////
  G4Box* VD_Z = new G4Box("VD_Z", roomSizeX/2, roomSizeY/2, VDt/2);
  G4LogicalVolume* VD_ZLV = new G4LogicalVolume(VD_Z, WorldMaterial, "VD_ZLV");

   VD[3] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, -roomSizeZ/2 + VDt/2),
                 VD_ZLV,
                 "VD_uPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
   VD[4] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, roomSizeZ/2 - VDt/2),
                 VD_ZLV,
                 "VD_dPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VD_Y = new G4Box("VD_Y", roomSizeX/2, VDt/2, roomSizeZ/2);
  G4LogicalVolume* VD_YLV = new G4LogicalVolume(VD_Y, WorldMaterial, "VD_YLV");

  VD[5] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, -roomSizeY/2 + VDt/2, 0),
                 VD_YLV,
                 "VD_fPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  VD[6] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, roomSizeY/2 - VDt/2, 0),
                 VD_YLV,
                 "VD_bPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VD_X = new G4Box("VD_X",  VDt/2, roomSizeY/2, roomSizeZ/2);
  G4LogicalVolume* VD_XLV = new G4LogicalVolume(VD_X, WorldMaterial, "VD_XLV");

  VD[7] = new G4PVPlacement
                (0,
                 G4ThreeVector(-roomSizeX/2 + VDt/2, 0, 0),
                 VD_XLV,
                 "VD_lPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  VD[8] = new G4PVPlacement
                (0,
                 G4ThreeVector(roomSizeX/2 - VDt/2, 0, 0),
                 VD_XLV,
                 "VD_rPV",
                 worldLV,
                 false,
                 0,
                 false);


//////////////////////////////////////////
  G4Box* VDtarget_Z = new G4Box("VDtarget_Z", LeadL/2, LeadL/2, VDt/2);
  G4LogicalVolume* VDtarget_ZLV = new G4LogicalVolume(VDtarget_Z, WorldMaterial, "VDtarget_ZLV");

   VD[9] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, Lead_TargetZ - LeadL/2 - VDt/2),
                 VDtarget_ZLV,
                 "VDtarget_uPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[10] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, Lead_TargetZ + LeadL/2 + VDt/2),
                 VDtarget_ZLV,
                 "VDtarget_dPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VDtarget_Y = new G4Box("VDtarget_Y", LeadL/2, VDt/2, LeadL/2);
  G4LogicalVolume* VDtarget_YLV = new G4LogicalVolume(VDtarget_Y, WorldMaterial, "VDtarget_YLV");

   VD[11] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, -LeadL/2 - VDt/2, Lead_TargetZ),
                 VDtarget_YLV,
                 "VDtarget_fPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[12] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, LeadL/2 + VDt/2, Lead_TargetZ),
                 VDtarget_YLV,
                 "VDtarget_bPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VDtarget_X = new G4Box("VDtarget_X", VDt/2, LeadL/2, LeadL/2);
  G4LogicalVolume* VDtarget_XLV = new G4LogicalVolume(VDtarget_X, WorldMaterial, "VDtarget_XLV");

   VD[13] = new G4PVPlacement
                (0,
                 G4ThreeVector(-LeadL/2 - VDt/2, 0, Lead_TargetZ),
                 VDtarget_XLV,
                 "VDtarget_lPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[14] = new G4PVPlacement
                (0,
                 G4ThreeVector(LeadL/2 + VDt/2, 0, Lead_TargetZ),
                 VDtarget_XLV,
                 "VDtarget_rPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VDbox_Z = new G4Box("VDbox_Z", PolyA/2, PolyA/2, VDt/2);
  G4LogicalVolume* VDbox_ZLV = new G4LogicalVolume(VDbox_Z, WorldMaterial, "VDbox_ZLV");

   VD[15] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, Lead_TargetZ - PolyA/2 - 3*VDt/2),
                 VDbox_ZLV,
                 "VDbox_uPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[16] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, 0, Lead_TargetZ + PolyA/2 + 3*VDt/2),
                 VDbox_ZLV,
                 "VDbox_dPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VDbox_Y = new G4Box("VDbox_Y", PolyA/2, VDt/2, PolyA/2);
  G4LogicalVolume* VDbox_YLV = new G4LogicalVolume(VDbox_Y, WorldMaterial, "VDbox_YLV");

   VD[17] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, -PolyA/2 - 3*VDt/2, Lead_TargetZ),
                 VDbox_YLV,
                 "VDbox_fPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[18] = new G4PVPlacement
                (0,
                 G4ThreeVector(0, PolyA/2 + 3*VDt/2, Lead_TargetZ),
                 VDbox_YLV,
                 "VDbox_bPV",
                 worldLV,
                 false,
                 0,
                 false);

//////////////////////////////////////////
  G4Box* VDbox_X = new G4Box("VDbox_X", VDt/2, PolyA/2, PolyA/2);
  G4LogicalVolume* VDbox_XLV = new G4LogicalVolume(VDbox_X, WorldMaterial, "VDbox_XLV");

   VD[19] = new G4PVPlacement
                (0,
                 G4ThreeVector(-PolyA/2 - 3*VDt/2, 0, Lead_TargetZ),
                 VDbox_XLV,
                 "VDbox_lPV",
                 worldLV,
                 false,
                 0,
                 false);

   VD[20] = new G4PVPlacement
                (0,
                 G4ThreeVector(PolyA/2 + 3*VDt/2, 0, Lead_TargetZ),
                 VDbox_XLV,
                 "VDbox_rPV",
                 worldLV,
                 false,
                 0,
                 false);


/**/

////////////////////////////////////////////////////////////////////////

  //                                        
  // Visualization attributes
  //
  worldLV->SetVisAttributes (G4VisAttributes::GetInvisible());

  auto simpleBoxVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  simpleBoxVisAtt->SetVisibility(true);

  //
  // Always return the physical World
  //
  return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
  // G4SDManager::GetSDMpointer()->SetVerboseLevel(1);


  // 
  // Magnetic field
  //
  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
  // the field value is not zero.
  G4ThreeVector fieldValue;
  fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
  fMagFieldMessenger->SetVerboseLevel(1);
  
  // Register the field messenger for deleting
  G4AutoDelete::Register(fMagFieldMessenger);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
