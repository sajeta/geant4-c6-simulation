#include "UserInput.hh"
#include "RecorderBase.hh"

#include "G4SystemOfUnits.hh"

//Sampling options
const bool UserInput::isolethargic=false; //option for sampling the energies (isolethargic or uniform)

const int UserInput::STATE=0; //state among the list of 12B LEVELs

const long long UserInput::events=1e8; //events to be generated  (1e9 / day [for bertini])
const bool UserInput::count_detected=false; //option for fulfilling the quota of emitted or detected events
const double UserInput::min_energy=20.*MeV; //minimum sampling energy
const double UserInput::max_energy=20.*MeV; //maximum sampling energy

//Mode of simulation
const bool UserInput::emit_protons=true;
const bool UserInput::reduced_output=true;

//Output options
const bool UserInput::output_option=true; //option for outputting results into files
const G4String UserInput::output_address="/output/"; //address for output files (realtive to the directory with source-code files)
const G4String UserInput::output_name="run."; //filename base for output files
const long long UserInput::report_event=1e5; //number of events to be completed before reporting
const int UserInput::precision=4; //number of digits beyond decimal point in scientific notation (0 for not-set)
const long long UserInput::max_RAM_entries=100; //maximum number of events to keep in memory before flushing them out into the file

//Other options
const bool UserInput::check_overlaps=false; //option for checking geometrical overlaps
const bool UserInput::list_particles=false; //option for listing the predefined particles
const bool UserInput::list_models=false; //option for listing predefined physics models
const long long UserInput::max_warnings=0; //maximum number of runtime warning to output (<0 for none)

//Condition for writing the data (used in RecorderBase)
bool UserInput::OutputCondition(DataCollection data) //DataCollection defined in RecorderBase.hh
{
  if (!emit_protons) {
    //bool np_proton=data.creator_volume=="Carbon" && data.particle=="proton" && data.tracked_p==true && data.tracked_12B==true;

    //return np_proton && ((data.partial_deposit>0*MeV && data.detector.substr(0,4)=="SILI") || data.detector=="Carbon"); //samo np-protoni
    
    return (data.partial_deposit>0*MeV && data.detector.substr(0,2)=="Si") || (data.detector=="Carbon" && data.particle=="proton");
  } else {
    return (data.partial_deposit>0*MeV && data.detector.substr(0,2)=="Si");
    
    //return (data.partial_deposit>0*MeV && data.detector.substr(0,2)=="Si") || data.detector=="Carbon";
  }
}

//Excited levels of 12B
const double UserInput::LEVEL[50]={0.000000*MeV,0.953140*MeV,1.673650*MeV,2.620800*MeV,2.723000*MeV,3.389100*MeV,3.759000*MeV,4.301000*MeV,4.460000*MeV,4.518000*MeV,
			5.000000*MeV,5.612000*MeV,5.726000*MeV,6.000000*MeV,6.600000*MeV,7.060000*MeV,7.545000*MeV,7.670000*MeV,7.700000*MeV,7.836000*MeV,
			7.937000*MeV,8.100000*MeV,8.120000*MeV,8.240000*MeV,8.376000*MeV,8.580000*MeV,8.707000*MeV,9.040000*MeV,9.175000*MeV,9.430000*MeV,
			9.585000*MeV,9.758000*MeV,9.830000*MeV,10.000000*MeV,10.110000*MeV,10.220000*MeV,10.435000*MeV,10.590000*MeV,10.900000*MeV,11.080000*MeV,
			11.310000*MeV,11.590000*MeV,12.345000*MeV,12.750000*MeV,13.330000*MeV,13.400000*MeV,14.820000*MeV,15.500000*MeV,21.800000*MeV,23.900000*MeV};
