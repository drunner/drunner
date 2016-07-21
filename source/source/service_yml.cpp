#include "yaml-cpp/yaml.h"

#include "service_yml.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"

namespace serviceyml
{
 
   simplefile::simplefile(Poco::Path path) : mPath(path)
   {
      drunner_assert(mPath.isFile(),"Coding error: path provided to simplefile is not a file!");
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
      {
         drunner_assert(it->IsScalar(), "Containers must be a simple sequence of strings (container names).");
         mContainers.push_back(it->as<std::string>());
      }

      // load configuration
      YAML::Node configuration = yamlfile["configuration"];
      for (auto it = configuration.begin(); it != configuration.end(); ++it)
      {
         Configuration ci;
         drunner_assert(it->first.IsScalar(), "Configuration items must be a map of maps.");
         ci.name = it->first.as<std::string>();
         ci.required = false;

         drunner_assert(it->second.IsMap(),"service.yml format is incorrect - the configuration "+ci.name+" must contain a map of properties.");
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

   void _setoperation(std::string opline, const variables &v, Operation &op)
   {
      std::string sopline = v.substitute(opline);
      utils::split_in_args(sopline, op.args);
      drunner_assert(op.args.size() > 0, "Empty command line in yaml file");
      op.command = op.args[0];
      op.args.erase(op.args.begin());
   }

   cResult file::loadyml(const variables & v)
   {
      if (kRSuccess != simplefile::loadyml())
         return kRError;
      drunner_assert(mPath.isFile(),"Coding error: path provided to loadyml is not a file.");
      drunner_assert(utils::fileexists(mPath),"The expected file does not exist: "+mPath.toString()); // ctor of simplefile should have set mReadOkay to false if this wasn't true.

      YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      drunner_assert(yamlfile,"Unable to read the yaml file: "+mPath.toString());

      if (yamlfile["commands"])
      {// load commands.
         YAML::Node commands = yamlfile["commands"];
         for (auto it = commands.begin(); it != commands.end(); ++it)
         {
            CommandLine cl;
            drunner_assert(it->first.IsScalar(), "Command lines must be a map of sequences.");
            cl.name = it->first.as<std::string>();
            Operation op;
            if (it->second.IsScalar())
            {
               Operation op;
               _setoperation(it->second.as<std::string>(), v, op);
               cl.operations.push_back(op);
            }
            else
            {
               drunner_assert(it->second.IsSequence(), "Command " + cl.name + " must contain a sequence of commands to run.");
               for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
               {
                  Operation op;
                  _setoperation(it2->as<std::string>(), v, op);
                  cl.operations.push_back(op);
               }
            }
            mCommands.push_back(cl);
         }
      }

      if (yamlfile["volumes"])
      { // load volumes
         YAML::Node volumes = yamlfile["volumes"];
         for (auto it = volumes.begin(); it != volumes.end(); ++it)
         {
            Volume vol;
            vol.name = v.substitute(it->first.as<std::string>());
            vol.backup = it->second["backup"].as<bool>();
            if (it->second["external"])
               vol.external = it->second["external"].as<bool>();
            mVolumes.push_back(vol);
         }
      }

      drunner_assert(yamlfile["help"], "All service.yml files are required to have a help command.");
      mHelp = v.substitute(yamlfile["help"].as<std::string>());

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