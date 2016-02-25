#include <iostream>
#include <fstream>

#include "createsupportfiles.h"
#include "utils.h"
#include "logmsg.h"


// internal
void create_validator_image(const std::string & supportpath, const params & p);


//---------------------------------------------------------------------------------



eResult create_support_files(const std::string & supportpath, const params & p)
{

//   int bashcommand(std::string command, std::string & output);

   if (utils::fileexists(supportpath))
      utils::deltree(supportpath,p);

   utils::makedirectory(supportpath,p);

   create_validator_image(supportpath,p);
   return kRSuccess;
}

void create_validator_image(const std::string & supportpath, const params & p)
{
   std::string vdata=R"EOF(#!/bin/bash
set -o nounset
set -e
# Validate the image I'm running in as being Docker Runner compatible

function die { echo "Not dRunner compatible - $1"; exit 1 ; }

if [ "$UID" -eq 0 ]; then die "the container runs as root." ; fi

# Check mandatory files in image (global var IMAGENAME) before touching host. Is it a valid dr container?
readonly REQDFILES=("servicecfg.sh" "servicerunner")
for CFILE in "${REQDFILES[@]}"; do
   if [ ! -e "/drunner/$CFILE" ]; then
      die "required file not present: /drunner/$CFILE"
   fi
done

source /drunner/servicecfg.sh || die "Couldn't load servicecfg.sh"
exit 0
}
)EOF";

   std::string op, validator_image=supportpath+"/validator-image";
   std::ofstream ofs;
   ofs.open(validator_image);
   ofs << vdata;
   ofs.close();
   if (utils::bashcommand("chmod a+x "+validator_image,op) != 0)
      logmsg(kLERROR, "Unable to change permissions on "+validator_image,p);
   logmsg(kLDEBUG,"Created "+supportpath+"/validator-image",p);
}
