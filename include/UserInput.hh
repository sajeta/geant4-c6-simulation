#ifndef UserInput_h
#define UserInput_h 1

#include "globals.hh"

class DataCollection;

class UserInput {
public:
  UserInput();
  ~UserInput();
  
  static bool GetOutputOption(){return output_option;}
  static G4String GetOutputLocation(){return output_address;}
  static G4String GetOutputName(){return output_name;}
  static long long GetEventToReport(){return report_event;}
  static int GetOutputPrecision(){return precision;}
  static long long GetMaxRAMEntries(){return max_RAM_entries;}
  static double GetMinEnergy(){return min_energy;}
  static double GetMaxEnergy(){return max_energy;}
  static double GetIsolethargicOption(){return isolethargic;};
  static long long GetEvents(){return events;}
  static bool CountDetected(){return count_detected;}
  static bool CheckOverlaps(){return check_overlaps;}
  static bool ListParticles(){return list_particles;}
  static bool ListModels(){return list_models;}
  static bool EmitProtons(){return emit_protons;}
  static bool ReducedOutput(){return reduced_output;}
  static long long GetMaxWarnings(){return max_warnings;}
  static double GetExcitedLevel(){return LEVEL[STATE];};
  
  static bool OutputCondition(DataCollection);
  
public:
  static const bool output_option;
  static const G4String output_address;
  static const G4String output_name;
  static const long long report_event;
  static const int precision;
  static const long long max_RAM_entries;
  static const double min_energy;
  static const double max_energy;
  static const bool isolethargic;
  static const long long events;
  static const bool count_detected;
  static const bool check_overlaps;
  static const bool list_particles;
  static const bool list_models;
  static const bool emit_protons;
  static const bool reduced_output;
  static const long long max_warnings;
  
  static const int STATE;
  static const double LEVEL[50];
};

#endif
