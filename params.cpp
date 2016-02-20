#include <string>
#include "params.h"
#include "utils.h"
#include "showhelp.h"

#include <stdio.h>
#include <getopt.h>
#include <iostream>

std::string params::substitute( const std::string & source ) const
{
   std::string d;
   d=utils::replacestring(source,"${VERSION}",mVersion);
   return d;
}


params::params(int argc, char **argv) {
  mVersion="0.1 Dev";
  mOMode=om_normal;

  // parse command line stuff.
  int c;
  while (1)
  {
     int option_index = 0;
     static struct option long_options[] =
          { // name, has_arg, flag=0, val={0, character}
            {"verbose", 0, 0, 'v'},
            {"silent", 0, 0, 's'},
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

         default:
            showhelp(*this,"Unrecognised option "+c);
      }

      if (optind>=argc) showhelp(*this);

      std::string tcmd=argv[optind];
      for (int i=optind+1;i<argc;++i)
         mOptions.push_back(argv[i]);
  }
}
