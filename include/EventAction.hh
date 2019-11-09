#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class RecorderBase;

class EventAction : public G4UserEventAction
{
  public:
    EventAction(RecorderBase*);
    ~EventAction();
    
    void BeginOfEventAction(const G4Event* event);
    void EndOfEventAction(const G4Event* event);

    static long long GetTotalEvents();
    static void ResetTotalEvents();
    
  private: 
    RecorderBase *recorder;
    
    static long long total_events;
};

#endif

    
