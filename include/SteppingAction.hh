#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <vector>
using namespace std;

class RecorderBase;
class SensitiveDetector;

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(RecorderBase*);
    ~SteppingAction();

    static void BeginOfEvent();
    void UserSteppingAction(const G4Step*);
    static void EndOfEvent();
    
    static unsigned int ReportNewSensitive(SensitiveDetector*);
    static void ReportDetection(unsigned int,const G4Step*);
    static void ReportProtonCosCM(double);
    
  private:
    static unsigned int ExpandVectors(const G4Step*);
    static void ClearVectors();
    
    static void TrackInteresting(int,int);
    
    static RecorderBase *recorder;
    
    static G4String volatile_name; //changes between the versions of Geant4
    
    static vector<SensitiveDetector*> sensitive;
    static vector< vector<double> > deposited_energy;
    
    static vector<int> index,trackID,parentID;
    static vector<double> initial_energy,final_energy;
    static vector<G4String> particle_name,creator_process,creator_volume;
    static vector<G4ThreeVector> initial_momentum;
    
    static bool *tracker_n,*tracker_p,*tracker_d,*tracker_12B,*tracker_13B;
    static double *parent_energy;
    
    static double proton_cos_CM;
};

#endif
