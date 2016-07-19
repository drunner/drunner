#include <string>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <map>
#include <sstream>

#include "getopt.h"
#include "params.h"
#include "utils.h"
#include "globallogger.h"
#include "buildnum.h"
#include "exceptions.h"
#include "drunner_paths.h"
#include "dassert.h"

// Parse command line parameters.
params::params(int argc, char **argv) :
   mCommandList({
   {"clean",c_clean},
   {"list",c_list},
   {"checkimage",c_checkimage},
   {"unittest",c_unittest},
   {"help",c_help},
   {"install",c_install},
   {"uninstall",c_uninstall},
   {"obliterate",c_obliterate},
   {"update",c_setup},
   {"setup",c_setup},
   {"backup",c_backup},
   {"restore",c_restore},
   {"recreate",c_recreate},
   {"status",c_status},
   {"build",c_build},
   {"configure",c_configure},
   {"servicecmd",c_servicecmd}
   })
{
   _setdefaults();
   _parse(argc, argv);
}

std::string params::substitute( const std::string & source ) const
{
   std::string d(source);
   d=utils::replacestring(d,"${VERSION}",mVersion);
   d=utils::replacestring(d,"${EXENAME}","drunner");
   d=utils::replacestring(d,"${ROOTPATH}", drunnerPaths::getPath_Exe().toString());
   return d;
}

bool params::isdrunnerCommand(std::string c) const
{
   return (getdrunnerCommand(c) != c_UNDEFINED);
}

bool params::isHook(std::string c) const
{
   size_t pos = c.find_last_of('_');
   if (pos == std::string::npos) // no _
      return false;
   std::string tag = c;
   tag.erase(0, pos+1);
   c.erase(pos);

   if (utils::stringisame(tag, "start") || utils::stringisame(tag, "end"))
      return isdrunnerCommand(c);
   else
      return false; // tag is not start or end.
}

eCommand params::getdrunnerCommand(std::string c) const
{
   auto it = mCommandList.find(c);
   if (it == mCommandList.end())
      return c_UNDEFINED;
   if (utils::stringisame(it->first, c))
      return it->second;
   else
      return c_UNDEFINED;
}

void params::_setdefaults()
{
   mVersion = getVersionStr();

   mLogLevel = kLINFO;
   mServiceOutput_supportcalls = false;
   mServiceOutput_servicecmd = true;

   mCmd = c_UNDEFINED;

   mDevelopmentMode = false;
   mPause = false;
}



void params::_parse(int argc, char **argv)
{
// parse command line stuff.
int c;
while (1)
{
   int option_index = 0;
   static struct option long_options[] =
         { // name, has_arg, flag=0, val={0, character}
            {"verbose", 0, 0, 'v'},
            {"silent", 0, 0, 's'},
            {"getoutput", 0, 0, 'o'},
            {"normal", 0, 0, 'n'},
            {"developer",0,0,'d'},
            {"pause",0,0,'p'},
//            {"create", 1, 0, 'c'},
            {0, 0, 0, 0}
         };

      // run getopt_long, hiding errors.
      extern int opterr;
      opterr = 0;
      c = getopt_long (argc, argv, "vsnodp",
                     long_options, &option_index);

      if (c == -1) // no more options.
         break;

      switch (c)
      {
         case 's':
            mLogLevel=kLERROR;
            mServiceOutput_supportcalls = false;
            mServiceOutput_servicecmd = false;
            mOptions.push_back("-s");
            break;

         case 'v':
            mLogLevel=kLDEBUG;
            mServiceOutput_supportcalls = true;
            mServiceOutput_servicecmd = true;
            mOptions.push_back("-v");
            break;

         case 'o':
            mLogLevel=kLERROR;
            mServiceOutput_supportcalls = false;
            mServiceOutput_servicecmd = true;
            mOptions.push_back("-o");
            break;

         case 'n':
            mLogLevel=kLINFO;
            mServiceOutput_supportcalls = false;
            mServiceOutput_servicecmd = true;
            mOptions.push_back("-n");
            break;

         case 'd':
            mDevelopmentMode = true;
            mOptions.push_back("-d");
            break;

         case 'p':
            mPause = true;
            break;

         default:
            throw eExit("Unrecognised option."); //" -" + std::string(1,c));
      }
   }

   if (mOptions.size() == 0)
      mOptions.push_back("-n");

   int opx=optind;
   if (opx >= argc) // drunner with no command.
   {
      mCmdStr = "help";
      mCmd = c_help;
   }
   else
   {
      mCmdStr = argv[opx++];
      std::transform(mCmdStr.begin(), mCmdStr.end(), mCmdStr.begin(), ::tolower);

      // store the arguments to the command. getopt_long permeates the args so all non-options are at end.
      for (int i = opx; i<argc; ++i)
         mArgs.push_back(argv[i]);

      if (mCmdStr.compare(0, 10, "__plugin__") == 0)
      {
         mCmdStr.erase(0, 10);
         mCmd = c_plugin;
      }
      else
         mCmd = getdrunnerCommand(mCmdStr);
   }

}

const std::string & params::getArg(int n) const 
{ 
   poco_assert(n < (int)mArgs.size());
   return mArgs[n]; 
}
