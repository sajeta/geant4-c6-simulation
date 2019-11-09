#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

class G4LogicalVolume;
class RecorderBase;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction(RecorderBase*);
    ~DetectorConstruction();

    G4VPhysicalVolume* Construct();
    
    void Configuration_0(G4LogicalVolume*,G4ThreeVector&);
    void Configuration_1(G4LogicalVolume*,G4ThreeVector&);
    void Configuration_2(G4LogicalVolume*,G4ThreeVector&);
    void Configuration_3(G4LogicalVolume*,G4ThreeVector&);
    void Configuration_4(G4LogicalVolume*,G4ThreeVector&);
    void Configuration_5(G4LogicalVolume*,G4ThreeVector&);
    
private:
    void ReportAsSensitive(G4LogicalVolume*);
    
    RecorderBase* recorder;
};

#endif

