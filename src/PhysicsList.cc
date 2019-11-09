#include "PhysicsList.hh"
#include "RecorderBase.hh"
#include "UserInput.hh"
#include "WarningSuppressor.hh"

#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4SpinDecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4IonElasticPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4EmExtraPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4ChargeExchangePhysics.hh"
#include "G4OpticalPhysics.hh"

#include "G4BaryonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4Neutron.hh"
#include "G4ProcessManager.hh"

#include "G4HadronElasticProcess.hh"
#include "G4NeutronInelasticProcess.hh"
#include "G4HadronFissionProcess.hh"
#include "G4HadronCaptureProcess.hh"

#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPInelastic.hh"
#include "G4NeutronHPThermalScattering.hh"
#include "G4NeutronHPFission.hh"
#include "G4ParaFissionModel.hh"
#include "G4NeutronHPCapture.hh"
#include "G4HadronElastic.hh"
#include "G4BinaryCascade.hh"
#include "G4CascadeInterface.hh"
#include "G4INCLXXInterface.hh"
#include "G4AblaInterface.hh"
#include "G4LFission.hh"
#include "G4NeutronRadCapture.hh"
#include "G4TheoFSGenerator.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4QGSMFragmentation.hh"
#include "G4QGSModel.hh"
#include "G4QGSParticipants.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4PreCompoundModel.hh"
#include "G4ExcitationHandler.hh"
#include "G4LundStringFragmentation.hh"
#include "G4FTFModel.hh"

#include "G4NeutronHPElasticData.hh"
#include "G4HadronElasticDataSet.hh"
#include "G4BGGNucleonElasticXS.hh"
#include "G4NeutronHPJENDLHEElasticData.hh"
#include "G4NeutronHPInelasticData.hh"
#include "G4NeutronInelasticCrossSection.hh"
#include "G4BGGNucleonInelasticXS.hh"
#include "G4NeutronHPJENDLHEInelasticData.hh"
#include "G4NeutronHPThermalScatteringData.hh"
#include "G4NeutronHPFissionData.hh"
#include "G4HadronFissionDataSet.hh"
#include "G4NeutronHPCaptureData.hh"
#include "G4HadronCaptureDataSet.hh"

#include <vector>
using namespace std;

