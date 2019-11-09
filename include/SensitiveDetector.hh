#ifndef SensitiveDetector_h
#define SensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include "globals.hh"

#include <vector>
using namespace std;

class RecorderBase;

class SensitiveDetector : public G4VSensitiveDetector
{
public:
  
  SensitiveDetector(G4String,RecorderBase*);
  ~SensitiveDetector();
  
  void Initialize (G4HCofThisEvent*);
  bool ProcessHits(G4Step*,G4TouchableHistory*);
  void EndOfEvent(G4HCofThisEvent*);
  
  G4String GetName();

private:
  G4String detector;
  unsigned int detector_index;
  RecorderBase *recorder;
};

#endif
