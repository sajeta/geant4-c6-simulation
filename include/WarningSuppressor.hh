#ifndef WarningSuppressor_h
#define WarningSuppressor_h 1

class WarningSuppressor
{
public:
  WarningSuppressor();
  ~WarningSuppressor();
  static void SuppressWarning(void*);
  
private:
  static void* dummy;
};

#endif
