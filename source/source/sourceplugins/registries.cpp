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

   sourceinfo registries::get(const std::string imagename) const
   {
      // load registries and see if we can find a match for nicename.
      // [registry/]nicename[:tag]
      std::string registry, dservicename, tag;
      cResult r = splitImageName(imagename, registry, dservicename, tag);
      if (!r.success())
      {
         logmsg(kLWARN, r.what());
         return sourceinfo();
      }

      std::string r = mData.getVal(registry);

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
            return cError("Format must be [registry/]dService[:tag]");

         if (pos > 0)
            registry = repo.substr(0, pos);
         repo.erase(0, pos + 1);
      }

      // extract tag
      tag = "";
      pos = repo.find(':');
      if (pos != std::string::npos)
      {
         if (pos == 0)
            return cError("Missing dService name in image " + imagename);
         tag = repo.substr(pos + 1);
         repo.erase(repo.begin() + pos, repo.end());
      }
      return kRSuccess;
   }
}

