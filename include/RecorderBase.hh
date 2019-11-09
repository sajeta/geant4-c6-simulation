#ifndef RecorderBase_h
#define RecorderBase_h 1

#include "globals.hh"
#include "G4ThreeVector.hh"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

class DataCollection; //DEFINED BELOW

class RecorderBase {
public:
  RecorderBase(G4String);
  ~RecorderBase();
  
  void ReportHostileTakeover(bool automatic=true,bool hostile=true);
  void ReportEvent(DataCollection); //ADD OR REMOVE ARGUMENTS AS NECESSARY
  long long GetAcceptedEvents();
  bool GetHostileTakeover();
  
private:
  void OpenFile();
  void CloseFile();
  bool PrepareForOutput();
  
  G4String address,filename,simulation_id;
  ofstream out;
  bool hostile_takeover;
  long long written_counts,accepted_events,last_event,next_flush;
};

//#############################################################################################################################################

//This class only serves as the container for the data, for the more elegant passing of the simulation results between the parts of the program

class DataCollection {
public:
  DataCollection()
  {
    partial_deposit=0;
    total_deposit=0;
    initial_energy=0;
    detector="";
    particle="";
    creator_volume="";
    creator_process="";
    initial_P=G4ThreeVector();
    tracked_n=false;
    tracked_p=false;
    tracked_d=false;
    tracked_12B=false;
    tracked_13B=false;
    proton_cos_CM=0;
  }
  
  DataCollection(G4String _detector_,double _partial_deposit_,double _total_deposit_,double _initial_energy_,
		 G4String _particle_,G4String _creator_volume_,G4String _creator_process_,G4ThreeVector _initial_P_,
		 bool _tracked_n_,bool _tracked_p_,bool _tracked_d_,bool _tracked_12B_,bool _tracked_13B_,double _proton_cos_CM_)
  {
    partial_deposit=_partial_deposit_;
    total_deposit=_total_deposit_;
    initial_energy=_initial_energy_;
    detector=_detector_;
    particle=_particle_;
    creator_volume=_creator_volume_;
    creator_process=_creator_process_;
    initial_P=_initial_P_;
    tracked_n=_tracked_n_;
    tracked_p=_tracked_p_;
    tracked_d=_tracked_d_;
    tracked_12B=_tracked_12B_;
    tracked_13B=_tracked_13B_;
    proton_cos_CM=_proton_cos_CM_;
  }
  
public:
  double partial_deposit;
  double total_deposit;
  double initial_energy;
  G4String detector;
  G4String particle;
  G4String creator_volume;
  G4String creator_process;
  G4ThreeVector initial_P;
  bool tracked_n;
  bool tracked_p;
  bool tracked_d;
  bool tracked_12B;
  bool tracked_13B;
  double proton_cos_CM;
};

#endif
