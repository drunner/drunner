#ifndef __CREATESUPPORTFILES_H
#define __CREATESUPPORTFILES_H

#include "enums.h"
#include "params.h"
#include "utils.h"
#include "generate.h"

void generate_validator_image(const std::string & supportpath, const params & p)
{
   std::string vdata=R"EOF(#!/bin/bash
set -o nounset
set -e
# Validate the image I'm running in as being Docker Runner compatible

function die { echo "Not dRunner compatible - $1"; exit 1 ; }

if [ "$UID" -eq 0 ]; then die "the container runs as root." ; fi

# Check mandatory files in image (global var IMAGENAME) before touching host. Is it a valid dService?
readonly REQDFILES=("servicecfg.sh" "servicerunner")
for CFILE in "${REQDFILES[@]}"; do
   if [ ! -e "/drunner/$CFILE" ]; then
      die "required file not present: /drunner/$CFILE"
   fi
done

source /drunner/servicecfg.sh || die "Couldnt load servicecfg.sh"
exit 0
}
)EOF";

   generate(supportpath+"/validator-image",p,S_755,vdata);
}


#endif
