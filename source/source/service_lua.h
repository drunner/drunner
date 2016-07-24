#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <string>
#include <vector>

#include <Poco/Path.h>
#include "cresult.h"
#include "utils.h"
#include "service_vars.h"
#include "service_paths.h"

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


   // lua file.
   class luafile {
   public:
      luafile(const servicePaths & p);
      
      // loads the lua file, initialises the variables, loads the variables if able.
      cResult loadlua();

      // saves the variables.
      cResult saveServiceVars();

      std::string getHelp() const { return mHelp; }
      std::string getImageName() const;

      const std::vector<std::string> & getContainers() const;
      const std::vector<Configuration> & getConfigItems() const;
      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<std::string> & vols) const;

   private:
      cResult safeloadvars();

      const servicePaths & mServicePaths;
      
      serviceVars mServiceVars;

      std::vector<std::string> mContainers;
      std::vector<Configuration> mConfigItems;
      std::vector<Volume> mVolumes;

      std::string mHelp;
   };

} // namespace

#endif
