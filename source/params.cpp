#include <string>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>
#include <iostream>
#include <map>

#include "params.h"
#include "utils.h"
#include "showhelp.h"
#include "build_number.h"


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

   commandlist["setup"]=c_setup;
   commandlist["clean"]=c_clean;
   commandlist["list"]=c_list;
   commandlist["update"]=c_update;
   commandlist["checkimage"]=c_checkimage;
   commandlist["backup"]=c_backup;
   commandlist["restore"]=c_restore;
   commandlist["install"]=c_install;
   commandlist["recover"]=c_recover;
   commandlist["uninstall"]=c_uninstall;
   commandlist["obliterate"]=c_obliterate;
   commandlist["enter"]=c_enter;
   commandlist["status"]=c_status;

   it=commandlist.find(s);
   if (it==commandlist.end())
      showhelp(*this,"Unknown command \"" + s + "\".");
   return it->second;
}

// Parse command line parameters.
params::params(int argc, char **argv)
{
mVersion=VERSION_STR;
mLogLevel=kLINFO;
mDisplayServiceOutput=true;
mCmd=c_UNDEFINED;

// parse command line stuff.
int c;
while (1)
{
   int option_index = 0;
   static struct option long_options[] =
         { // name, has_arg, flag=0, val={0, character}
            {"verbose", 0, 0, 'v'},
            {"silent", 0, 0, 's'},
            {"getoutput", 0, 0, 'g'},
//            {"create", 1, 0, 'c'},
            {0, 0, 0, 0}
         };

      c = getopt_long (argc, argv, "vs",
                     long_options, &option_index);
      if (c == -1)
         break;

      switch (c)
      {
         case 0:
            // temp.
            printf ("option %s", long_options[option_index].name);
            if (optarg)
            printf (" with arg %s", optarg);
            printf ("\n");
         break;

         case 's':
            mLogLevel=kLERROR;
            mDisplayServiceOutput=false;
            break;

         case 'v':
            mLogLevel=kLDEBUG;
            mDisplayServiceOutput=true;
            break;

         case 'g':
            mLogLevel=kLERROR;
            mDisplayServiceOutput=true;
            break;

         default:
            showhelp(*this,"Unrecognised option "+c);
      }
   }
   
   // drunner with no command.
   if (optind>=argc) showhelp(*this,"Please enter a command.");

   // confirm the command is valid and convert to enum.
   mCmd=parsecmd(argv[optind]);

   // store the arguments to the command.
   for (int i=optind+1;i<argc;++i)
      mArgs.push_back(argv[i]);
}

