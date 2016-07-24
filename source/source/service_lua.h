#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <string>
#include <vector>

#include <Poco/Path.h>
#include "service_variables.h"
#include "cresult.h"
#include "utils.h"

namespace servicelua
{

   struct Volume 
   {
      bool backup;
      bool external;
      std::string name;
   };

   enum configtype
   {
      kCF_port,
      kCF_path,
      kCF_existingpath,
      kCF_string,
   };

   struct Configuration 
   {
      std::string name;
      std::string defaultval;
      std::string description;
      configtype type;
      bool required;
   };

   class simplefile {
   public: 
      simplefile(Poco::Path path);

      const std::vector<std::string> & getContainers() const; 
      const std::vector<Configuration> & getConfigItems() const;
      
      virtual cResult loadlua();

   protected:
      std::vector<std::string> mContainers;
      std::vector<Configuration> mConfigItems;
      const Poco::Path mPath;
   };

   class file : public simplefile {
   public:
      file(Poco::Path path); // reads file, throws if bad. Applies variable substitution using variables.
      
      cResult loadlua(const variables & v);

      //const std::vector<Volume> & getVolumes() const        { return mVolumes; }
      //const std::vector<CommandDefinition> & getCommands() const  { return mCommands; }
      std::string getHelp() const                           { return mHelp; }

      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<std::string> & vols) const;

   private:
      std::vector<Volume> mVolumes;
      //std::vector<CommandDefinition> mCommands;
      std::string mHelp;
   };

} // namespace

#endif
