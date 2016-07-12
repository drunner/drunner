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
      std::string name() { return mName; }

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

   class file {
   public:
      file(Poco::Path path, const variables & v); // reads file, applies variable substitution. Throws if bad.

      const std::vector<volume> & getVolumes() { return mVolumes; }
      const std::vector<configitem> & getConfigItems() { return mConfigItems; }
      const std::vector<commandline> & getCommands() { return mCommands; }
      std::string getHelp() { return mHelp; }
      bool readokay() { return mReadOkay; }

   private:
      std::vector<volume> mVolumes;
      std::vector<configitem> mConfigItems;
      std::vector<commandline> mCommands;
      std::string mHelp;
      bool mReadOkay;
   };

} // namespace

#endif
