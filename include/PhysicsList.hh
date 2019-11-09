#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"

class RecorderBase;

class PhysicsList: public G4VModularPhysicsList
{
public:
  PhysicsList(RecorderBase*);
  ~PhysicsList();
  
  void ConstructParticle();
  void ConstructProcess();
  void SetCuts();
  
private:
  void GeneralPhysics();
  void NeutronPhysics();
  
  RecorderBase *recorder;
};

#endif

