#ifndef __DSERVICELOGGER_H
#define __DSERVICELOGGER_H

#include "enums.h"
#include "params.h"
#include <string>

#include <Poco/PipeStream.h>


static void dServiceLog(
   Poco::PipeInputStream & istrm_cout,
   Poco::PipeInputStream & istrm_cerr,
   bool isServiceCmd
);


//enum eStage
//{
//   kSearching,
//   kGotEscape,
//   kTriggered
//};
//
//class escapefilter
//{
//public:
//   escapefilter();
//   void strip(std::string & buffer);
//   eStage mStage;
//   size_t mPos;
//};
//
//class dServiceLogger
//{
//public:
//   dServiceLogger(bool cerr, bool isServiceCmd);
//   void log(const char * const buf, int n);
//
//private:
//   void processbuffer(std::string buffer);
//   void logverbatim(std::string s);
//
//   edServiceOutput mOutputMode;
//
//   bool mCErr;
//   bool mInitialised;
//   escapefilter mEscapeFilter;
//};



#endif
