#ifndef __PARAMS_H
#define __PARAMS_H

#include <vector>
#include <string>

#include "enums.h"

class params {
public:
   params(int argc, char **argv);
   std::string substitute( const std::string & source ) const;

   const std::string & getVersion() const             {return mVersion;}
   eCommand getCommand() const                        {return mCmd;}
   eLogLevel getLogLevel() const                      {return mLogLevel;}
   bool getDisplayServiceOutput() const               {return mDisplayServiceOutput;}
   const std::vector<std::string> & getArgs() const   {return mArgs;}
   int numArgs() const                                {return mArgs.size();}

   // implicit conversion to allow easy logging.
   operator eLogLevel() const                         {return mLogLevel;}

   std::string getLogLevelOption() const;

private:
   std::string mVersion;
   eCommand mCmd;
   std::vector<std::string> mArgs;
   eLogLevel mLogLevel;
   bool mDisplayServiceOutput;
   eCommand parsecmd(std::string s) const;
   params();
};

#endif
