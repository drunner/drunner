#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"

#include "utils.h"
#include "utils_docker.h"
#include "dassert.h"
#include "service_dservicedefinstall.h"


namespace sddi
{ 
   // ---- DEPRECATED -----
   cResult copy_from_container(std::string imagename, const servicePaths & sp)
   {
      utils_docker::pullImage(imagename);

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

   // splitImageName
   // splits the imagename into the registry nice name, the repo and the tag.
   cResult splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag)
   {
      // imagename is of form:  registry/repo:tag
      repo = imagename;

      // extract registry
      registry = "drunner";
      auto pos = repo.find('/');
      if (pos != std::string::npos)
      {
         if (repo.find(':') != std::string::npos &&
            repo.find(':') < pos)
            return cError("Format must be registry/repo:tag");

         if (pos > 0)
            registry = repo.substr(0, pos);
         repo.erase(0, pos + 1);
      }
      
      // extract tag
      tag = "master";
      pos = repo.find(':');
      if (pos != std::string::npos)
      {
         if (pos == 0)
            return cError("Missing repo name in image " + imagename);
         tag = repo.substr(pos + 1);
         repo.erase(repo.begin() + pos, repo.end());
      }
      return kRSuccess;
   }

   cResult copyfiles(Poco::Path pfrom, Poco::Path pto)
   {
      Poco::Path servicelua = pfrom;
      servicelua.setFileName("service.lua");
      if (!utils::fileexists(servicelua))
         return cError("Couldn't find " + servicelua.toString()+"\nIs this a dService?!");
      
      try
      {
         Poco::DirectoryIterator it(pfrom);
         Poco::DirectoryIterator end;
         while (it != end)
         {
            if (it->isFile())
            {
               logmsg(kLDEBUG, "Installing " + it.name());
               Poco::File(it.path()).copyTo(pto.toString());
            }
            ++it;
         }
      }
      catch (const Poco::Exception & e) {
         return cError(std::string("Couldn't copy files - ") + e.what());
      }
      return kRSuccess;
   }

   cResult handleLocal(std::string & imagename, const servicePaths & sp)
   {
      static const std::string local = "local|";

      Poco::Path path;
      if (imagename.compare(".") == 0)
      {
         path = Poco::Path::current();
         imagename = local + path.toString();
      }
      else
      {
         if (imagename.find(local) != 0)
            return kRNoChange;
         path = imagename.substr(local.length());
      }

      return copyfiles(path,sp.getPathdService());
   }


   cResult copy_from_github(std::string & imagename, const servicePaths & sp)
   {
      std::string registry, repo, tag;

      cResult r = handleLocal(imagename, sp);
      if (!r.noChange())
         return r;

      r = splitImageName(imagename, registry, repo, tag);
      if (!r.success())
         fatal(r.what());

      logmsg(kLDEBUG,"registry = " + registry + ", repo = " + repo + ", tag = " + tag);

      registries reg;
      return reg.copy(registry, repo, tag, sp.getPathdService());
   }
}

