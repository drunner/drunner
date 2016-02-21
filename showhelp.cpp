#include <iostream>

#include "showhelp.h"
#include "utils.h"
#include "logmsg.h"

void showhelp(const params::params & p, std::string cMsg) {
   logverbatim(kLINFO,R"EOF(

NAME
   drunner - docker Runner

VERSION
   ${VERSION}

LOCATION
   $ROOTPATH

SYNOPSIS
   drunner [OPTION]... [COMMAND] [ARGS]...

OPTIONS
   -v    verbose
   -s    silent
   -g    drunner silent, service normal (for capturing service output)

DESCRIPTION
Provides a standard way to manage and run docker services.
Intended to be used both manually and via Ansible.
See http://drunner.io

   drunner setup

   drunner clean
   drunner list
   drunner update
   drunner checkimage IMAGENAME

   [PASS=?] drunner backup  SERVICENAME BACKUPFILE
   [PASS=?] drunner restore BACKUPFILE  SERVICENAME

   drunner install    IMAGENAME [SERVICENAME]
   drunner update     SERVICENAME
   drunner recover    SERVICENAME
   drunner uninstall  SERVICENAME
   drunner obliterate SERVICENAME
   drunner enter      SERVICENAME [ARGS]
   drunner status     SERVICENAME

   SERVICENAME
   SERVICENAME COMMAND ARGS


EXIT CODE
   0   - success
   1   - error
   3   - no changes made

)EOF", p);   
   
   logmsg(kLERROR, cMsg, p);
}
