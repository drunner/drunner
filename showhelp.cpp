#include "showhelp.h"
#include <iostream>


   void showhelp(const params & p, std::string cMsg) {
      if (p.mOMode != om_silent) {
         std::cerr << p.substitute(R"EOF(

NAME
   drunner - docker Runner

VERSION
   ${VERSION}

LOCATION
   $ROOTPATH

SYNOPSIS
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

DESCRIPTION
   Provides a standard way to manage and run containers supporting dr.
   Intended to be used both manually and via Ansible.
   See https://github.com/j842/drunner

EXIT CODE
   0   - success
   1   - error
   3   - no changes made

)EOF");
      if (cMsg.length()>0)
         std::cerr << cMsg << std::endl;
      }
      exit(1);
   }
