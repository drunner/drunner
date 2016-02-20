#include <string>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>
#include <iostream>
#include <map>

#include "params.h"
#include "utils.h"
#include "showhelp.h"

std::string params::substitute( const std::string & source ) const
{
   std::string d;
   d=utils::replacestring(source,"${VERSION}",mVersion);
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
  mVersion="0.1 Dev";
  mOMode=om_normal;
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
            mOMode=om_silent;
            break;

         case 'v':
            mOMode=om_verbose;
            break;

         case 'g':
            mOMode=om_getouput;
            break;

         default:
            showhelp(*this,"Unrecognised option "+c);
      }
   }
   if (optind>=argc) showhelp(*this);

   mCmd=parsecmd(argv[optind]);

   for (int i=optind+1;i<argc;++i)
      mArgs.push_back(argv[i]);
}
