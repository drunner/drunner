#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>

#include <Poco/StreamCopier.h>

#include "dservicelogger.h"
#include "globallogger.h"
#include "globalcontext.h"

// remove escape sequences from string.
void stripescapes(std::string &s)
{
   while (true)
   { // breakable loop
      size_t pos = s.find('\033');
      if (pos == std::string::npos)
         return;
      s.erase(pos, 1);
      if (s.length() > pos && s[pos] == '[')
      {
         size_t epos = s.find('m', pos);
         if (epos != std::string::npos)
            s.erase(pos, epos - pos + 1);
      }
   }
}

void dServiceLog(Poco::PipeInputStream & istrm_cout, bool isServiceCmd)
{
   edServiceOutput mOutputMode;

   if (isServiceCmd)
      mOutputMode = GlobalContext::getParams()->getServiceOutput_servicecmd();
   else
      mOutputMode = GlobalContext::getParams()->getServiceOutput_hooks();

   if (mOutputMode == kOSuppressed)
      return; // nothing to output :-)

   if (mOutputMode == kORaw)
   {
      Poco::StreamCopier::copyStreamUnbuffered(istrm_cout, std::cout);
   }
   else
   {
      escapefilter ef;
      char buf[2] = { 'x',0 };
      bool initialised = false;

      while (true)
      {
         int i = istrm_cout.get();
         if (i == -1)
            return;
         char c = (char)i;

         if (ef.valid(c))
         { // output c.
            if (!initialised)
               logverbatim(kLINFO, getheader(kLINFO));
            initialised = true;
            buf[0] = c;
            logverbatim(kLINFO, buf);
            if (c == '\n')
               initialised = false;
         }
      }
   }
}

escapefilter::escapefilter() : mStage(kSearching)
{
}

bool escapefilter::valid(char c)
{   
   switch (mStage)
   {
   case kSearching:
      if (c != '\033')
         return true;
      mStage = kGotEscape;
      return false;

   case kGotEscape:
      if (c != '[')
      {
         mStage = kSearching;
         return true;
      }
      mStage = kTriggered;
      return false;

   default: // kTriggered
      if (c != 'm')
         return false;
      mStage = kSearching;
      return false;
   }
}