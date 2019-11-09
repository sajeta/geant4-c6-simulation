#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4RotationMatrix.hh"
#include "globals.hh"

class G4Event;
class RecorderBase;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction(RecorderBase*);    
    ~PrimaryGeneratorAction();

    void GeneratePrimaries(G4Event*);
    
    static G4double GetInitialEnergy();
    static G4ThreeVector GetInitialPosition();
    static G4ThreeVector GetInitialDirection();
    
    static void ReportSampleSize(G4ThreeVector,double,double,double,double,G4RotationMatrix);
    static void ReportBeamRadius(double);
    
  private:
    static const double pi;
    
    static double initial_energy;
    static G4ThreeVector initial_position;
    static G4ThreeVector initial_direction;

    static G4ThreeVector sample_position;
    static double sample_Rx;
    static double sample_Ry;
    static double sample_width;
    static double sample_angle;
    static G4RotationMatrix sample_rotation;
    static double beam_radius;
    
    RecorderBase *recorder;
    G4ParticleGun *particle_gun;
};

#endif
