#include <iostream>

#include "showhelp.h"
#include "utils.h"
#include "globallogger.h"
#include "globalcontext.h"

void showhelp(std::string cMsg) {

   const params & p(*GlobalContext::getParams());

   logmsg(kLINFO,p.substitute(R"EOF(
NAME
   ${EXENAME}

VERSION
   ${VERSION}

DESCRIPTION
   Provides a standard way to manage and run docker services.
   See http://drunner.io
)EOF"));


      logmsg(kLINFO, p.substitute(R"EOF(
SYNOPSIS
   ${EXENAME} [OPTION] [COMMAND] [ARGS]...

OPTIONS
   -n    output: normal (default)
   -v    output: verbose
   -l    output: logged (dService output is logged)
   -s    output: silent
   -o    output: capture dService output (drunner silent, raw dService output)
   -d    development mode (don't explicitly pull images).

COMMANDS
   ${EXENAME} clean
   ${EXENAME} list
   ${EXENAME} update
   ${EXENAME} checkimage IMAGENAME

   [PASS=?] ${EXENAME} backup  SERVICENAME BACKUPFILE
   [PASS=?] ${EXENAME} restore BACKUPFILE  SERVICENAME

   ${EXENAME} install    IMAGENAME [SERVICENAME]
   ${EXENAME} uninstall  SERVICENAME
   ${EXENAME} obliterate SERVICENAME

   ${EXENAME} update     SERVICENAME
   ${EXENAME} recreate   SERVICENAME [IMAGENAME]

   ${EXENAME} status     SERVICENAME

EXIT CODE
   0   - success
   1   - error
   3   - no changes made
)EOF"));

   if (cMsg.length() > 0)
      logmsg(kLERROR, cMsg);
}
