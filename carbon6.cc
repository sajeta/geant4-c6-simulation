#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "SensitiveDetector.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"
#include "WarningSuppressor.hh"
#include "ExceptionHandler.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4PhysListFactory.hh"
#include "G4ExceptionHandler.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

#include "Randomize.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>

using namespace std;
using namespace chrono;

G4String HumanTime(double);

int main(int argc,char** argv)
{
  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
  CLHEP::HepRandom::setTheSeed(time(0));
  
  // Construct the default run manager
  G4RunManager *runManager=new G4RunManager;
  
  ExceptionHandler *exceptions=new ExceptionHandler(new G4ExceptionHandler);
  RecorderBase *recorder=new RecorderBase((G4String)__FILE__);
  
  G4PhysListFactory factory;
  if (UserInput::ListModels()) {
    G4cout<<"AVAILABLE MODELS:"<<G4endl;
    for (unsigned int i=0; i<factory.AvailablePhysLists().size(); i++) {
      G4cout<<i<<".\t  "<<(factory.AvailablePhysLists())[i]<<endl;
    }
    G4cout<<G4endl;
  }
  
/*
  //ALTERNATIVE TO CUSTOM PHYSICS LIST: one of prearanged ones
  G4VModularPhysicsList* physics=factory.GetReferencePhysList("QGSP_BERT_HP_EMY");
  runManager->SetUserInitialization(physics);
*/
  
  //Set mandatory initialization classes
  runManager->SetUserInitialization(new DetectorConstruction(recorder));
  runManager->SetUserInitialization(new PhysicsList(recorder));
  runManager->SetUserAction(new PrimaryGeneratorAction(recorder));

  //Set user action classes
  runManager->SetUserAction(new SteppingAction(recorder));     
  runManager->SetUserAction(new EventAction(recorder));
  runManager->SetUserAction(new RunAction(recorder));
  
  // Initialize G4 kernel
  runManager->Initialize();

#ifdef G4VIS_USE
  // Initialize visualization
  G4VisManager* visManager=new G4VisExecutive;
  //G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  //G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();
#endif
  
  G4cout<<"  Using "<<UserInput::STATE<<". excited state: "<<UserInput::GetExcitedLevel()<<" MeV\n"<<G4endl;
  
  // Get the pointer to the User Interface manager
  G4UImanager* UImanager=G4UImanager::GetUIpointer();
  G4String command,fileName;
  
  if (argc!=1) {
    // batch mode
    command="/control/execute ";
    fileName=argv[1];
    UImanager->ApplyCommand(command+fileName);
  } else {
    // interactive mode : define UI session
#ifdef G4UI_USE
    G4UIExecutive* ui=new G4UIExecutive(argc,argv);
#ifdef G4VIS_USE
    UImanager->ApplyCommand("/control/execute vis.mac"); 
#endif
    
    G4cout<<"\n\n********************************"<<G4endl;
    G4cout<<"ATTENTION:"<<G4endl;
    G4cout<<"The main simulation process will"<<G4endl;
    G4cout<<"start only after 'exit' command."<<G4endl;
    G4cout<<"********************************\n\n"<<G4endl;
      
    ui->SessionStart();
    delete ui;
#endif
  }
  
#ifdef G4VIS_USE
  UImanager->ApplyCommand("/vis/disable 1");
  visManager->SetVerboseLevel(0);
#endif
  
  G4cout<<"\nSimulation started!\n"<<G4endl;
  recorder->ReportHostileTakeover();
  
  clock_t cpu_start=clock();
  auto real_start=steady_clock::now();
  
  const int max_int=2e9; //approximate limit of signed int
  long long still_needed=UserInput::GetEvents();
  while (still_needed>0) {
    if (still_needed>max_int) {still_needed=max_int;}
    
    runManager->BeamOn(still_needed);

    if (UserInput::CountDetected()) {
      //Count detected
      still_needed=UserInput::GetEvents()-recorder->GetAcceptedEvents();
    } else {
      //Count emitted
      still_needed=UserInput::GetEvents()-EventAction::GetTotalEvents();
    }
  }
    
  clock_t cpu_stop=clock();
  auto real_stop=steady_clock::now();
    
  G4cout<<G4endl<<"Processed events: "<<EventAction::GetTotalEvents()<<"  (detected: "<<recorder->GetAcceptedEvents()<<")   [100%]"<<G4endl<<G4endl;

  //Calculate elapsed time
  double cpu_time=(cpu_stop-cpu_start)/(double)CLOCKS_PER_SEC;
  double real_time=duration_cast<duration<double>>(real_stop-real_start).count();
  
  /*
  auto elapsed_us=duration_cast<microseconds>(real_stop-real_start);
  double real_time=elapsed_us.count()/1.e6;
  */
  
  G4cout<<"Elapsed CPU time:   "<<HumanTime(cpu_time)<<G4endl;
  G4cout<<"Elapsed REAL time:  "<<HumanTime(real_time)<<G4endl<<G4endl;
  
  //Deletings
  delete recorder;
  delete exceptions;

#ifdef G4VIS_USE
  delete visManager;
#endif
  delete runManager;

  return 0;
}

//###########################################################################################

G4String HumanTime(double s)
{
  const double S=1;
  const double M=60*S;
  const double H=60*M;
  const double D=24*H;
  const double W=7*D;

  const int UNITS=5;
  const double time_unit[UNITS]={W,D,H,M,S};
  const G4String unit_name[UNITS]={"weeks","days","hours","minutes","seconds"};
  
  G4String humanly_readable="";
  ostringstream *buffer;
  int unit;
  bool go_on=false;
  for (int i=0; i<UNITS-1; i++) {
    unit=s/time_unit[i];
    if (go_on || unit>=1) {
      buffer=new ostringstream;
      (*buffer)<<unit;
      humanly_readable+=(*buffer).str()+" "+unit_name[i]+", ";
      delete buffer;
      s-=unit*time_unit[i];
      go_on=true;
    }
  }
  
  //Final unit must always be output 
  buffer=new ostringstream;
  (*buffer)<<round(s/time_unit[UNITS-1]);
  humanly_readable+=(*buffer).str()+" "+unit_name[UNITS-1];
  delete buffer;
  
  return humanly_readable;
}
