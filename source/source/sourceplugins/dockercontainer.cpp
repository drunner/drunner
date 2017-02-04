
#include "dockercontainer.h"
#include "utils_docker.h"
#include "dassert.h"

namespace sourcecopy
{
   static const std::string dockerstr = "docker:";

   // Copy from within a docker container.
   cResult dockercontainer::install(std::string imagename, const servicePaths & sp)
   {
      cResult r = utils_docker::pullImage(imagename);
      if (!r.success())
         return r;

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

   bool dockercontainer::pluginmatch(std::string imagename)
   {
      return (imagename.find(dockerstr) == 0);
   }

   cResult dockercontainer::normaliseNames(std::string & imagename, std::string & servicename)
   {
      // format is docker:[url or docker hub repo/]image[:tag]
      drunner_assert(imagename.find(dockerstr) == 0,"Malformed imagename in dockercontainer::checkname - "+imagename);
      std::string s = imagename;
      s.erase(0, dockerstr.length());

      if (s.find('/') == std::string::npos)
         s = "drunner/" + s;

      size_t pos = s.find_last_of('/');

      std::string url = s.substr(0, pos + 1); // keep the /
      std::string repo = s.substr(pos + 1);

      if (servicename.length() == 0)
      {
         servicename = repo;
         size_t pos2 = servicename.find(':', pos);
         if (pos2 != std::string::npos)
            servicename.erase(pos2);
      }

      imagename = dockerstr + url + repo;

      return cResult();
   }

}