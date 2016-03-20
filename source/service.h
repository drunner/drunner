#ifndef __SERVICECLASS_H
#define __SERVICECLASS_H

#include "params.h"
#include "sh_drunnercfg.h"
#include "cresult.h"

class sh_variables;

// class to manage the dService.
class service
{
public:
   service(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename = "" );

   std::string getPath() const;
   std::string getPathdRunner() const;
   std::string getPathTemp() const;
   std::string getPathServiceRunner() const;
   std::string getPathVariables() const;
   std::string getPathServiceCfg() const;
   std::string getName() const;

   bool isValid() const;
   void validateImage();

   cResult servicecmd();

   eResult uninstall();
   eResult obliterate();
   eResult recover();
   int status();
   void update();
   void install();
   void recreate(bool updating);
   void backup(const std::string & backupfile);
   void restore(const std::string & backupfile);
   void enter(); // uses execl, so never returns.

   const params & getParams() const;

   const std::string getImageName() const;

private:
   void setImageName(std::string imagename);
   void ensureDirectoriesExist() const;
   void createVolumes(const sh_variables * variables);
   void createLaunchScript();
   std::string getUserID();
   void logmsg(eLogLevel level, std::string s) const;

   const std::string mName;
   std::string mImageName;
   const sh_drunnercfg & mSettings;
   const params & mParams;
};

#endif

