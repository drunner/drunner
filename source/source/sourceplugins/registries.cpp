#include <memory>

#include "Poco/File.h"
#include <cereal/archives/json.hpp>
#include <fstream>

#include "utils.h"
#include "utils_docker.h"
#include "dassert.h"
#include "registries.h"
#include "drunner_paths.h"
#include "buildnum.h"
#include "timez.h"
#include "localdir.h"

namespace sourceplugins
{

   static std::string gitprefix = "git:";

   registries::registries()
   {
      mPath = drunnerPaths::getPath_Settings().setFileName("registries");

      if (load().noChange())
      { // create and save defaults.
         mData.setVal("drunner","https://github.com/drunner/");
         cResult r = save();
         if (!r.success())
            fatal("Couldn't save registries file:\n" + r.what());
      }
   }

   cResult registries::install(std::string imagename, const servicePaths & sp)
   {
      Poco::Path dest = sp.getPathdService();
      std::string registry, repo, tag;
      cResult r = splitImageName(imagename, registry, repo, tag);
      if (!r.success())
         fatal(r.what());

      logmsg(kLDEBUG, "registry = " + registry + ", repo = " + repo + ", tag = " + tag);
      return copy(registry, repo, tag, dest);
   }

   bool registries::pluginmatch(std::string imagename)
   {
      // we have to say yes to most things here :/
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

   cResult registries::normaliseNames(std::string & imagename, std::string & servicename)
   {
      // optional prefix.
      std::string gitprefix = "git:";
      if (imagename.find(gitprefix) == 0)
         imagename.insert(0, gitprefix);

      if (servicename.length() > 0)
         return kRSuccess;

      std::string registry, repo, tag;
      cResult r = splitImageName(imagename, registry, repo, tag);
      servicename = repo;
      return r;
   }

   cResult registries::addregistry(std::string nicename, std::string giturl)
   {
      if (mData.getVal(nicename).compare(giturl) == 0)
         return kRNoChange;

      mData.setVal(nicename, giturl);
      return save();
   }

   cResult registries::delregistry(std::string nicename)
   {
      if (!mData.hasKey(nicename))
         return kRNoChange;

      mData.delKey(nicename);
      return save();
   }


   std::string _pad(std::string x, unsigned int w)
   {
      while (x.length() < w) x += " ";
      return x;
   }
   inline int _max(int a, int b) { return (a > b) ? a : b; }

   cResult registries::showregistries()
   {
      int maxkey = 0;
      for (const auto & y : mData.getAll())
         maxkey = _max(maxkey, y.first.length());
      for (const auto & y : mData.getAll())
         logmsg(kLINFO, " " + _pad(y.first, maxkey) + " -> " + y.second);

      return kRSuccess;
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
      op.args = { "--progress", "-b", tag, "--depth", "1", url };
      utils::tempfolder scratch(drunnerPaths::getPath_Temp().pushDirectory("drunnerdef-" + timeutils::getDateTimeStr()));
      tKeyVals env;
      std::string outstr;
      int r = utils::runcommand_stream(op, kORaw, scratch.getpath(), env, &outstr);
      if (r != 0)
         return kRNoChange; // fail to clone.

      return localdir::copydServiceFiles(scratch.getpath().pushDirectory(repo),dest);
   }

   // splitImageName
   // splits the imagename into the registry nice name, the repo and the tag.
   cResult registries::splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag) const
   {
      // imagename is of form:  git:[registry/]repo[:tag]
      if (imagename.find(gitprefix) != 0)
         return cError("Missing git: prefix for git registry.");
      
      repo = imagename;
      repo.erase(0, gitprefix.length());

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

   cResult registries::load()
   {
      if (!utils::fileexists(mPath))
         return kRNoChange;

      // read the settings.
      std::ifstream ifs(mPath.toString());
      if (ifs.bad())
         return cError("Unable to open " + mPath.toString() + " for reading.");

      try
      {
         cereal::JSONInputArchive archive(ifs);
         archive(mData);
      }
      catch (const cereal::Exception & e)
      {
         return cError("Cereal exception on reading registries: " + std::string(e.what()));
      }

      return kRSuccess;
   }

   cResult registries::save()
   {
      std::ofstream ofs(mPath.toString());
      if (ofs.bad())
         return cError("Unable to open " + mPath.toString() + " for writing.");

      try
      {
         cereal::JSONOutputArchive archive(ofs);
         archive(mData);
      }
      catch (const cereal::Exception & e)
      {
         return cError("Cereal exception on writing registries: " + std::string(e.what()));
      }

      return kRSuccess;
   }





}

