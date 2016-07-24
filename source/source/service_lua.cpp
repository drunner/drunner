#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"

namespace servicelua
{
 
   simplefile::simplefile(Poco::Path path) : mPath(path)
   {
      drunner_assert(mPath.isFile(),"Coding error: path provided to simplefile is not a file!");
   }
   

   cResult simplefile::loadlua()
   {
      if (!utils::fileexists(mPath))
         return kRError;

      //YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      //if (!yamlfile)
      //{
      //   logmsg(kLDEBUG, "Unable to read yaml file from " + mPath.toString());
      //   return kRError;
      //}

      //// load containers
      //YAML::Node containers = yamlfile["containers"];
      //for (auto it = containers.begin(); it != containers.end(); ++it)
      //{
      //   drunner_assert(it->IsScalar(), "Containers must be a simple sequence of strings (container names).");
      //   mContainers.push_back(it->as<std::string>());
      //}

      //// load configuration
      //YAML::Node configuration = yamlfile["configuration"];
      //for (auto it = configuration.begin(); it != configuration.end(); ++it)
      //{
      //   Configuration ci;
      //   drunner_assert(it->first.IsScalar(), "Configuration items must be a map of maps.");
      //   ci.name = it->first.as<std::string>();
      //   ci.required = false;

      //   drunner_assert(it->second.IsMap(),"service.yml format is incorrect - the configuration "+ci.name+" must contain a map of properties.");
      //   YAML::Node value = it->second;
      //   if (value["description"].IsDefined())
      //      ci.description = value["description"].as<std::string>();
      //   if (value["default"].IsDefined())
      //      ci.defaultval = value["default"].as<std::string>();
      //   if (value["required"].IsDefined())
      //      ci.required = value["required"].as<bool>();

      //   mConfigItems.push_back(ci);
      //}

      return kRSuccess;
   }

   file::file(Poco::Path path) : simplefile(path)
   {
   }

   //void _setoperation(std::string opline, const variables &v, CommandLine &op)
   //{
   //   std::string sopline = v.substitute(opline);
   //   if (kRError == utils::split_in_args(sopline, op))
   //      fatal("Empty command line in yaml file");
   //}

   cResult file::loadlua(const variables & v)
   {
      if (kRSuccess != simplefile::loadlua())
         return kRError;
      drunner_assert(mPath.isFile(),"Coding error: path provided to loadyml is not a file.");
      drunner_assert(utils::fileexists(mPath),"The expected file does not exist: "+mPath.toString()); // ctor of simplefile should have set mReadOkay to false if this wasn't true.

      //YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      //drunner_assert(yamlfile,"Unable to read the yaml file: "+mPath.toString());

      //if (yamlfile["volumes"])
      //{ // load volumes
      //   YAML::Node volumes = yamlfile["volumes"];
      //   for (auto it = volumes.begin(); it != volumes.end(); ++it)
      //   {
      //      Volume vol;
      //      vol.name = v.substitute(it->first.as<std::string>());
      //      vol.backup = it->second["backup"].as<bool>();
      //      if (it->second["external"])
      //         vol.external = it->second["external"].as<bool>();
      //      mVolumes.push_back(vol);
      //   }
      //}

      //drunner_assert(yamlfile["help"], "All service.yml files are required to have a help command.");
      //mHelp = v.substitute(yamlfile["help"].as<std::string>());

      return kRSuccess;
   }

   void file::getManageDockerVolumeNames(std::vector<std::string> & vols) const
   {
      drunner_assert(vols.size() == 0,"Coding error: passing dirty volume vector to getManageDockerVolumeNames");
      for (const auto & v : mVolumes)
         if (v.external)
            vols.push_back(v.name);
   }
   void file::getBackupDockerVolumeNames(std::vector<std::string> & vols) const
   {
      drunner_assert(vols.size() == 0, "Coding error: passing dirty volume vector to getBackupDockerVolumeNames");
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