#include <memory>

#include "Poco/String.h"
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
#include "registry.h"

registries::registries()
{
   mPath = drunnerPaths::getPath_Settings().setFileName("registries");

   if (load().noChange())
   { // create and save defaults.
      mData.setVal(
         registrydefinition("drunner", "http", "https://raw.githubusercontent.com/drunner/registry/master/registry")
      );
      cResult r = save();
      if (!r.success())
         fatal("Couldn't save registries file:\n" + r.what());
   }
}

cResult registries::addregistry(std::string nicename, std::string protocol, std::string url)
{
   mData.setVal(registrydefinition(nicename, protocol, url));
   return save();
}

cResult registries::delregistry(std::string nicename)
{
   if (!mData.exists(nicename))
      return kRNoChange;

   mData.delVal(nicename);
   return save();
}

cResult registries::showregistries()
{
   int maxkey = 0, maxproto=0, maxurl=0;
   for (const auto & y : mData.getAll())
   {
      maxkey = utils::_max(maxkey, y.mNiceName.length());
      maxproto = utils::_max(maxproto, y.protostr().length());
      maxurl = utils::_max(maxurl, y.mURL.length());
   }
   for (const auto & y : mData.getAll())
      logmsg(kLINFO, 
         " " + utils::_pad(y.mNiceName, maxkey) + 
         " -> " + utils::_pad(y.protostr(),maxproto) +
         " : " + utils::_pad(y.mURL,maxurl));

   return kRSuccess;
}

registrydefinition registries::get(const std::string imagename) const
{
   // load registries and see if we can find a match for nicename.
   // [registry/]nicename[:tag]
   std::string registry, dservicename, tag;
   cResult r = splitImageName(imagename, registry, dservicename, tag);
   if (!r.success())
   {
      logmsg(kLWARN, r.what());
      return registrydefinition();
   }

   registrydefinition regdata;
   if (!mData.getVal(registry,regdata).success())
      fatal("Unable to acccess registry " + registry);

   sourcecopy::registry reg(regdata.mURL);
   sourcecopy::registryitem item;
   r = reg.get(dservicename, item);
   if (!r.success())
   {
      logmsg(kLWARN, r.what());
      return registrydefinition();
   }

   return regdata;
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
cResult registries::splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag)
{
   // imagename is of form:  [registry/]repo[:tag]

   repo = imagename;

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
      if (pos < repo.length() - 1)
         tag = repo.substr(pos + 1);
      repo.erase(pos);
   }
   return kRSuccess;
}

void registrydefinitions::setVal(const registrydefinition & val)
{
   for (unsigned int i=0;i<mItems.size();++i)
      if (Poco::icompare(mItems[i].mNiceName, val.mNiceName) == 0)
      {
         mItems.erase(mItems.begin() + i);
      }
}

bool registrydefinitions::exists(std::string nicename) const
{
   for (auto & x : mItems)
      if (Poco::icompare(x.mNiceName, nicename) == 0)
         return true;
   return false;
}

cResult registrydefinitions::getVal(std::string nicename, registrydefinition & val) const
{
   for (auto & x : mItems)
      if (Poco::icompare(x.mNiceName, nicename) == 0)
      {
         val = x;
         return kRSuccess;
      }
   val= registrydefinition();
   return cError("Couldn't find " + nicename + " in registry.");
}

void registrydefinitions::delVal(std::string nicename)
{
}

const std::vector<registrydefinition>& registrydefinitions::getAll() const
{
   return mItems;
}
