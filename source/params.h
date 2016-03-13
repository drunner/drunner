#ifndef __PARAMS_H
#define __PARAMS_H

#include <vector>
#include <string>

#include "enums.h"

class params {
public:
   params(int argc, char **argv);
   params(eLogLevel ll);
   std::string substitute( const std::string & source ) const;

   const std::string & getVersion() const { return mVersion; }
   eCommand getCommand() const { return mCmd; }
   eLogLevel getLogLevel() const { return mLogLevel; }
   bool getDisplayServiceOutput() const { return mDisplayServiceOutput; }
   bool getServiceOutputRaw() const { return mServiceOutputRaw; }
   const std::vector<std::string> & getArgs() const { return mArgs; }
   int numArgs() const { return mArgs.size(); }

   // implicit conversion to allow easy logging.
   operator eLogLevel() const { return mLogLevel; }

   std::string getOption() const { return mOption; }

private:
   std::string mVersion;
   eCommand mCmd;
   std::vector<std::string> mArgs;
   eLogLevel mLogLevel;
   bool mDisplayServiceOutput;
   bool mServiceOutputRaw;
   std::string mOption;
   eCommand parsecmd(std::string s) const;
   params();
   void setdefaults();
};

#endif
