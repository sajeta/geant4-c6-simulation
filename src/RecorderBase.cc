#include "EventAction.hh"
#include "RecorderBase.hh"
#include "PrimaryGeneratorAction.hh"
#include "UserInput.hh"

#include "G4SystemOfUnits.hh"

#include <time.h> 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RecorderBase::RecorderBase(G4String output_address)
{
  time_t timer;
  time(&timer);
  simulation_id=ctime(&timer);
  G4cout<<"Simulation time: "<<simulation_id<<G4endl;

  address=output_address.substr(0,output_address.find_last_of('/'));
  output_address=UserInput::GetOutputLocation();
  if (output_address(0)!='/') {address+="/";}
  address+=output_address;
  if (output_address(output_address.length()-1)!='/') {address+="/";}
  G4cout<<"Output address: "<<address<<G4endl<<G4endl;
  
  written_counts=0;
  accepted_events=0;
  last_event=-1;
  hostile_takeover=false;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RecorderBase::~RecorderBase()
{
  CloseFile();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RecorderBase::ReportHostileTakeover(bool automatic,bool hostile)
{
  hostile_takeover=hostile;
  EventAction::ResetTotalEvents();
  
  if (automatic) {
    if (hostile_takeover) {
      OpenFile();
    } else {
      CloseFile();
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool RecorderBase::PrepareForOutput()
{
  if (UserInput::GetOutputOption()) {
    ifstream in;
    ostringstream *buffer;
    int file_counter=1;
    do {
      buffer=new ostringstream;
      (*buffer)<<file_counter;
      filename=address+UserInput::GetOutputName()+(*buffer).str()+".dat";
      delete buffer;
      in.open(filename.c_str());
      if (in.is_open()) {
	file_counter++;
	in.close();
      } else {
	in.close();
	break;
      }
    } while (true);
    return true;
  } else {
    return false;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RecorderBase::OpenFile()
{
  CloseFile();
  
  written_counts=0;
  accepted_events=0;
  last_event=-1;
  next_flush=UserInput::GetMaxRAMEntries();
  
  if (hostile_takeover && PrepareForOutput()) {
    out.open(filename.c_str());
    if (out.is_open()) {
      G4cout<<"\nOpened: "<<filename<<G4endl<<G4endl;
    } else {
      out.close();
      G4cout<<"\nERROR: Can not open: "<<filename<<G4endl;
      G4cout<<"       Exiting now!"<<G4endl<<G4endl;
      exit(0);
    }

    if (UserInput::GetOutputPrecision()>0 && !UserInput::ReducedOutput()) {
      out.setf(ios::scientific,ios::floatfield);
      out.precision(UserInput::GetOutputPrecision());
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RecorderBase::CloseFile()
{
  if (out.is_open()) {
    out.close();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RecorderBase::ReportEvent(DataCollection data)
{
  if (UserInput::OutputCondition(data)) {
    //Counting accepted
    if (EventAction::GetTotalEvents()!=last_event) {
      accepted_events++;
      last_event=EventAction::GetTotalEvents();
    }
  
    //Writeout
    if (out.is_open()) {

      //ADD OR REMOVE DATA AS REQUIRED
      if (!UserInput::ReducedOutput()) {
	//Full output
	out<<EventAction::GetTotalEvents()<<" "<<PrimaryGeneratorAction::GetInitialEnergy()/MeV<<" "
	   <<data.detector<<" "<<data.creator_volume<<" "<<data.particle<<" "
	   <<data.partial_deposit/MeV<<" "<<data.total_deposit/MeV<<" "
	   <<data.tracked_n<<" "<<data.tracked_p<<" "<<data.tracked_d<<" "<<data.tracked_12B<<" "<<data.tracked_13B<<" "
	   <<data.initial_energy/MeV<<" "<<data.initial_P.z()<<"\n";
      } else {
	//Reduced output
	out<<EventAction::GetTotalEvents()<<" "<<PrimaryGeneratorAction::GetInitialEnergy()/MeV<<" "
	   <<data.detector<<" "<<data.total_deposit/MeV<<" "<<data.initial_energy/MeV<<" "<<data.initial_P.z()<<" "<<data.proton_cos_CM<<"\n";
      }
      
      written_counts++;
      if (UserInput::GetMaxRAMEntries()>0 && written_counts>=next_flush) {
	out.flush();
	next_flush=UserInput::GetMaxRAMEntries()*(1+(long long)(written_counts/UserInput::GetMaxRAMEntries()));
      }
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

long long RecorderBase::GetAcceptedEvents()
{
  return accepted_events;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool RecorderBase::GetHostileTakeover()
{
  return hostile_takeover;
}
