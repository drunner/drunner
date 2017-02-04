#include "git.h"
#include "dassert.h"

namespace sourcecopy
{


   cResult git::install(std::string imagename, const servicePaths & sp)
   {
      Poco::Path dest = sp.getPathdService();
      std::string registry, repo, tag;
      cResult r = splitImageName(imagename, registry, repo, tag);
      if (!r.success())
         fatal(r.what());

      logmsg(kLDEBUG, "registry = " + registry + ", repo = " + repo + ", tag = " + tag);
      return copy(registry, repo, tag, dest);
   }

   bool git::pluginmatch(std::string imagename)
   {
      // we have to say yes to most things here as this is the default.
      // just check imagename is of form:  [git:][registry/]repo[:tag]

      // optional prefix. Ensured present in normalize, which hasn't been called.
      std::string gitprefix = "git:";
      if (imagename.find(gitprefix) == 0)
         imagename.erase(0, gitprefix.length());

      if (std::count(imagename.begin(), imagename.end(), ':') > 1)
         return false;
      if (std::count(imagename.begin(), imagename.end(), '/') > 1)
         return false;
      if (std::count(imagename.begin(), imagename.end(), ' ') > 0)
         return false;
      size_t ps = imagename.find('/');
      size_t pc = imagename.find(':');
      if (pc != std::string::npos && ps != std::string::npos)
         if (pc < ps)
            return false;  // blah:blah/blah
      return true;
   }

   cResult git::normaliseNames(std::string & imagename, std::string & servicename)
   {
      // optional prefix.
      std::string gitprefix = "git:";
      if (imagename.find(gitprefix) == std::string::npos)
         imagename.insert(0, gitprefix);

      if (servicename.length() > 0)
         return kRSuccess;

      std::string registry, repo, tag;
      cResult r = splitImageName(imagename, registry, repo, tag);
      servicename = repo;

      drunner_assert(imagename.find(gitprefix) == 0, "Logic error in registries::normaliseNames.");

      return r;
   }


   cResult registries::copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const
   { // copy the dService definition from the specified registry.
      if (!mData.hasKey(nicename))
         return kRNoChange; // couldn't find registry.

                            // try dxx_repo first.
      std::string url = "d" + getVersionNice() + "_" + mData.getVal(nicename) + repo + ".git";
      if (copy(url, repo, tag, dest).success())
         return kRSuccess;

      // and just the base repo second.
      url = mData.getVal(nicename) + repo + ".git";
      if (copy_url(url, repo, tag, dest).success())
         return kRSuccess;

      return kRNoChange;
   }

   cResult registries::copy_url(std::string url, std::string repo, std::string tag, Poco::Path dest) const
   {
      CommandLine op;
      op.command = "git";
      op.args = { "clone", "--progress", "-b", tag, "--depth", "1", url };
      utils::tempfolder scratch(drunnerPaths::getPath_Temp().pushDirectory("drunnerdef-" + timeutils::getDateTimeStr()));
      tKeyVals env;
      std::string outstr;
      int r = utils::runcommand_stream(op, kORaw, scratch.getpath(), env, &outstr);
      if (r != 0)
         return kRNoChange; // fail to clone.

      return localdir::copydServiceFiles(scratch.getpath().pushDirectory(repo), dest);
   }

}