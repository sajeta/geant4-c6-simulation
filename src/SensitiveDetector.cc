#include "SensitiveDetector.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"
#include "WarningSuppressor.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"
#include "G4VTouchable.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4ThreeVector.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SensitiveDetector::SensitiveDetector(G4String name,RecorderBase* r) : G4VSensitiveDetector(name)
{
  detector=name;
  recorder=r;

  G4cout<<"Adding sensitive volume: "<<detector<<G4endl;
  detector_index=SteppingAction::ReportNewSensitive(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SensitiveDetector::~SensitiveDetector()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4String SensitiveDetector::GetName()
{
  return detector;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SensitiveDetector::Initialize (G4HCofThisEvent* HC)
{
  WarningSuppressor::SuppressWarning(HC);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool SensitiveDetector::ProcessHits(G4Step* step,G4TouchableHistory* history)
{
  //Tracking may be performed here, inside the sensitive detector (and usually is)!
  //But, as a more complex and more general example, let's assume that we need to
  //track the particles throughout all volumes (that is, including non-sensitives)
  //and only record the energy deposited (and any other relevant info) inside the
  //sensitives. In this case the tracking needs to be performed in the SteppingAction
  //and it's best just to send the appropriate signals from here, because the
  //data related only to sensitives may have to be synchronyzed with all the other
  //data (from other volumes).

  SteppingAction::ReportDetection(detector_index,step);
  
  WarningSuppressor::SuppressWarning(history);
  
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SensitiveDetector::EndOfEvent(G4HCofThisEvent* HC)
{
  WarningSuppressor::SuppressWarning(HC);
}
