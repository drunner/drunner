#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <Poco/Path.h>
#include "yaml-cpp/yaml.h"
#include "service_variables.h"

namespace serviceyml
{

   class volume {
   public:
      volume(std::string s);
      std::string name() const { return mName; }

   private:
      std::string mName;
   };

   enum configtype
   {
      kCF_port,
      kCF_path,
      kCF_existingpath,
      kCF_string,
   };

   class configitem {
   public:
      configitem(YAML::Node & node);

      bool required() const { return mRequired; }
      std::string defaultvalue() const { return mDefault; }
      std::string name() const { return mName; }
      configtype type() const { return mType; }

   private:
      std::string mName;
      std::string mDefault;
      configtype mType;
      bool mRequired;
   };

   class operation {
   public:
      operation(std::string s);

      std::string command() const { return mCommand; }
      const std::vector<std::string> & getArgs() const { return mArgs; }

   private:
      std::string mCommand;
      std::vector<std::string> mArgs;
   };

   class commandline {
   public:
      commandline(YAML::Node & node);

   private:
      std::vector<operation> mOperations;
   };

   class simplefile {
   public: 
      simplefile(Poco::Path path);

      const std::vector<std::string> & getContainers() const { return mContainers; }
      const std::vector<configitem> & getConfigItems() const { return mConfigItems; }

      bool readokay() { return mReadOkay; }

   protected:
      std::vector<std::string> mContainers;
      std::vector<configitem> mConfigItems;
      bool mReadOkay;
   };

   class file : public simplefile {
   public:
      file(Poco::Path path, const variables & v); // reads file, throws if bad. Applies variable substitution using variables.

      const std::vector<volume> & getVolumes() const { return mVolumes; }
      const std::vector<commandline> & getCommands() const { return mCommands; }
      std::string getHelp() { return mHelp; }

      void getDockerVolumeNames(std::vector<std::string> & vols) const;

   private:
      std::vector<volume> mVolumes;
      std::vector<commandline> mCommands;
      std::string mHelp;
   };

} // namespace

#endif
