#include "yaml-cpp/yaml.h"

#include "service_yml.h"
#include "utils.h"
#include "globallogger.h"

namespace serviceyml
{
   //volume::volume(std::string name, const YAML::Node & element) : mName(name)
   //{
   //   mBackedup = true;
   //   mManaged = true;

   //   poco_assert(element.IsMap());
   //   for (auto it = element.begin(); it != element.end(); ++it)
   //   {
   //      std::string prop = it->first.as<std::string>();
   //      switch (utils::str2int(prop.c_str()))
   //      {
   //      case utils::str2int("manage"):
   //         mManaged = it->second.as<bool>();
   //         break;

   //      case utils::str2int("backup"):
   //         mBackedup = it->second.as<bool>();
   //         break;

   //      default:
   //         fatal("Unrecognised volume property: " + prop);
   //      }
   //   }
   //}

/*
   configitem::configitem(std::string name, const YAML::Node & element)
   {
         if (!element.IsMap())
            logmsg(kLERROR, "Configuration entries in the YAML file must be maps.");

         mName = name;
         mRequired = true;
         mDescription = "No description set.";
         mDefault = "";
         mType = kCF_string;

         for (auto keyvals = element.begin(); keyvals != element.end(); ++keyvals)
         {
            std::string key = keyvals->first.as<std::string>();
            std::string val = keyvals->second.as<std::string>();
            switch (utils::str2int(key.c_str()))
            {
            case utils::str2int("description"):
               mDescription = val;
               break;

            case utils::str2int("required"):
               if (val.length() > 0 && (::tolower(val[0]) == 'n' || ::tolower(val[0]) == 'f'))
                  mRequired = false;
               break;

            case utils::str2int("default"):
               mDefault = val;
               break;

            case utils::str2int("type"):
               logmsg(kLWARN, "Types not yet implemented.");
               break;

            default:
               logmsg(kLERROR, "Unkown key: " + key);
            }
         }
   }
*/

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
         YAML::Node value = it->second;
         ci.description = value["description"].as<std::string>();
         ci.defaultval = value["default"].as<std::string>();
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



/*
   operation::operation(std::string s)
   {
      if (s.length() == 0)
         fatal("empty string passed to operation");
      utils::split_in_args(s, mArgs);
      if (mArgs.size() == 0)
         fatal("whitespace string passed to operation");
      mCommand = mArgs[0];
      mArgs.erase(mArgs.begin());
   }

   commandline::commandline(std::string name) : mName(name)
   {
   }

   void commandline::addoperation(operation o)
   {
      mOperations.push_back(o);
   }

   std::string commandline::getName() const
   {
      return mName;
   }

   const std::vector<operation> & commandline::getOperations() const
   {
      return mOperations;
   }

   const std::vector<std::string> & simplefile::getExtraContainers() const
   {
      return mExtraContainers;
   }

   const std::vector<configitem> & simplefile::getConfigItems() const
   {
      return mConfigItems;
   }

*/
} // namespace