#ifndef __LOGSTATEMENT_H
#define __LOGSTATEMENT_H

#include "enums.h"
#include "params.h"
#include <string>

void logmsg(eLogLevel level, std::string s, const params & p);
void logverbatim(eLogLevel level, std::string s, const params & p);

std::string getheader();

class dServiceLogger
{
public:
   dServiceLogger(bool cerr, const params & p);
   void log(const char * const buf, int n);

private:
   void init();
   void finish();
   void processbuffer(std::string buffer);
   void nonrawoutput(std::string s);

   bool mCErr;
   const params & mP;
   bool mInitialised;
};

void fatal(std::string s);

#endif
