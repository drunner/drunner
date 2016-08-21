#ifndef __PARAMS_H
#define __PARAMS_H

#include <vector>
#include <string>
#include <map>

#include "enums.h"

class params {
public:
   params(int argc, char **argv);
   std::string substitute( const std::string & source ) const;

   const std::string & getVersion() const { return mVersion; }
   eCommand getCommand() const { return mCmd; }
   std::string getCommandStr() const { return mCmdStr; }
   eLogLevel getLogLevel() const { return mLogLevel; }

   edServiceOutput supportCallMode() const { return mServiceOutput_supportcalls ? kORaw : kOSuppressed; }
   edServiceOutput serviceCmdMode() const { return mServiceOutput_servicecmd ? kORaw : kOSuppressed; }

   const std::vector<std::string> & getArgs() const { return mArgs; }
   int numArgs() const { return mArgs.size(); }
   const std::string & getArg(int n) const;
  
   const std::vector<std::string> & getOptions() const { return mOptions; }
   bool isDevelopmentMode() const { return mDevelopmentMode; }
   bool doPause() const { return mPause; }

   bool isdrunnerCommand(std::string c) const;
   bool isHook(std::string c) const;
   eCommand getdrunnerCommand(std::string c) const;

private:
   std::string mVersion;
   eCommand mCmd;
   std::string mCmdStr;
   std::vector<std::string> mArgs;
   eLogLevel mLogLevel;
   bool mDevelopmentMode;
   bool mPause;
   const std::map<std::string, eCommand> mCommandList;

   bool mServiceOutput_supportcalls;
   bool mServiceOutput_servicecmd;

   std::vector<std::string> mOptions;
   params();
   void _setdefaults();
   void _parse(int argc, char **argv);
};

#endif
