#include "WarningSuppressor.hh"

void* WarningSuppressor::dummy;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WarningSuppressor::WarningSuppressor()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WarningSuppressor::~WarningSuppressor()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WarningSuppressor::SuppressWarning(void* package)
{
  dummy=package;
}
