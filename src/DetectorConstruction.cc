#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"
#include "PrimaryGeneratorAction.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"
#include "WarningSuppressor.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Tubs.hh"
#include "G4EllipticalTube.hh"
#include "G4Sphere.hh"
#include "G4Torus.hh"
#include "G4Cons.hh"
#include "G4Para.hh"
#include "G4CSGSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(RecorderBase* r) : G4VUserDetectorConstruction()
{
  recorder=r;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  //Define materials
  G4Material *air=G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
  G4Material *vacuum=G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
  G4Material *C=G4NistManager::Instance()->FindOrBuildMaterial("G4_C");
  //G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  //G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  //G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  G4VisAttributes* color_cyan= new G4VisAttributes(G4Colour::Cyan()); 
  
  //Geometry
  double WorldSize=20.*cm;
  
  //World
  G4Box *world=new G4Box("World",WorldSize/2,WorldSize/2,WorldSize/2);
  G4LogicalVolume *logic_world=new G4LogicalVolume(world,vacuum,"World");                              
  G4VPhysicalVolume *phys_world=new G4PVPlacement(0,G4ThreeVector(0,0,0),logic_world,"World",0,false,0,UserInput::CheckOverlaps());
  
  //BEAM RADIUS
  const double R=2.5*cm;
  
  PrimaryGeneratorAction::ReportBeamRadius(R);
  
  //Carbon sample
  G4RotationMatrix C_rotation;
  double C_angle=-45.*deg;
  C_rotation.rotateX(C_angle);
  
  double C_diameter_x=2.*R+2.*mm;
  double C_diameter_y=C_diameter_x/cos(C_angle);
  double C_width=0.25*mm; //ZA ORIGINALNI PROPOSAL JE BILO 0.5*mm !!!
  G4ThreeVector C_position=G4ThreeVector(0,0,0);
  
  G4EllipticalTube *C_shape=new G4EllipticalTube("Carbon",C_diameter_x/2.,C_diameter_y/2.,C_width/2.);
  G4LogicalVolume *C_logical=new G4LogicalVolume(C_shape,C,"Carbon");
  G4VPhysicalVolume *C_physical=new G4PVPlacement(G4Transform3D(C_rotation,C_position),C_logical,"Carbon",logic_world,false,0,UserInput::CheckOverlaps());

  C_logical->SetVisAttributes(color_cyan);
  ReportAsSensitive(C_logical);
  
  PrimaryGeneratorAction::ReportSampleSize(C_position,C_diameter_x/2.,C_diameter_y/2.,C_width,abs(C_angle),C_rotation);
  
  WarningSuppressor::SuppressWarning(vacuum);
  WarningSuppressor::SuppressWarning(air);
  WarningSuppressor::SuppressWarning(C_physical);
  
  //SILICON CONFIGURATIONS
  
  //Configuration_0(logic_world,C_position);
  Configuration_1(logic_world,C_position);
  //Configuration_2(logic_world,C_position);
  //Configuration_3(logic_world,C_position);
  //Configuration_4(logic_world,C_position);
  //Configuration_5(logic_world,C_position);

  G4cout<<G4endl;
  return phys_world;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_0(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int LAYERS=2;
  
  double Si_distance=43.*mm;
  double Si_separation=7.*mm;
  double Si_X=50.*mm;
  double Si_Y=50.*mm;
  double Si_Z[LAYERS]={20.*um,300.*um};
  
  double active_edge=3.*mm;
  double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);
  
  G4Box *Si_shape[LAYERS][STRIPS_1],*Dead_shape[LAYERS][STRIPS_1-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1],*Dead_logical[LAYERS][STRIPS_1-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1],*Dead_physical[LAYERS][STRIPS_1-1];
  
  ostringstream *buffer;
  string name;
  for (int i=0; i<LAYERS; i++) {
    for (int j=0; j<STRIPS_1; j++) {
      //Active strip
      buffer=new ostringstream;
      (*buffer)<<"Si_"<<i+1<<(char)('A'+j);
      name=(*buffer).str();
      delete buffer;
      
      double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
      G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift);

      Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
      Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
      Si_physical[i][j]=new G4PVPlacement(0,active_position,Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

      Si_logical[i][j]->SetVisAttributes(color_red);
      ReportAsSensitive(Si_logical[i][j]);
      
      WarningSuppressor::SuppressWarning(Si_physical[i][j]);

      //Dead layer
      if (j<STRIPS_1-1) {
	buffer=new ostringstream;
	(*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
      
	G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.);
	
	Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	Dead_physical[i][j]=new G4PVPlacement(0,dead_position,Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	Dead_logical[i][j]->SetVisAttributes(color_green);
	
	WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_1(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int STRIPS_2=16;
  const int LAYERS=2;
  
  G4Box *Si_shape[LAYERS][STRIPS_1+STRIPS_2],*Dead_shape[LAYERS][STRIPS_1+STRIPS_2-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1+STRIPS_2],*Dead_logical[LAYERS][STRIPS_1+STRIPS_2-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1+STRIPS_2],*Dead_physical[LAYERS][STRIPS_1+STRIPS_2-1];
  
  {
    double Si_distance=43.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=-2.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);

    double Si_theta=0.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=0; j<STRIPS_1; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	/*
	buffer=new ostringstream;
	if( i == 0){
		(*buffer)<<"Strip_PD_"<<j;
	}else{
		(*buffer)<<"Strip_DD_"<<j;
	}
	name=(*buffer).str();
	delete buffer;
	*/
	
	double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!
	
	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
  //SECOND SET OF LAYERS (floating parentheses are so that all deeper parameters are local)
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=0.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_2*active_edge)/(STRIPS_2-1.);

    double Si_theta=(180.+45.)*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=STRIPS_1; j<STRIPS_1+STRIPS_2; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	/*
	buffer=new ostringstream;
	if( i == 0){
		(*buffer)<<"Strip_PG_"<<j;
	}else{
		(*buffer)<<"Strip_DG_"<<j;
	}
	name=(*buffer).str();
	delete buffer;
	*/
	
	double active_shift=(C_position.z()-Si_Y/2.)+(j-STRIPS_1)*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!

	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1+STRIPS_2-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_2(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int STRIPS_2=16;
  const int LAYERS=2;
  
  G4Box *Si_shape[LAYERS][STRIPS_1+STRIPS_2],*Dead_shape[LAYERS][STRIPS_1+STRIPS_2-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1+STRIPS_2],*Dead_logical[LAYERS][STRIPS_1+STRIPS_2-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1+STRIPS_2],*Dead_physical[LAYERS][STRIPS_1+STRIPS_2-1];
  
  {
    double Si_distance=43.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=-2.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);

    double Si_theta=0.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=0; j<STRIPS_1; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!
	
	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
  //SECOND SET OF LAYERS (floating parentheses are so that all deeper parameters are local)
  {
    double Si_distance=43.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=2.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_2*active_edge)/(STRIPS_2-1.);

    double Si_theta=180.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=STRIPS_1; j<STRIPS_1+STRIPS_2; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+(j-STRIPS_1)*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!

	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1+STRIPS_2-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_3(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int STRIPS_2=16;
  const int LAYERS=2;
  
  G4Box *Si_shape[LAYERS][STRIPS_1+STRIPS_2],*Dead_shape[LAYERS][STRIPS_1+STRIPS_2-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1+STRIPS_2],*Dead_logical[LAYERS][STRIPS_1+STRIPS_2-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1+STRIPS_2],*Dead_physical[LAYERS][STRIPS_1+STRIPS_2-1];
  
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=43.*mm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);

    double Si_theta=45.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=0; j<STRIPS_1; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!
	
	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
  //SECOND SET OF LAYERS (floating parentheses are so that all deeper parameters are local)
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=-43.*mm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_2*active_edge)/(STRIPS_2-1.);

    double Si_theta=(180.+45.)*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=STRIPS_1; j<STRIPS_1+STRIPS_2; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+(j-STRIPS_1)*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!

	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1+STRIPS_2-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_4(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int STRIPS_2=16;
  const int LAYERS=2;
  
  G4Box *Si_shape[LAYERS][STRIPS_1+STRIPS_2],*Dead_shape[LAYERS][STRIPS_1+STRIPS_2-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1+STRIPS_2],*Dead_logical[LAYERS][STRIPS_1+STRIPS_2-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1+STRIPS_2],*Dead_physical[LAYERS][STRIPS_1+STRIPS_2-1];
  
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=0;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);

    double Si_theta=45.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=0; j<STRIPS_1; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!
	
	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
  //SECOND SET OF LAYERS (floating parentheses are so that all deeper parameters are local)
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=0;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_2*active_edge)/(STRIPS_2-1.);

    double Si_theta=(180.+45.)*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=STRIPS_1; j<STRIPS_1+STRIPS_2; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+(j-STRIPS_1)*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!

	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1+STRIPS_2-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::Configuration_5(G4LogicalVolume *logic_world,G4ThreeVector &C_position)
{
  //Define materials
  G4Material *Si=G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");
  
  //Define colors
  G4VisAttributes* color_red= new G4VisAttributes(G4Colour::Red()); 
  G4VisAttributes* color_green= new G4VisAttributes(G4Colour::Green());
  
  //Silicon strips
  const int STRIPS_1=16;
  const int STRIPS_2=16;
  const int LAYERS=2;
  
  G4Box *Si_shape[LAYERS][STRIPS_1+STRIPS_2],*Dead_shape[LAYERS][STRIPS_1+STRIPS_2-1];
  G4LogicalVolume *Si_logical[LAYERS][STRIPS_1+STRIPS_2],*Dead_logical[LAYERS][STRIPS_1+STRIPS_2-1];
  G4VPhysicalVolume *Si_physical[LAYERS][STRIPS_1+STRIPS_2],*Dead_physical[LAYERS][STRIPS_1+STRIPS_2-1];
  
  {
    double Si_distance=43.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=1.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_1*active_edge)/(STRIPS_1-1.);

    double Si_theta=30.*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=0; j<STRIPS_1; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+j*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!
	
	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
  
  //SECOND SET OF LAYERS (floating parentheses are so that all deeper parameters are local)
  {
    double Si_distance=60.*mm; //RADIUS OF ROTATION!
    double Si_front_shift=0.*cm;
    double Si_separation=7.*mm;
    double Si_X=50.*mm;
    double Si_Y=50.*mm;
    double Si_Z[LAYERS]={20.*um,300.*um};
    
    double active_edge=3.*mm;
    double dead_edge=(Si_Y-STRIPS_2*active_edge)/(STRIPS_2-1.);

    double Si_theta=(180.+45.)*deg;
    G4RotationMatrix Si_rotation;
    Si_rotation.rotateX(Si_theta);
    
    G4ThreeVector fixed_shift=G4ThreeVector(0,0,Si_front_shift);
    
    ostringstream *buffer;
    string name;
    for (int i=0; i<LAYERS; i++) {
      for (int j=STRIPS_1; j<STRIPS_1+STRIPS_2; j++) {
	//Active strip
	buffer=new ostringstream;
	(*buffer)<<"Si_"<<i+1<<(char)('A'+j);
	name=(*buffer).str();
	delete buffer;
	
	double active_shift=(C_position.z()-Si_Y/2.)+(j-STRIPS_1)*(active_edge+dead_edge)+active_edge/2.;
	G4ThreeVector active_position=G4ThreeVector(0,-(Si_distance+i*Si_separation),active_shift); //as yet unrotated both in orientation and position!

	Si_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,active_edge/2.);
	Si_logical[i][j]=new G4LogicalVolume(Si_shape[i][j],Si,name.c_str());
	Si_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*active_position+fixed_shift),Si_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());
	
	Si_logical[i][j]->SetVisAttributes(color_red);
	ReportAsSensitive(Si_logical[i][j]);
	
	WarningSuppressor::SuppressWarning(Si_physical[i][j]);

	//Dead layer
	if (j<STRIPS_1+STRIPS_2-1) {
	  buffer=new ostringstream;
	  (*buffer)<<"Dead_"<<i+1<<(char)('A'+j);
	  name=(*buffer).str();
	  delete buffer;
	
	  G4ThreeVector dead_position=active_position+G4ThreeVector(0,0,(active_edge+dead_edge)/2.); //as yet unrotated both in orientation and position!
	  
	  Dead_shape[i][j]=new G4Box(name.c_str(),Si_X/2.,Si_Z[i]/2.,dead_edge/2.);
	  Dead_logical[i][j]=new G4LogicalVolume(Dead_shape[i][j],Si,name.c_str());
	  Dead_physical[i][j]=new G4PVPlacement(G4Transform3D(Si_rotation,Si_rotation*dead_position+fixed_shift),Dead_logical[i][j],name.c_str(),logic_world,false,0,UserInput::CheckOverlaps());

	  Dead_logical[i][j]->SetVisAttributes(color_green);
	  
	  WarningSuppressor::SuppressWarning(Dead_physical[i][j]);
	}
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ReportAsSensitive(G4LogicalVolume* detector)
{
  SensitiveDetector *sensitive=new SensitiveDetector(detector->GetName(),recorder);
  G4SDManager::GetSDMpointer()->AddNewDetector(sensitive);  
  detector->SetSensitiveDetector(sensitive);
}
