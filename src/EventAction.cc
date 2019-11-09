#include "EventAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"
#include "WarningSuppressor.hh"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

long long EventAction::total_events=0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RecorderBase* r) : G4UserEventAction()
{
  recorder=r;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

long long EventAction::GetTotalEvents()
{
  return total_events;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::ResetTotalEvents()
{
  total_events=0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* event)
{
  total_events++;
  
  SteppingAction::BeginOfEvent();
  
  WarningSuppressor::SuppressWarning((void*)event);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{
  SteppingAction::EndOfEvent();

  //Progress writeout
  if (UserInput::GetEventToReport()>0 && total_events%UserInput::GetEventToReport()==0) {
    if (!recorder->GetHostileTakeover()) {
      G4cout<<"Processed events: "<<total_events<<"  (detected: "<<recorder->GetAcceptedEvents()<<")"<<G4endl; 
    } else {
      G4String portion;
      double completion;
      
      if (UserInput::CountDetected()) {
	completion=100.*recorder->GetAcceptedEvents()/(double)UserInput::GetEvents();
      } else {
	completion=100.*EventAction::GetTotalEvents()/(double)UserInput::GetEvents();
      }
      
      ostringstream *buffer=new ostringstream;
      buffer->setf(ios::fixed,ios::floatfield);
      buffer->precision(2);
      (*buffer)<<completion;
      portion=(*buffer).str();
      delete buffer;

      G4cout<<"Processed events: "<<total_events<<"  (detected: "<<recorder->GetAcceptedEvents()<<")   ["<<portion<<"%]"<<G4endl;
    }
  }
  
  WarningSuppressor::SuppressWarning((void*)event);
}
