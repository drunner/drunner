#ifndef __DSERVICELOGGER_H
#define __DSERVICELOGGER_H

#include "enums.h"
#include "params.h"
#include <string>

#include <Poco/PipeStream.h>


void dServiceLog(Poco::PipeInputStream & istrm_cout, bool isServiceCmd);


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
   bool valid(char c);
   eStage mStage;
};

#endif
