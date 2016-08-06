#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sstream>

#include "globalcontext.h"
#include "globallogger.h"
#include "exceptions.h"
#include "utils.h"
#include "drunner_setup.h"
#include "command_general.h"
#include "drunner_settings.h"
#include "showhelp.h"
#include "main.h"
#include "unittests.h"
#include "service.h"
#include "plugins.h"
#include "validateimage.h"
#include "service_install.h"
#include "drunner_settings.h"

//  sudo apt-get install build-essential g++-multilib libboost-all-dev

using namespace utils;

// ----------------------------------------------------------------------------------------------------------------------

void waitforreturn()
{
   if (GlobalContext::hasParams())
      if (GlobalContext::getParams()->doPause())
      {
         std::cerr << std::endl << std::endl << "--- PRESS RETURN TO CONTINUE ---" << std::endl;
         std::string s;
         std::getline(std::cin, s);
      }
}

void _handleexception()
{

}

int main(int argc, char **argv)
{
   try
   {
      GlobalContext::init(argc, argv);

      logmsg(kLDEBUG,"dRunner C++ "+GlobalContext::getParams()->getVersion());

      cResult rval = mainroutines::process();
      if (rval.error())
      {
         logdbg("Error context: "+rval.context());
         fatal(rval.what());
      }
      waitforreturn();
      return rval;
   }

   catch (const eExit & e) {
      waitforreturn();
      return e.exitCode();
   }
}

// 

std::string _imageparse(std::string imagename)
{
   if (imagename.find('/') == std::string::npos)
      return "drunner/" + imagename;
   return imagename;
}

// ----------------------------------------------------------------------------------------------------------------------

cResult mainroutines::process()
{
   const params & p(*GlobalContext::getParams());

   // allow unit tests to be run directly from installer.
   if (p.getCommand()==c_unittest)
   {
      // todo: pass args through to catch.
      // int result = Catch::Session().run( argc, argv );
      int result = UnitTest();
      if (result!=0)
         logmsg(kLERROR,"Unit tests failed.");
      logmsg(kLINFO,"All unit tests passed.");
      return kRSuccess;
   }

   if (!GlobalContext::hasSettings())
      fatal("Settings global object not created.");

   if (p.getCommand()==c_configure)
   {
      CommandLine cl;
      cl.command = "configure";
      cl.args = p.getArgs();
      drunnerSettings newSettings;
      return newSettings.handleConfigureCommand(cl); // needs to be read/write settings.
   }

   cResult rval = GlobalContext::getSettings()->checkRequired();
   if (!rval.success())
      fatal("Please configure dRunner:\n " + rval.what());

   // ----------------
   // command handling
   switch (p.getCommand())
   {
      case c_clean:
      {
         command_general::clean();
         break;
      }

      case c_list:
      {
         command_general::showservices();
         break;
      }

      case c_update:
      {
         if (p.numArgs() < 1)
            return drunnerSetup::update_drunner();
         else
         {
            service_install svi(p.getArg(0));
            return svi.update();
         }
      }

      case c_setup:
      {
         if (p.numArgs() < 1)
            return drunnerSetup::check_setup(true);
         else
            fatal("Setup not available for dServices.");
         break;
      }

      case c_checkimage:
      {
         if (p.numArgs()<1)
            logmsg(kLERROR,"Usage: drunner checkimage IMAGENAME");
         
         validateImage::validate(_imageparse(p.getArg(0)));
         break;
      }

      case c_install:
      {
         if (p.numArgs()<1 || p.numArgs()>2)
            logmsg(kLERROR,"Usage: drunner install IMAGENAME [SERVICENAME]");
         std::string imagename = _imageparse(p.getArg(0));
         std::string servicename;
         if ( p.numArgs()==2)
            servicename=p.getArg(1); // if empty then install will set to default from imagename.
         else
         {
            servicename = imagename;
            size_t found;
            while ((found = servicename.find("/")) != std::string::npos)
               servicename.erase(0, found + 1);
            while ((found = servicename.find(":")) != std::string::npos)
               servicename.erase(found);
         }

         service_install svc(servicename, imagename);
         return svc.install();
      }

      case c_restore:
      {
         if (p.numArgs() < 2)
            logmsg(kLERROR, "Usage: [PASS=?] drunner restore BACKUPFILE SERVICENAME");

         service_install svc(p.getArg(1));
         return svc.service_restore(p.getArg(0));
      }

      case c_backup:
      {
         if (p.numArgs() < 2)
            logmsg(kLERROR, "Usage: [PASS = ? ] drunner backup SERVICENAME BACKUPFILE");
         service svc(p.getArg(0));
         return svc.backup(p.getArg(1));
      }

      case c_servicecmd:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "servicecmd should not be invoked manually.");

         service svc(p.getArg(0));
         return svc.servicecmd();
      }

      case c_status:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner status SERVICENAME");
         service svc(p.getArg(0));
         return svc.status();
      }

      case c_recreate:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner recover SERVICENAME [IMAGENAME]");
         std::string servicename = p.getArg(0);
         if (p.numArgs() == 2)
         {
            service_install svc(servicename, _imageparse(p.getArg(1)));
            return svc.recover();
         }
         else {
            service_install svc(servicename);
            return svc.recover();
         }
      }

      case c_uninstall:
      {
         if (p.numArgs()<1)
            logmsg(kLERROR, "Usage: drunner uninstall SERVICENAME");
         service_install svc(p.getArg(0));
         return svc.uninstall();
      }

      case c_obliterate:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner obliterate SERVICENAME");

         service_install svc(p.getArg(0));
         return svc.obliterate();
      }

      case c_help:
      {
         drunnerSetup::check_setup(false);
         showhelp();
         return kRSuccess;
      }

      case c_plugin:
      {
         plugins p;
         return p.runcommand();
      }


      default:
      {
         return cError("The command '"+p.getCommandStr()+"' is not recognised.");
      }
   }
   return 0;
}


// ----------------------------------------------------------------------------------------------------------------------
