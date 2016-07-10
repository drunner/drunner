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
#include "command_dev.h"
#include "drunnerSettings.h"
#include "showhelp.h"
#include "main.h"
#include "unittests.h"
#include "service.h"
#include "plugins.h"
#include "checkprerequisits.h"

//  sudo apt-get install build-essential g++-multilib libboost-all-dev

using namespace utils;

// ----------------------------------------------------------------------------------------------------------------------

void waitforreturn()
{
#if defined(_WIN32) && defined(_DEBUG)
   std::cerr << std::endl << std::endl << "--- PRESS RETURN TO CONTINUE ---" << std::endl;
   std::string s;
   std::getline(std::cin, s);
#endif
}

int main(int argc, char **argv)
{
   try
   {
      check_prerequisits();

      GlobalContext::init(argc, argv);

      logmsg(kLDEBUG,"dRunner C++, version "+GlobalContext::getParams()->getVersion());

      int rval = mainroutines::process();
      waitforreturn();
      return rval;
   }

   catch (const eExit & e) {
      waitforreturn();
      return e.exitCode();
   }
}

// ----------------------------------------------------------------------------------------------------------------------

int mainroutines::process()
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
      return 0;
   }

   if (!GlobalContext::hasSettings())
      fatal("Settings global object not created.");

   logmsg(kLDEBUG,"Settings read from "+GlobalContext::getSettings()->getPath_drunnerSettings_sh().toString());


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
         if (p.numArgs()<1)
            drunner_setup::update(); // defined in command_setup
         else
         { // first argument is service name.
            service s(p.getArg(0));
            s.update();
         }
         break;
      }

      case c_checkimage:
      {
         if (p.numArgs()<1)
            logmsg(kLERROR,"Usage: drunner checkimage IMAGENAME");
         
         validateImage(p.getArg(0));
         break;
      }

      case c_install:
      {
         if (p.numArgs()<1 || p.numArgs()>2)
            logmsg(kLERROR,"Usage: drunner install IMAGENAME [SERVICENAME]");
         std::string imagename = p.getArg(0);
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

         service svc(servicename, imagename);
         svc.install();
         break;
      }

      case c_restore:
      {
         if (p.numArgs() < 2)
            logmsg(kLERROR, "Usage: [PASS=?] drunner restore BACKUPFILE SERVICENAME");

         return service_restore(p.getArg(1), p.getArg(0));
      }

      case c_backup:
      {
         if (p.numArgs() < 2)
            logmsg(kLERROR, "Usage: [PASS = ? ] drunner backup SERVICENAME BACKUPFILE");
         service svc(p.getArg(0));
         svc.backup(p.getArg(1));
         break;
      }

      case c_enter:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner enter SERVICENAME");
         service svc(p.getArg(0));
         svc.enter();
         break;
      }

      case c_build:
      {
         if (p.numArgs()<1)
            command_dev::build();
         else
            command_dev::build(p.getArg(0));
         break;
      }

      case c_servicecmd:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "servicecmd should not be invoked manually.");

         service svc(p.getArg(0));
         if (!svc.isValid())
            logmsg(kLERROR, "Service " + svc.getName() + " is not valid - try recover.");

         return svc.servicecmd();
      }

      case c_status:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner status SERVICENAME");
         service svc(p.getArg(0));
         return svc.status();
      }

      case c_recover:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner recover SERVICENAME [IMAGENAME]");
         std::string servicename = p.getArg(0);
         std::string imagename;
         if (p.numArgs() < 2)
         { // see if we can read the imagename from the damaged service.
            service svc1(servicename);
            if (!utils::fileexists(svc1.getPath()))
               logmsg(kLERROR, "That service is not installed. Try installing, which will preserve any data volumes.");

            imagename = svc1.getImageName();
            poco_assert(imagename.length() > 0);
         }
         else
            imagename = p.getArg(1);

         logmsg(kLINFO, "Recovering " + servicename + " from image " + imagename);
         service svc(servicename, imagename);
         return (int)svc.recover();
      }

      case c_uninstall:
      {
         if (p.numArgs()<1)
            logmsg(kLERROR, "Usage: drunner uninstall SERVICENAME");
         service svc(p.getArg(0));
         return (int)svc.uninstall();
      }

      case c_obliterate:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "Usage: drunner obliterate SERVICENAME");

         service_obliterate svco(p.getArg(0));
         return (int)svco.obliterate();
      }

      case c_help:
      {
         showhelp();
         return kRSuccess;
      }

      case c_saveenvironment:
      {
         if (p.numArgs() < 3)
            logmsg(kLERROR, "Usage: drunner __save-environment SERVICENAME KEY VALUE");
         service svc(p.getArg(0));
         if (!svc.isValid())
            logmsg(kLERROR, "Service " + svc.getName() + " is not valid - try recover.");

         svc.getEnvironment().save_environment(p.getArg(1), p.getArg(2));
         logmsg(kLDEBUG, "Save environment variable " + p.getArg(1) + "=" + p.getArg(2));
         return kRSuccess;
      }

      case c_plugin:
      {
         plugins p;
         return p.runcommand();
      }


      default:
         {
            logmsg(kLERROR,R"EOF(

          /-------------------------------------------------------------\
          |   That command has not been implemented and I am sad. :,(   |
          \-------------------------------------------------------------/
)EOF");
            return 1;
      }
   }
   return 0;
}


// ----------------------------------------------------------------------------------------------------------------------
