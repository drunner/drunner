#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

#include "showhelp.h"
#include "utils.h"
#include "logmsg.h"

void showhelp(const params & p, std::string cMsg) {
   logmsg(kLINFO,p.substitute(R"EOF(
NAME
   ${EXENAME}

VERSION
   ${VERSION}

DESCRIPTION
   Provides a standard way to manage and run docker services.
   See http://drunner.io
)EOF"), p);

   if (utils::isInstalled())
      logmsg(kLINFO,p.substitute(R"EOF(
SYNOPSIS
   ${EXENAME} [OPTION] [COMMAND] [ARGS]...

OPTIONS
   -n    normal (default)
   -v    verbose
   -l    logged (dService output is logged)
   -s    silent
   -o    capture dService output (drunner silent, raw dService output)

COMMANDS
   ${EXENAME} clean
   ${EXENAME} list
   ${EXENAME} update
   ${EXENAME} checkimage IMAGENAME

   [PASS=?] ${EXENAME} backup  SERVICENAME BACKUPFILE
   [PASS=?] ${EXENAME} restore BACKUPFILE  SERVICENAME

   ${EXENAME} install    IMAGENAME [SERVICENAME]
   ${EXENAME} update     SERVICENAME
   ${EXENAME} recover    SERVICENAME [IMAGENAME]
   ${EXENAME} uninstall  SERVICENAME
   ${EXENAME} obliterate SERVICENAME
   ${EXENAME} enter      SERVICENAME [ARGS]
   ${EXENAME} status     SERVICENAME

   SERVICENAME
   SERVICENAME COMMAND ARGS

EXIT CODE
   0   - success
   1   - error
   3   - no changes made
)EOF"), p);
   else
      logmsg(kLINFO,p.substitute(R"EOF(
SYNOPSIS
   Install drunner with services to be stored under ROOTPATH:
   ${EXENAME} [OPTION] ROOTPATH

OPTIONS
   -v    verbose
   -s    silent
)EOF"), p);

   //setenv("Sniggle", "wiggle", 1);
   //execl("/usr/bin/env", "env", 0);
   //std::cerr << cMsg << std::endl;
   logmsg(kLERROR, cMsg, p);
}