PhysicsList::PhysicsList(RecorderBase* r) : G4VModularPhysicsList()
{ 
  SetVerboseLevel(1);
  recorder=r;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::~PhysicsList()
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructParticle()
{
  G4BaryonConstructor *baryons=new G4BaryonConstructor();
  G4LeptonConstructor *leptons=new G4LeptonConstructor();
  G4BosonConstructor *bosons=new G4BosonConstructor();
  G4IonConstructor *ions=new G4IonConstructor();
  G4MesonConstructor *mesons=new G4MesonConstructor();
  G4ShortLivedConstructor *exotic=new G4ShortLivedConstructor();

  baryons->ConstructParticle();
  leptons->ConstructParticle();
  bosons->ConstructParticle();
  ions->ConstructParticle();
  mesons->ConstructParticle();
  exotic->ConstructParticle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructProcess()
{
  AddTransportation();
  GeneralPhysics();
  NeutronPhysics();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCuts()
{
  G4VModularPhysicsList::SetCuts();
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::GeneralPhysics()
{
  vector<G4VPhysicsConstructor*> general_physics;
  general_physics.push_back(new G4EmStandardPhysics());
  general_physics.push_back(new G4DecayPhysics());
  general_physics.push_back(new G4RadioactiveDecayPhysics());
  general_physics.push_back(new G4SpinDecayPhysics());
  general_physics.push_back(new G4IonElasticPhysics());
  general_physics.push_back(new G4IonPhysics());
  general_physics.push_back(new G4EmExtraPhysics());
  general_physics.push_back(new G4StoppingPhysics());
  general_physics.push_back(new G4HadronElasticPhysics());
  general_physics.push_back(new G4HadronInelasticQBBC()); //A LOT of alternatives
  //general_physics.push_back(new G4ChargeExchangePhysics());
  //general_physics.push_back(new G4OpticalPhysics()); //WARNING Do not activate unless neccessary!
  for(unsigned int i=0; i<general_physics.size(); i++) {
    general_physics[i]->ConstructProcess();
  }
  general_physics.clear(); //just clear the pointers
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::NeutronPhysics()
{
  G4ParticleDefinition* neutron=G4Neutron::NeutronDefinition();
  G4ProcessManager *manager=neutron->GetProcessManager();
  G4ProcessVector *process_list=manager->GetProcessList();
  
  G4HadronElasticProcess *theElasticProcess=new G4HadronElasticProcess();
  G4NeutronInelasticProcess *theInelasticProcess=new G4NeutronInelasticProcess();
  G4HadronFissionProcess *theFissionProcess=new G4HadronFissionProcess(); 
  G4HadronCaptureProcess *theCaptureProcess=new G4HadronCaptureProcess();

  //Removing previously assigned processes
  G4String process_name;
  for (int i=0; i<process_list->size(); i++) {
    process_name=(*process_list)[i]->GetProcessName();

    if (process_name==theElasticProcess->GetProcessName() || process_name==theInelasticProcess->GetProcessName() || process_name==theFissionProcess->GetProcessName() || process_name==theCaptureProcess->GetProcessName()) {
      manager->RemoveProcess(i--); //The size of process list changes and following processes are shifted back!
    }
  }

  //Elastic scattering
  G4HadronElastic *theHEElasticModel=new G4HadronElastic();
  G4NeutronHPElastic *theElasticModel=new G4NeutronHPElastic();
  G4NeutronHPThermalScattering* theThermalElasticModel = new G4NeutronHPThermalScattering();
  //G4HadronElasticDataSet *theVHEElasticData=new G4HadronElasticDataSet(); //Alternative to G4BGGNucleonElasticXS
  G4BGGNucleonElasticXS *theVHEElasticData=new G4BGGNucleonElasticXS(G4Neutron::Neutron()); //Alternative to G4HadronElasticDataSet
  G4NeutronHPJENDLHEElasticData *theHEElasticData=new G4NeutronHPJENDLHEElasticData();
  G4NeutronHPElasticData *theElasticData=new G4NeutronHPElasticData();
  G4NeutronHPThermalScatteringData* theThermalElasticData = new G4NeutronHPThermalScatteringData();
  theElasticProcess->AddDataSet(theVHEElasticData); //Fisrt!
  theElasticProcess->AddDataSet(theHEElasticData); //Second!
  theElasticProcess->AddDataSet(theElasticData); //Third!
  theElasticProcess->AddDataSet(theThermalElasticData); //Fourth!
  theThermalElasticModel->SetMaxEnergy(4.*eV);
  theElasticModel->SetMinEnergy(4.*eV);
  theElasticModel->SetMaxEnergy(20.*MeV);
  theHEElasticModel->SetMinEnergy(20.*MeV);
  theElasticProcess->RegisterMe(theThermalElasticModel);
  theElasticProcess->RegisterMe(theElasticModel);
  theElasticProcess->RegisterMe(theHEElasticModel);
  manager->AddDiscreteProcess(theElasticProcess);
  
  //Inelastic scattering
  //Very high energies
    //QGSP model
    G4TheoFSGenerator *QGSPmodel=new G4TheoFSGenerator("QGSP");
    G4GeneratorPrecompoundInterface *QGSPcascade=new G4GeneratorPrecompoundInterface();
    G4ExcitationHandler *QGSPhandler=new G4ExcitationHandler();
    G4PreCompoundModel *QGSPpreEquilib=new G4PreCompoundModel(QGSPhandler);
      G4QGSMFragmentation *QGSPfragmentation=new G4QGSMFragmentation();
      G4QGSModel< G4QGSParticipants > *QGSPstringModel=new G4QGSModel< G4QGSParticipants >;
    G4ExcitedStringDecay *QGSPstringDecay=new G4ExcitedStringDecay(QGSPfragmentation);
    QGSPcascade->SetDeExcitation(QGSPpreEquilib);
    QGSPmodel->SetTransport(QGSPcascade);
    QGSPstringModel->SetFragmentationModel(QGSPstringDecay);
    QGSPmodel->SetHighEnergyGenerator(QGSPstringModel);
    //FTFP model
    G4TheoFSGenerator *FTFPmodel=new G4TheoFSGenerator("FTFP");
    G4GeneratorPrecompoundInterface *FTFPcascade=new G4GeneratorPrecompoundInterface();
    G4ExcitationHandler *FTFPhandler=new G4ExcitationHandler();
    G4PreCompoundModel *FTFPpreEquilib=new G4PreCompoundModel(FTFPhandler);
      G4LundStringFragmentation *FTFPfragmentation=new G4LundStringFragmentation();
      G4FTFModel *FTFPstringModel=new G4FTFModel();
    G4ExcitedStringDecay *FTFPstringDecay=new G4ExcitedStringDecay(FTFPfragmentation);
    FTFPcascade->SetDeExcitation(FTFPpreEquilib);
    FTFPmodel->SetTransport(FTFPcascade);
    FTFPstringModel->SetFragmentationModel(FTFPstringDecay);
    FTFPmodel->SetHighEnergyGenerator(FTFPstringModel);
  //Intermediate energies
  G4CascadeInterface *bertiniInelasticModel=new G4CascadeInterface();
  G4BinaryCascade *binaryInelasticModel=new G4BinaryCascade(); 
    //INCL+ABLA
    G4INCLXXInterface *INCLmodel=new G4INCLXXInterface();
    G4AblaInterface *theAblaInterface=new G4AblaInterface();
    INCLmodel->SetDeExcitation(theAblaInterface);
    //INCL+ABLA
  //Low energies
  G4NeutronHPInelastic *theInelasticModel=new G4NeutronHPInelastic();
  G4NeutronHPThermalScattering *theThermalInelasticModel=new G4NeutronHPThermalScattering();
  //G4NeutronInelasticCrossSection *theVHEInelasticData=new G4NeutronInelasticCrossSection(); //Alternative to G4BGGNucleonInelasticXS
  G4BGGNucleonInelasticXS *theVHEInelasticData= new G4BGGNucleonInelasticXS(G4Neutron::Neutron()); //Alternative to G4NeutronInelasticCrossSection
  G4NeutronHPJENDLHEInelasticData *theHEInelasticData=new G4NeutronHPJENDLHEInelasticData();
  G4NeutronHPInelasticData *theInelasticData=new G4NeutronHPInelasticData();
  G4NeutronHPThermalScatteringData* theThermalInelasticData = new G4NeutronHPThermalScatteringData();
  theInelasticProcess->AddDataSet(theVHEInelasticData); //First!
  theInelasticProcess->AddDataSet(theHEInelasticData); //Second!
  theInelasticProcess->AddDataSet(theInelasticData); //Third!
  theInelasticProcess->AddDataSet(theThermalInelasticData); //Fourth!
  theThermalInelasticModel->SetMaxEnergy(4.*eV);
  theInelasticModel->SetMinEnergy(4.*eV);
  theInelasticModel->SetMaxEnergy(20.*MeV); 

  INCLmodel->SetMinEnergy(0.*MeV);
  INCLmodel->SetMaxEnergy(1.*GeV);
  
  binaryInelasticModel->SetMinEnergy(0.*GeV);
  binaryInelasticModel->SetMaxEnergy(10.*GeV);
  
  bertiniInelasticModel->SetMinEnergy(20.*MeV);
  bertiniInelasticModel->SetMaxEnergy(10.*GeV);
  
  QGSPmodel->SetMinEnergy(0.*GeV);
  QGSPmodel->SetMaxEnergy(100.*TeV);
  
  FTFPmodel->SetMinEnergy(10.*GeV);
  FTFPmodel->SetMaxEnergy(100.*TeV);
  theInelasticProcess->RegisterMe(theThermalInelasticModel);
  theInelasticProcess->RegisterMe(theInelasticModel);
  //theInelasticProcess->RegisterMe(INCLmodel);
  //theInelasticProcess->RegisterMe(binaryInelasticModel);
  theInelasticProcess->RegisterMe(bertiniInelasticModel);
  //theInelasticProcess->RegisterMe(QGSPmodel);
  theInelasticProcess->RegisterMe(FTFPmodel);
  manager->AddDiscreteProcess(theInelasticProcess);
 
  //Fission
  G4LFission *theHEFissionModel=new G4LFission();
  //G4NeutronHPFission *theFissionModel=new G4NeutronHPFission(); //WARNING Does not create fragments; does create delayed neutrons!
  G4ParaFissionModel *theFissionModel=new G4ParaFissionModel(); //WARNING Does create fragments; does not create delayed neutrons!
  G4HadronFissionDataSet *theHEFissionData=new G4HadronFissionDataSet();
  G4NeutronHPFissionData *theFissionData=new G4NeutronHPFissionData();
  theFissionProcess->AddDataSet(theHEFissionData); //First!
  theFissionProcess->AddDataSet(theFissionData); //Second!
  theFissionModel->SetMaxEnergy(60.*MeV); //60 MeV for G4ParaFissionModel; 20 MeV for G4NeutronHPFission
  theHEFissionModel->SetMinEnergy(60.*MeV);
  theFissionProcess->RegisterMe(theFissionModel);
  theFissionProcess->RegisterMe(theHEFissionModel);
  manager->AddDiscreteProcess(theFissionProcess);
  
  //Capture
  G4NeutronRadCapture *theHECaptureModel=new G4NeutronRadCapture();
  G4NeutronHPCapture *theCaptureModel=new G4NeutronHPCapture();
  G4HadronCaptureDataSet *theHECaptureData=new G4HadronCaptureDataSet();
  G4NeutronHPCaptureData *theCaptureData=new G4NeutronHPCaptureData();
  theCaptureProcess->AddDataSet(theHECaptureData); //First!
  theCaptureProcess->AddDataSet(theCaptureData); //Second!
  theCaptureModel->SetMaxEnergy(20.*MeV);
  theHECaptureModel->SetMinEnergy(20.*MeV);
  theCaptureProcess->RegisterMe(theCaptureModel);
  theCaptureProcess->RegisterMe(theHECaptureModel);
  manager->AddDiscreteProcess(theCaptureProcess);
  
  WarningSuppressor::SuppressWarning(theThermalInelasticModel);
  WarningSuppressor::SuppressWarning(theInelasticModel);
}
