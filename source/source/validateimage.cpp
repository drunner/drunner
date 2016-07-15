#include <iterator>

#include "validateimage.h"
#include "utils.h"
#include "drunner_paths.h"
#include "globallogger.h"
#include "basen.h"

namespace validateImage
{
   void validate(std::string imagename)
   {
      if (utils::imageisbranch(imagename))
         logmsg(kLDEBUG, imagename + " looks like a development branch (won't be pulled).");
      else
         logmsg(kLDEBUG, imagename + " looks like a production image.");

      std::string data = R"EOF(#!/bin/bash
set -o nounset
set -e
# Validate the image I'm running in as being Docker Runner compatible

function die { echo "Not dRunner compatible - $1"; exit 1 ; }

if [ "$UID" -eq 0 ]; then die "the container runs as root." ; fi

# Check mandatory files in image (global var IMAGENAME) before touching host. Is it a valid dService?
[ -e "/drunner/service.yml" ] || die "does not have service.yml file."

[ ! -e "/drunner/servicecfg.sh" ] || \
   die "Outdated dService with servicecfg.sh (no longer supported)."

[ ! -e "/drunner/servicerunner" ] || \
   echo "Backward compatible dService with deprecated servicerunner."

exit 0
)EOF";
      std::string encoded_data;
      bn::encode_b64(data.begin(), data.end(), std::back_inserter(encoded_data));
      int n = encoded_data.length() % 4;
      if (n == 2) encoded_data += "==";
      if (n == 3) encoded_data += "=";
      poco_assert(n != 1);

      std::string command = "docker";
      std::vector<std::string> args = { "run","--rm",imagename,"/bin/bash","-c",
         "\"echo " + encoded_data + " | base64 -di > /tmp/validate ; /bin/bash /tmp/validate\"" };

      std::string op;
      int rval = utils::runcommand(command, args, op, true);

      if (rval != 0)
      {
         if (utils::findStringIC(op, "Unable to find image"))
            logmsg(kLERROR, "Couldn't find image " + imagename);
         else
            logmsg(kLERROR, op);
      }

      logmsg(kLDEBUG, op);

#ifdef _WIN32
      logmsg(kLINFO, "[Y] " + imagename + " is dRunner compatible.");
#else
      logmsg(kLINFO, "\u2714  " + imagename + " is dRunner compatible.");
#endif      
   }

}