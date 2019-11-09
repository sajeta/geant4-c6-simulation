#ifndef ExceptionHandler_h
#define ExceptionHandler_h 1

#include "G4VExceptionHandler.hh"
#include "globals.hh"

class G4ExceptionHandler;

class ExceptionHandler : public G4VExceptionHandler
{
  public:
    ExceptionHandler(G4ExceptionHandler*);
    ~ExceptionHandler();

  private:
    bool Notify(const char*,const char *,G4ExceptionSeverity,const char*);
    
    static long long exceptions;
    
    G4ExceptionHandler *default_handler;
};

#endif

    
