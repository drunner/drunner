#include "service_yml.h"
#include "utils.h"
#include "globallogger.h"

namespace serviceyml
{

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
               if (val.length() > 0 && ::tolower(val[0]) == 'n' || ::tolower(val[0]) == 'f')
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


   simplefile::simplefile(Poco::Path path)
   {
      mReadOkay = false;
      poco_assert(path.isFile());
      if (!utils::fileexists(path))
         return;

      YAML::Node yamlfile = YAML::LoadFile(path.toString());
      if (!yamlfile)
      {
         logmsg(kLDEBUG, "Unable to read yaml file from " + path.toString());
         return;
      }

      if (yamlfile["extracontainers"])
      { // load containers
         YAML::Node containers = yamlfile["containers"];
         if (containers.Type() != YAML::NodeType::Sequence)
            logmsg(kLERROR, "Error: Containers section in YAML file must be a sequence.");
         for (auto it = containers.begin(); it != containers.end(); ++it)
         {
            logmsg(kLDEBUG,"Added container " + it->as<std::string>());
            mExtraContainers.push_back(it->as<std::string>());
         }
      }

      if (yamlfile["configuration"])
      { // load configuration
         YAML::Node configuration = yamlfile["configuration"];
         for (auto it = configuration.begin(); it != configuration.end(); ++it)
         {
            std::string name = it->first.as<std::string>();
            YAML::Node element(it->second.as<YAML::Node>());
            configitem ci(name,element);
            mConfigItems.push_back(ci);
         }
      }
      mReadOkay = true;
   }

   file::file(Poco::Path path, const variables & v) : simplefile(path)
   {
      if (!mReadOkay)
         return; // ctor of simplefile failed to read file.
      poco_assert(path.isFile());
      poco_assert(utils::fileexists(path)); // ctor of simplefile should have set mReadOkay to false if this wasn't true.

      YAML::Node yamlfile = YAML::LoadFile(path.toString());
      poco_assert(yamlfile);

      if (yamlfile["commands"])
      {// load commands.

      }
   }

   void file::getDockerVolumeNames(std::vector<std::string> & vols) const
   {
      poco_assert(vols.size() == 0);
      for (const auto & v : mVolumes)
         vols.push_back(v.name());
   }


} // namespace