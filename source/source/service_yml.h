#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <Poco/Path.h>
#include "yaml-cpp/yaml.h"
#include "service_variables.h"
#include "cresult.h"


namespace serviceyml
{

   class volume {
   public:
      volume(std::string name, const YAML::Node & element);
      std::string name() const   { return mName; }
      bool isBackedUp() const    { return mBackedup; }
      bool isManaged() const     { return mManaged; }
   private:
      bool mBackedup;
      bool mManaged;
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
      configitem(std::string name, const YAML::Node & element);

      bool required() const            { return mRequired; }
      std::string defaultvalue() const { return mDefault; }
      std::string name() const         { return mName; }
      std::string description() const  { return mDescription; }
      configtype type() const          { return mType; }

   private:
      std::string mName;
      std::string mDefault;
      std::string mDescription;
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
      commandline(std::string name);
      void addoperation(operation o);
      std::string getName() const;
      const std::vector<operation> & getOperations() const;
      
   private:
      std::string mName;
      std::vector<operation> mOperations;
   };

   class simplefile {
   public: 
      simplefile(Poco::Path path);

      const std::vector<std::string> & getExtraContainers() const; 
      const std::vector<configitem> & getConfigItems() const;
      
      virtual cResult loadyml();

   protected:
      std::vector<std::string> mExtraContainers;
      std::vector<configitem> mConfigItems;
      const Poco::Path mPath;
   };

   class file : public simplefile {
   public:
      file(Poco::Path path); // reads file, throws if bad. Applies variable substitution using variables.
      
      cResult loadyml(const variables & v);

      const std::vector<volume> & getVolumes() const        { return mVolumes; }
      const std::vector<commandline> & getCommands() const  { return mCommands; }
      std::string getHelp() const                           { return mHelp; }

      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<std::string> & vols) const;

   private:
      std::vector<volume> mVolumes;
      std::vector<commandline> mCommands;
      std::string mHelp;
   };

} // namespace

#endif
