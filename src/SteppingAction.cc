#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "SensitiveDetector.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"

#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4NeutronInelasticProcess.hh"

RecorderBase* SteppingAction::recorder;
G4String SteppingAction::volatile_name;
vector<SensitiveDetector*> SteppingAction::sensitive;
vector< vector<double> > SteppingAction::deposited_energy;
vector<int> SteppingAction::index;
vector<int> SteppingAction::trackID;
vector<int> SteppingAction::parentID;
vector<double> SteppingAction::initial_energy;
vector<double> SteppingAction::final_energy;
vector<G4String> SteppingAction::particle_name;
vector<G4String> SteppingAction::creator_process;
vector<G4String> SteppingAction::creator_volume;
vector<G4ThreeVector> SteppingAction::initial_momentum;
bool* SteppingAction::tracker_n;
bool* SteppingAction::tracker_p;
bool* SteppingAction::tracker_d;
bool* SteppingAction::tracker_12B;
bool* SteppingAction::tracker_13B;
double* SteppingAction::parent_energy;
double SteppingAction::proton_cos_CM=2;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(RecorderBase* r) : G4UserSteppingAction()
{ 
  recorder=r;
  
  //Get out the volatile name
  G4NeutronInelasticProcess *n_inelastic=new G4NeutronInelasticProcess();
  volatile_name=n_inelastic->GetProcessName();
  delete n_inelastic;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{ 
  ClearVectors();
  sensitive.clear();
  deposited_energy.clear();
} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

unsigned int SteppingAction::ExpandVectors(const G4Step* step)
{
  G4Track* track=step->GetTrack();
  
  //index array is such that: trackID[i]==trackID[index[trackID[i]]] !!!
  //whenever the push_back is made, the newly added term is equal to: given_vector[index[track->GetTrackID()]]
  
  //Since the tracking is done in SteppingAction instead in SensitiveDetector, 'index' could in principle be
  //avoided, because we will store the data for ALL particles, so we could directly use trackID for indexing.
  //However, the 'index' approach is more general and is more memory efficient if used for tracking inside
  //SensitiveDetector, since then only the data for particles passing through the sensivitives are needed.
  //In addition, 'index' approach is independent of initial counter for track IDs (0 or 1 or whatever),
  //so it's safe against any possible future change in internal Geant4 track-ID counting.
  
  int member=-1;
  if (track->GetTrackID()<(int)index.size()) {
    member=index[track->GetTrackID()]; //may be -1 if the track is not already in memory
  }
  
  //Expand vectors if necessary
  if (member<0) {
    member=trackID.size();
    
    //Expand regular vectors
    trackID.push_back(track->GetTrackID());
    parentID.push_back(track->GetParentID());
    initial_energy.push_back(track->GetVertexKineticEnergy());
    final_energy.push_back(step->GetPreStepPoint()->GetKineticEnergy()); //PRE-step
    initial_momentum.push_back(track->GetVertexMomentumDirection());
    particle_name.push_back(track->GetDefinition()->GetParticleName());
    creator_volume.push_back(track->GetLogicalVolumeAtVertex()->GetName());
    if (track->GetParentID()==0) {
      creator_process.push_back("User");
    } else {
      creator_process.push_back(track->GetCreatorProcess()->GetProcessName());
    }
    
    //Expand deposited energy for all sensitives
    for (unsigned int i=0; i<deposited_energy.size(); i++) {
      deposited_energy[i].push_back(0);
    }
    
    //Expand index
    if ((int)index.size()<=track->GetTrackID()) {
      unsigned int last_size=index.size();
      index.resize(track->GetTrackID()+1);
      for (unsigned int i=last_size; i<index.size(); i++) {
	index[i]=-1;
      }
    }
    index[track->GetTrackID()]=member;
  }

  return member;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::ClearVectors()
{
  trackID.clear();
  parentID.clear();
  index.clear();
  initial_energy.clear();
  final_energy.clear();
  particle_name.clear();
  creator_process.clear();
  creator_volume.clear();
  initial_momentum.clear();
  
  for (unsigned int i=0; i<deposited_energy.size(); i++) {
    deposited_energy[i].clear();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

unsigned int SteppingAction::ReportNewSensitive(SensitiveDetector* new_one)
{
  sensitive.push_back(new_one);
  deposited_energy.resize(sensitive.size());
  
  return deposited_energy.size()-1; //returns the index of the last term!
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::BeginOfEvent()
{
  ClearVectors();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  //G4Track* track=step->GetTrack();

  unsigned int member=ExpandVectors(step);
  
  final_energy[member]=step->GetPreStepPoint()->GetKineticEnergy(); //PRE-step
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void SteppingAction::ReportDetection(unsigned int index,const G4Step* step)
{ 
  if (index<deposited_energy.size()) { //check index>=0 is unnecessary since index is unsigned
    unsigned int member=ExpandVectors(step);
    
    if (step->GetTrack()->GetGlobalTime()<1e3*ns) {
      deposited_energy[index][member]+=step->GetTotalEnergyDeposit()-step->GetNonIonizingEnergyDeposit(); //only ionisation deposit
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::ReportProtonCosCM(double _proton_cos_CM_)
{
  proton_cos_CM=_proton_cos_CM_;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::EndOfEvent()
{
  const unsigned int SENSITIVES=deposited_energy.size();
  const unsigned int TRACKS=trackID.size();
  
  //This is example of tracking through the particle history, searching for the connection with the
  //deepest interesting parent, like the particle directly fired by user (deepest_parent_ID=0),
  //or the first product created by the primary particle (deepest_parent_ID=1).
  
  int deepest_parent_ID; //usually 0 or 1
  if (UserInput::EmitProtons()) { //emit protons
    deepest_parent_ID=0; 
  } else { //emit neutrons
    deepest_parent_ID=1; 
  }

  //At the end, this array will hold the index of the deepest parent, for each particle.
  //The properties of the deepest parent may then be accessed as ('i' is the particle index):
  //                         property[interesting_parent[i]]
  //TAKE CARE! 'interesting_parent' will be parent's index, NOT its track-ID!
  int *interesting_parent=new int[TRACKS];
  
  //'cumulative_deposit' will hold energy deposition assigned ALL THE WAY UP to the deepest parent.
  //For simplicity, assume that every track is a candidate for a deepest parent.
  //Ath the end, only the cumulative_deposit of deepest parent will have non-zero cumulative deposit.
  double **cumulative_deposit=new double*[SENSITIVES]; //as many as sensitive detectors
  double *total_deposit=new double[SENSITIVES]; //as many as sensitive detectors
  for (unsigned int i=0; i<SENSITIVES; i++) {
    cumulative_deposit[i]=new double[TRACKS]; //as many as tracks
    for (unsigned int j=0; j<TRACKS; j++) {
      cumulative_deposit[i][j]=0;
    }
    total_deposit[i]=0;
  }
  
  //Special trackers
  tracker_n=new bool[TRACKS];
  tracker_p=new bool[TRACKS];
  tracker_d=new bool[TRACKS];
  tracker_12B=new bool[TRACKS];
  tracker_13B=new bool[TRACKS];
  parent_energy=new double[TRACKS];
  
  for (unsigned int i=0; i<TRACKS; i++) {
    tracker_n[i]=false;
    tracker_p[i]=false;
    tracker_d[i]=false;
    tracker_12B[i]=false;
    tracker_13B[i]=false;
    parent_energy[i]=0;
  }
  
  //Tracking
  for (unsigned int i=0; i<TRACKS; i++) {
    //At this level it holds:
    //                index[trackID[i]]==i
    
    //Also, remember that it always holds (by construction):
    //          trackID[index[trackID[j]]==trackID[j]

    if (parentID[i]<deepest_parent_ID) {
      //This is the particle deeper in history than what we are interested in.
      interesting_parent[i]=-1;
      
    } else if (parentID[i]==deepest_parent_ID) {
      //This is the interesting particle!
      interesting_parent[i]=i;
      
      for (unsigned int j=0; j<SENSITIVES; j++) {
	cumulative_deposit[j][i]+=deposited_energy[j][i];
	total_deposit[j]+=deposited_energy[j][i];
      }
      
      TrackInteresting(trackID[i],parentID[i]);
      
    } else {
      //These are all secondary particles.

      //This is a safe search code, which can also be applied if the tracking was performed inside SensitiveDetector.
      //In that case, since we wouldn't have tracked outside the sensitive volume, we would need to take care that there were
      //no complex reactions outside it, such that the deepest parent could not be reached in the history from the sensitive volume.
      int current_parent=parentID[i];
      interesting_parent[i]=index[current_parent];
      TrackInteresting(trackID[i],parentID[i]);
      
      while (index[current_parent]>=0 && parentID[index[current_parent]]>deepest_parent_ID) { //particle not yet found in history
	TrackInteresting(current_parent,parentID[index[current_parent]]);
	current_parent=parentID[index[current_parent]];
	interesting_parent[i]=index[current_parent];
      }
      
      //The following condition is just for safety in case this code was transferred into SensitiveDetector,
      //since the link with the deepest parent could then be lost (due to tracking only inside the sensitive volume).
      if (index[current_parent]>=0 && parentID[index[current_parent]]==deepest_parent_ID) {
	for (unsigned int j=0; j<SENSITIVES; j++) {
	  cumulative_deposit[j][interesting_parent[i]]+=deposited_energy[j][i]; //assigning the energy deposition to interesting parent!
	  total_deposit[j]+=deposited_energy[j][i];
	}
      } else {
	interesting_parent[i]=-1;
      }

    }
  }
  
  //Reporting
  for (unsigned int i=0; i<SENSITIVES; i++) {
    for (unsigned int j=0; j<TRACKS; j++) {
      if (interesting_parent[j]==(int)j) {
	//Instead of this condition, one can also put 'cumulative_deposit[i][j]>0',
	//so as to report only the events with actual energy deposition. But, if the
	//deposited energy is one of the reported results, then this condition may
	//be incorporated through the OutputCondition() function from UserInput class.
	//This is more flexible, since one may want to output some data as soon as 
	//something passes through the detector, regardless of the deposited energy.
	
	DataCollection data(sensitive[i]->GetName(),cumulative_deposit[i][j],total_deposit[i],initial_energy[j],
			    particle_name[j],creator_volume[j],creator_process[j],initial_momentum[j],
			    tracker_n[index[parentID[j]]],tracker_p[index[parentID[j]]],tracker_d[index[parentID[j]]],tracker_12B[index[parentID[j]]],tracker_13B[index[parentID[j]]],
			    proton_cos_CM);
	recorder->ReportEvent(data);

      }
    }
  }

  //Deleting
  for (unsigned int i=0; i<SENSITIVES; i++) {
    delete[] cumulative_deposit[i];
  }
  delete[] cumulative_deposit;
  delete[] total_deposit;
  delete[] interesting_parent;
  
  delete[] tracker_n;
  delete[] tracker_p;
  delete[] tracker_d;
  delete[] tracker_12B;
  delete[] tracker_13B;
  delete[] parent_energy;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::TrackInteresting(int current_track,int current_parent)
{
  
  if (current_track>(int)index.size()-1 || current_parent>(int)index.size()-1 || index[current_track]<0 || index[current_parent]<0)
    return;

  if (creator_process[index[current_track]]==volatile_name) {
    bool is_n=(particle_name[index[current_track]]=="neutron");
    bool is_p=(particle_name[index[current_track]]=="proton");
    bool is_d=(particle_name[index[current_track]]=="deuteron");
    bool is_12B=(particle_name[index[current_track]].substr(0,3)=="B12");
    bool is_13B=(particle_name[index[current_track]].substr(0,3)=="B13");

    //First part of checking
    if (is_n) {
      tracker_n[index[current_track]]=true;
      //Here don't make final_energy check because neutron doesn't guarantee that interesting reaction has happened!
    } else if (is_p) {
      tracker_p[index[current_track]]=true;
      //Here don't make final_energy check because proton doesn't guarantee that interesting reaction has happened!
    } else if (is_d) {
      tracker_d[index[current_track]]=true;
      //Here don't make final_energy check because deuteron doesn't guarantee that interesting reaction has happened!
    } else if (is_12B) {
      tracker_12B[index[current_track]]=true;
      parent_energy[index[current_track]]=final_energy[index[current_parent]];
    } else if (is_13B) {
      tracker_13B[index[current_track]]=true;
      parent_energy[index[current_track]]=final_energy[index[current_parent]]; 
    }
    
    //Second part of checking
    if (tracker_n[index[current_track]]==true) {
      tracker_n[index[current_parent]]=true;
      //Here don't make final_energy assignment because neutron doesn't guarantee that interesting reaction has happened!
    }
    if (tracker_p[index[current_track]]==true) {
      tracker_p[index[current_parent]]=true;
      //Here don't make final_energy assignment because proton doesn't guarantee that interesting reaction has happened!
    }
    if (tracker_d[index[current_track]]==true) {
      tracker_d[index[current_parent]]=true;
      //Here don't make final_energy assignment because deuteron doesn't guarantee that interesting reaction has happened!
    }
    if (tracker_12B[index[current_track]]==true) {
      tracker_12B[index[current_parent]]=true;
      parent_energy[index[current_parent]]=parent_energy[index[current_track]];
    }
    if (tracker_13B[index[current_track]]==true) {
      tracker_13B[index[current_parent]]=true;
      parent_energy[index[current_parent]]=parent_energy[index[current_track]];
    }

  }
}
