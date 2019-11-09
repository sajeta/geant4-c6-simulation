#include "UserInput.hh"

#include "ExceptionHandler.hh"
#include "G4ExceptionHandler.hh"

long long ExceptionHandler::exceptions=0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ExceptionHandler::ExceptionHandler(G4ExceptionHandler* handler)
{
  default_handler=handler;
  //Must be delivered from outside because the last handler initialized is the one that takes over,
  //so it must not be initialized inside our ExceptionHandler, since this would make the default one
  //initialized later than our own.
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ExceptionHandler::~ExceptionHandler()
{
  delete default_handler;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool ExceptionHandler::Notify(const char *originOfException,const char *exceptionCode,G4ExceptionSeverity severity,const char *description)
{
  if (UserInput::GetMaxWarnings()>=0 && severity==JustWarning) {
    if (exceptions<UserInput::GetMaxWarnings()) {
      //Allow message
      default_handler->Notify(originOfException,exceptionCode,severity,description);

      exceptions++;
      if (exceptions>=UserInput::GetMaxWarnings()) {
	G4cout<<"Maximum number of warnings reached! No more warnings will be shown."<<G4endl<<G4endl;
      } 
    } 
    return false;
  } else {
    //Allow message
    return default_handler->Notify(originOfException,exceptionCode,severity,description);
  }
}
