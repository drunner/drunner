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


std::string params::substitute( const std::string & source ) const
{
   std::string d(source);
   d=utils::replacestring(d,"${VERSION}",mVersion);
   d=utils::replacestring(d,"${EXENAME}","drunner");
   d=utils::replacestring(d,"${ROOTPATH}", drunnerPaths::getPath_Exe().toString());
   return d;
}

void params::parsecmd()
{
   std::map<std::string, eCommand> commandlist;

   commandlist["clean"] = c_clean;
   commandlist["list"] = c_list;
   commandlist["update"] = c_setup;
   commandlist["setup"] = c_setup;
   commandlist["checkimage"] = c_checkimage;
   commandlist["backup"] = c_backup;
   commandlist["restore"] = c_restore;
   commandlist["install"] = c_install;
   commandlist["recover"] = c_recover;
   commandlist["recreate"] = c_recover; // synonym for recover.
   commandlist["uninstall"] = c_uninstall;
   commandlist["obliterate"] = c_obliterate;
   commandlist["enter"] = c_enter;
   commandlist["status"] = c_status;
   commandlist["build"] = c_build;
   commandlist["unittest"] = c_unittest;
   commandlist["servicecmd"] = c_servicecmd;
   commandlist["help"] = c_help;
   commandlist["__save-environment"]  = c_saveenvironment;

   if (mCmdStr.compare(0, 10, "__plugin__") == 0)
   {
      mCmdStr.erase(0,10);
      mCmd = c_plugin;
   }
   else
   {
      auto it = commandlist.find(mCmdStr);
      if (it == commandlist.end())
         throw eExit("Unknown command \"" + mCmdStr + "\".");
      mCmd = it->second;
   }
}

void params::setdefaults()
{
   mVersion = getVersionStr();
   mLogLevel = kLINFO;
   
   mServiceOutput_hooks = kOLogged;
   mServiceOutput_servicecmd = kORaw;

   mCmd = c_UNDEFINED;

   mDevelopmentMode = false;
   mPause = false;
}

// Parse command line parameters.
params::params(int argc, char **argv)
{
setdefaults();

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
            {"logged",0,0,'l'},
            {"developer",0,0,'d'},
            {"pause",0,0,'p'},
//            {"create", 1, 0, 'c'},
            {0, 0, 0, 0}
         };

      // run getopt_long, hiding errors.
      extern int opterr;
      opterr = 0;
      c = getopt_long (argc, argv, "vsnoldp",
                     long_options, &option_index);

      if (c == -1) // no more options.
         break;

      switch (c)
      {
         case 's':
            mLogLevel=kLERROR;
            mServiceOutput_hooks = kOSuppressed;
            mServiceOutput_servicecmd = kOSuppressed;
            mOptions.push_back("-s");
            break;

         case 'v':
            mLogLevel=kLDEBUG;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kORaw;
            mOptions.push_back("-v");
            break;

         case 'o':
            mLogLevel=kLERROR;
            mServiceOutput_hooks = kORaw;
            mServiceOutput_servicecmd = kORaw;
            mOptions.push_back("-o");
            break;

         case 'n':
            mLogLevel=kLINFO;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kORaw;
            mOptions.push_back("-n");
            break;

         case 'l':
            mLogLevel = kLINFO;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kOLogged;
            mOptions.push_back("-l");
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
      parsecmd();
   }

   // store the arguments to the command.
   for (int i=opx;i<argc;++i)
      mArgs.push_back(argv[i]);
}

const std::string & params::getArg(int n) const 
{ 
   poco_assert(n < (int)mArgs.size());
   return mArgs[n]; 
}
