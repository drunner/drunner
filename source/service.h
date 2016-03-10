#ifndef __SERVICE_H
#define __SERVICE_H

#include "params.h"
#include "sh_drunnercfg.h"

class service
{
public:
   service(const std::string & servicename, const sh_drunnercfg & settings, const params & prms);

   std::string getPath() const;
   std::string getPathdRunner() const;
   std::string getPathTemp() const;
   std::string getPathServiceRunner() const;
   std::string getPathVariables() const;
   std::string getName() const;

   void setName(const std::string & servicename);
   void ensureDirectoriesExist();

   bool isValid() const;

   void servicecmd();
   void update();

private:
   std::string mName;
   const sh_drunnercfg & mSettings;
   const params & mParams;
};



#endif

