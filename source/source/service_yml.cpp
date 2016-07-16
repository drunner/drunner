#include "yaml-cpp/yaml.h"

#include "service_yml.h"
#include "utils.h"
#include "globallogger.h"

namespace serviceyml
{
 
   simplefile::simplefile(Poco::Path path) : mPath(path)
   {
      poco_assert(mPath.isFile());
   }
   
   cResult simplefile::_loadyml()
   {
      if (!utils::fileexists(mPath))
         return kRError;

      YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      if (!yamlfile)
      {
         logmsg(kLDEBUG, "Unable to read yaml file from " + mPath.toString());
         return kRError;
      }

      // load containers
      YAML::Node containers = yamlfile["containers"];
      for (auto it = containers.begin(); it != containers.end(); ++it)
         mContainers.push_back(it->as<std::string>());

      // load configuration
      YAML::Node configuration = yamlfile["configuration"];
      for (auto it = configuration.begin(); it != configuration.end(); ++it)
      {
         Configuration ci;
         ci.name = it->first.as<std::string>();
         ci.required = false;

         poco_assert(it->second.IsMap());
         YAML::Node value = it->second;
         if (value["description"].IsDefined())
            ci.description = value["description"].as<std::string>();
         if (value["default"].IsDefined())
            ci.defaultval = value["default"].as<std::string>();
         if (value["required"].IsDefined())
            ci.required = value["required"].as<bool>();

         mConfigItems.push_back(ci);
      }

      return kRSuccess;
   }


   cResult simplefile::loadyml()
   {
      cResult rval(kRSuccess);
      try {
         rval=_loadyml();
      }
      catch (const YAML::Exception & e) {
         logmsg(kLERROR, std::string("YAML error: ") + e.what());
      }
      return rval;
   }

   file::file(Poco::Path path) : simplefile(path)
   {
   }

   cResult file::loadyml(const variables & v)
   {
      if (kRSuccess != simplefile::loadyml())
         return kRError;
      poco_assert(mPath.isFile());
      poco_assert(utils::fileexists(mPath)); // ctor of simplefile should have set mReadOkay to false if this wasn't true.

      YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      poco_assert(yamlfile);

      if (yamlfile["commands"])
      {// load commands.
         YAML::Node commands = yamlfile["commands"];
         for (auto it = commands.begin(); it != commands.end(); ++it)
         {
            CommandLine cl;
            cl.name = it->first.as<std::string>();
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
               Operation op;
               std::string opline = it2->as<std::string>();
               std::string sopline = v.substitute(opline);
               utils::split_in_args(sopline, op.args);
               poco_assert(op.args.size() > 0);
               op.command = op.args[0];
               op.args.erase(op.args.begin());
               cl.operations.push_back(op);
            }
            mCommands.push_back(cl);
         }
      }

      if (yamlfile["volumes"])
      { // load volumes
         YAML::Node volumes = yamlfile["volumes"];
         for (auto it = volumes.begin(); it != volumes.end(); ++it)
         {
            Volume v;
            v.name = it->first.as<std::string>();
            v.backup = it->second["backup"].as<bool>();
            v.manage = it->second["manage"].as<bool>();
            mVolumes.push_back(v);
         }
      }

      return kRSuccess;
   }

   void file::getManageDockerVolumeNames(std::vector<std::string> & vols) const
   {
      poco_assert(vols.size() == 0);
      for (const auto & v : mVolumes)
         if (v.manage)
            vols.push_back(v.name);
   }
   void file::getBackupDockerVolumeNames(std::vector<std::string> & vols) const
   {
      poco_assert(vols.size() == 0);
      for (const auto & v : mVolumes)
         if (v.backup)
            vols.push_back(v.name);
   }

   const std::vector<std::string> & simplefile::getContainers() const
   {
      return mContainers;
   }
   const std::vector<Configuration> & simplefile::getConfigItems() const
   {
      return mConfigItems;
   }

} // namespace