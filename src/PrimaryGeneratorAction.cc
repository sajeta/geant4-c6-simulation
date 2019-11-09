#include "PrimaryGeneratorAction.hh"
#include "SteppingAction.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <cmath>

const double PrimaryGeneratorAction::pi=3.14159265358979323846;

double PrimaryGeneratorAction::initial_energy=0;
G4ThreeVector PrimaryGeneratorAction::initial_position=G4ThreeVector();
G4ThreeVector PrimaryGeneratorAction::initial_direction=G4ThreeVector();
G4ThreeVector PrimaryGeneratorAction::sample_position=G4ThreeVector();
double PrimaryGeneratorAction::sample_Rx=0;
double PrimaryGeneratorAction::sample_Ry=0;
double PrimaryGeneratorAction::sample_width=0;
double PrimaryGeneratorAction::sample_angle=0;
G4RotationMatrix PrimaryGeneratorAction::sample_rotation;
double PrimaryGeneratorAction::beam_radius=0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(RecorderBase* r) : G4VUserPrimaryGeneratorAction()
{
  recorder=r;
  particle_gun=new G4ParticleGun(1);

  //Listing available paricles
  if (UserInput::ListParticles()) {
    G4cout<<"AVAILABLE PARTICLES:"<<G4endl;
    G4ParticleTable::G4PTblDicIterator* iter=G4ParticleTable::GetParticleTable()->GetIterator();
    iter->reset();
    while((*iter)()){
      G4ParticleDefinition* particle=iter->value();
      G4cout<<particle->GetParticleName()<<"  ";
      //G4cout <<"   -> "<<particle->GetParticleName()<< G4endl;
    }
    G4cout<<G4endl<<G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete particle_gun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::ReportSampleSize(G4ThreeVector position,double Rx,double Ry,double width,double angle,G4RotationMatrix rotation)
{
  sample_position=position;
  sample_Rx=Rx;
  sample_Ry=Ry;
  sample_width=width;
  sample_angle=angle;
  sample_rotation=rotation;
}
   
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
   
void PrimaryGeneratorAction::ReportBeamRadius(double radius)
{
  beam_radius=radius;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  //BEGIN EXAMPLE of generating an event
  //Energy sampling
  if (UserInput::GetIsolethargicOption()) {
    //Isolethargically distributed energies
    initial_energy=exp(log(UserInput::GetMinEnergy())+log(UserInput::GetMaxEnergy()/UserInput::GetMinEnergy())*G4UniformRand());
  } else {
    //Uniformly distributed energies
    initial_energy=UserInput::GetMinEnergy()+(UserInput::GetMaxEnergy()-UserInput::GetMinEnergy())*G4UniformRand(); 
  }

  G4ParticleDefinition* particle;
  
  //Position sampling (radial distribution of neutron beam)
  double sigma=0.75*cm;
  
  double x0,y0,z0,px,py,pz;
  do {
    x0=sigma*G4RandGauss::shoot();
    y0=sigma*G4RandGauss::shoot();
  } while (x0*x0+y0*y0>=beam_radius*beam_radius);
  
  double primary_energy; //because initial_energy must stay the neutron's energy, for reporting!!!
  if (!UserInput::EmitProtons()) {
    //Emit neutrons
    particle=G4ParticleTable::GetParticleTable()->FindParticle("neutron");
    
    px=0;
    py=0;
    pz=1;
    
    primary_energy=initial_energy;

    z0=0;
    initial_position=sample_position-G4ThreeVector(0,0,sample_Ry*sin(sample_angle))-G4ThreeVector(0,0,sample_width/2.+1.*mm)+G4ThreeVector(x0,y0,z0);
    
  } else {
    //Emit protons
    particle=G4ParticleTable::GetParticleTable()->FindParticle("proton");

    //Isotropic angles in CM frame
    double phi=2.*pi*G4UniformRand();
    double cos_theta_CM=2.*G4UniformRand()-1.;
    
    SteppingAction::ReportProtonCosCM(cos_theta_CM);
    
    //Constants
    const double Q_value=-12.5871*MeV-UserInput::GetExcitedLevel();
    const double mass_n=939.565413*MeV;
    const double mass_p=938.272046*MeV;
    const double mass_C=11.17792914*GeV; //12.*931.494095 MeV
    const double mass_B=mass_n+mass_C-mass_p-Q_value;    //otherwise:  11.19129803640085*GeV; //12.0143521*931.494095 MeV
    
    //Relativistic kinematics
    double beta_CM=sqrt(pow(initial_energy,2)+2*initial_energy*mass_n)/(initial_energy+(mass_n+mass_C));
    double gamma_CM=1./sqrt(1.-pow(beta_CM,2));
    
    double En_LAB=initial_energy+mass_n;
    double pn_LAB=sqrt(pow(En_LAB,2)-pow(mass_n,2));
    double En_CM=gamma_CM*(En_LAB-beta_CM*pn_LAB);
    double EC_CM=gamma_CM*mass_C;
    double Etot_CM=En_CM+EC_CM;
    double Ep_CM=Etot_CM/2.-(pow(mass_B,2)-pow(mass_p,2))/(2.*Etot_CM);
    if (Ep_CM>mass_p) {
      //above reaction threshold!
      double pp_CM=sqrt(pow(Ep_CM,2)-pow(mass_p,2));
      double Ep_LAB=gamma_CM*(Ep_CM+beta_CM*pp_CM*cos_theta_CM);
      double pp_LAB_r=pp_CM*sqrt(1.-pow(cos_theta_CM,2));
      double pp_LAB_z=gamma_CM*(pp_CM*cos_theta_CM+beta_CM*Ep_CM);

      /*
      double EB_CM=Etot_CM/2.+(pow(mass_B,2)-pow(mass_p,2))/(2.*Etot_CM); //not required
      double pB_CM=sqrt(pow(EB_CM,2)-pow(mass_B,2)); //not required
      double EB_LAB=gamma_CM*(EB_CM-beta_CM*pB_CM*cos_theta_CM); //not required
      double pB_LAB_r=pB_CM*sqrt(1.-pow(cos_theta_CM,2)); //not required
      double pB_LAB_z=gamma_CM*(-pB_CM*cos_theta_CM+beta_CM*EB_CM); //not required (minus becuase residual goes in the other direction in CM frame)
      
      //CHECK that the conservation laws are respected:
      G4cout<<(mass_n+mass_C+initial_energy-Ep_LAB-EB_LAB)/MeV<<"  "<<(pn_LAB-pp_LAB_z-pB_LAB_z)/MeV<<"  "<<(pp_LAB_r-pB_LAB_r)/MeV<<G4endl;
      */
      
      double cos_theta_LAB=pp_LAB_z/sqrt(pow(pp_LAB_r,2)+pow(pp_LAB_z,2));
      double sin_theta_LAB=sqrt(1.-pow(cos_theta_LAB,2));
    
      px=sin_theta_LAB*cos(phi);
      py=sin_theta_LAB*sin(phi);
      pz=cos_theta_LAB;

      primary_energy=Ep_LAB-mass_p; //now it is a kinetic energy of a proton
      
    } else {
      //below reaction threshold
      px=0;
      py=0;
      pz=1;
      
      primary_energy=0; //so that nothing happens
    }
    
    //Adjust position
    z0=-y0*tan(sample_angle)+(G4UniformRand()-0.5)*sample_width/cos(sample_angle);
    
    initial_position=sample_position+G4ThreeVector(x0,y0,z0);
    
    //KRIVO:
    /*
    z0=sample_position.z()+sample_width*(G4UniformRand()-0.5);
    y0/=cos(sample_angle); //preparing for projection
    initial_position=sample_position+sample_rotation*G4ThreeVector(x0,y0,z0);
    */
  }

  initial_direction=G4ThreeVector(px,py,pz);
  //END EXAMPLE
  
  //This is no longer an example (it's necessary)
  particle_gun->SetParticleDefinition(particle);
  particle_gun->SetParticleEnergy(primary_energy);
  particle_gun->SetParticlePosition(initial_position);
  particle_gun->SetParticleMomentumDirection(initial_direction);
  particle_gun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double PrimaryGeneratorAction::GetInitialEnergy()
{
  return initial_energy;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreeVector PrimaryGeneratorAction::GetInitialPosition()
{
  return initial_position;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreeVector PrimaryGeneratorAction::GetInitialDirection()
{
  return initial_direction;
}
