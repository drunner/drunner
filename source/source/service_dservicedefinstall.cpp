#include "utils.h"
#include "dassert.h"
#include "service_dservicedefinstall.h"


namespace sddi
{ 
   cResult copy_from_container(std::string imagename, const servicePaths & sp)
   {
      logdbg("Copying across drunner files from " + imagename);
      CommandLine cl("docker", { "run","--rm","-u","root","-v",
         sp.getPathdService().toString() + ":/tempcopy", imagename, "/bin/bash", "-c" });
#ifdef _WIN32
      cl.args.push_back("cp /drunner/* /tempcopy/ && chmod a+rw /tempcopy/*");
#else
      uid_t uid = getuid();
      cl.args.push_back("cp /drunner/* /tempcopy/ && chmod u+rw /tempcopy/* && chown -R " + std::to_string(uid) + " /tempcopy/*");
#endif
      std::string op;
      if (0 != utils::runcommand(cl, op))
         return cError("Could not copy the service files. You will need to reinstall the service.\nError:\n" + op);
      drunner_assert(utils::fileexists(sp.getPathServiceLua()), "The dService service.lua file was not copied across.");
      return kRSuccess;
   }
}

