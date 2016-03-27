#include <string>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <sstream>

#include "params.h"
#include "utils.h"
#include "showhelp.h"
#include "logmsg.h"
#include "buildnum.h"


std::string params::substitute( const std::string & source ) const
{
   std::string d(source);
   d=utils::replacestring(d,"${VERSION}",mVersion);
   d=utils::replacestring(d,"${EXENAME}",utils::get_exename());
   d=utils::replacestring(d,"${ROOTPATH}",utils::get_exepath());
   //d=utils::replacestring(d,"${TIME}",__TIME__);
   //d=utils::replacestring(d,"${DATE}",__DATE__);
   return d;
}

eCommand params::parsecmd(std::string s) const
{
   std::transform(s.begin(), s.end(), s.begin(), ::tolower);

   std::map<std::string, eCommand> commandlist;
   std::map<std::string, eCommand>::iterator it;

   commandlist["setup"] = c_setup;
   commandlist["clean"] = c_clean;
   commandlist["list"] = c_list;
   commandlist["update"] = c_update;
   commandlist["checkimage"] = c_checkimage;
   commandlist["backup"] = c_backup;
   commandlist["restore"] = c_restore;
   commandlist["install"] = c_install;
   commandlist["recover"] = c_recover;
   commandlist["uninstall"] = c_uninstall;
   commandlist["obliterate"] = c_obliterate;
   commandlist["enter"] = c_enter;
   commandlist["status"] = c_status;
   commandlist["build"] = c_build;
   commandlist["unittest"] = c_unittest;
   commandlist["servicecmd"] = c_servicecmd;

   it=commandlist.find(s);
   if (it==commandlist.end())
      showhelp(*this,"Unknown command \"" + s + "\".");
   return it->second;
}

void params::setdefaults()
{
   mVersion = getVersionStr();
   mLogLevel = kLINFO;
   
   mServiceOutput_hooks = kOLogged;
   mServiceOutput_servicecmd = kORaw;

   mOption = "-n";
   mCmd = c_UNDEFINED;
}

params::params(eLogLevel ll)
{
   setdefaults();
   mLogLevel=ll;
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
//            {"create", 1, 0, 'c'},
            {0, 0, 0, 0}
         };

      // run getopt_long, hiding errors.
      extern int opterr;
      opterr = 0;
      c = getopt_long (argc, argv, "vsnol",
                     long_options, &option_index);

      if (c == -1) // no more options.
         break;

      switch (c)
      {
         case 's':
            mLogLevel=kLERROR;
            mServiceOutput_hooks = kOSuppressed;
            mServiceOutput_servicecmd = kOSuppressed;
            mOption = "-s";
            break;

         case 'v':
            mLogLevel=kLDEBUG;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kORaw;
            mOption = "-v";
            break;

         case 'o':
            mLogLevel=kLERROR;
            mServiceOutput_hooks = kORaw;
            mServiceOutput_servicecmd = kORaw;
            mOption = "-o";
            break;

         case 'n':
            mLogLevel=kLINFO;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kORaw;
            mOption = "-n";
            break;

         case 'l':
            mLogLevel = kLINFO;
            mServiceOutput_hooks = kOLogged;
            mServiceOutput_servicecmd = kOLogged;
            mOption = "-n";
            break;

         default:
            showhelp(*this,"Unrecognised option."); //" -" + std::string(1,c));
      }
   }

   // drunner with no command.
   int opx=optind;
   if (utils::isInstalled())
   {
      if (opx>=argc)
         showhelp( *this, "Please enter a command.");
      mCmd=parsecmd(argv[opx++]);
   } else {
      // NOT installed.
      if (opx>=argc)
         showhelp( *this, R"EOF(Not yet installed.
Please provide command line argument:
 1) the ROOTPATH to install to, or
 2) unittest to run unittests
)EOF");
      if (utils::stringisame(argv[opx],"unittest"))
      {
         ++opx;
         mCmd=c_unittest;
      } else
         mCmd=c_setup;
   }

   // store the arguments to the command.
   for (int i=opx;i<argc;++i)
      mArgs.push_back(argv[i]);
}

const std::string & params::getArg(int n) const 
{ 
   if (n >= (int)mArgs.size())
      fatal("Coding error - attempting to get arg that doesn't exist.");
   return mArgs[n]; 
}
