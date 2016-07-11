#ifndef __SERVICECLASS_H
#define __SERVICECLASS_H

#include "params.h"
#include "drunner_settings.h"
#include "cresult.h"
#include "service_paths.h"

class drunnerCompose;

cResult service_restore(const std::string & servicename, const std::string & backupfile);

class service_obliterate : public servicePaths
{
public:
   service_obliterate(const std::string & servicename);
   eResult obliterate();
};

class cServiceEnvironment : protected settingsbash
{
   public:
      cServiceEnvironment(const servicePaths & paths);

      void save_environment(std::string key, std::string value);
      std::string get_value(const std::string & key) const;

      unsigned int getNumVars() const;
      std::string index2key(unsigned int i) const;

protected:
   Poco::Path mPath;
};


// class to manage the dService.
class service : public servicePaths
{
public:
   // will load imagename from variables.sh unless overridden with parameter.
   service(const std::string & servicename, std::string imagename = "" );

   bool isValid() const;

   cResult servicecmd();

   eResult uninstall();
   eResult obliterate();
   eResult recover();
   int status();
   void update();
   void install();
   void recreate(bool updating);
   void backup(const std::string & backupfile);
   void enter(); // uses execl, so never returns.

   const std::string getImageName() const;
   const params & getParams() const;

   cResult serviceRunnerCommand(const std::vector<std::string> & args) const;
   cServiceEnvironment & getEnvironment();
   const cServiceEnvironment & getEnvironmentConst() const;

private:
   void ensureDirectoriesExist() const;
   void createVolumes(const drunnerCompose * const drc);
   void createLaunchScript() const;
   std::string getUserID(std::string imagename) const;

   static std::string loadImageName(const std::string & servicename, std::string imagename);

   const std::string mImageName;
   cServiceEnvironment mEnvironment;
};



#endif

