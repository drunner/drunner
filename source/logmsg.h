#ifndef __LOGSTATEMENT_H
#define __LOGSTATEMENT_H

#include "enums.h"
#include "params.h"
#include <string>

void logmsg(eLogLevel level, std::string s, const params & p);
void logverbatim(eLogLevel level, std::string s, const params & p);

class dServiceLogger
{
public:
   dServiceLogger(bool cerr, const params & p);
   ~dServiceLogger();
   void log(const char * const buf, int n);

private:
   void init();
   void finish();

   bool mCErr;
   const params & mP;
   bool mInitialised;
};

void fatal(std::string s);

#endif
