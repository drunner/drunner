#ifndef __LOGSTATEMENT_H
#define __LOGSTATEMENT_H

#include "enums.h"
#include "params.h"
#include <string>

void logmsg(eLogLevel level, std::string s, const params & p);
void logverbatim(eLogLevel level, std::string s, const params & p);

std::string getheader();

enum eStage
{
   kSearching,
   kGotEscape,
   kTriggered
};

class escapefilter
{
public:
   escapefilter();
   void strip(std::string & buffer);
   eStage mStage;
   size_t mPos;
};

class dServiceLogger
{
public:
   dServiceLogger(bool cerr, const params & p, bool isServiceCmd);
   void log(const char * const buf, int n);

private:
   void init();
   void finish();
   void processbuffer(std::string buffer);
   void nonrawoutput(std::string s);

   edServiceOutput mOutputMode;

   bool mCErr;
   const params & mP;
   bool mInitialised;
   escapefilter mEscapeFilter;
};

void fatal(std::string s);

#endif
