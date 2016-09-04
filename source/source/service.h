#ifndef __SERVICECLASS_H
#define __SERVICECLASS_H

#include <memory>

#include "params.h"
#include "drunner_settings.h"
#include "cresult.h"
#include "service_paths.h"
#include "service_lua.h"
#include "service_vars.h"

// class to manage the dService.
class service : public servicePaths
{
public:
   service(std::string servicename); 

   const std::string getImageName() const;
   const servicelua::luafile & getServiceLua() const { return mServiceLua; }
   const serviceVars & getServiceVars() const { return *mServiceVarsPtr; }


   cResult backup(std::string backupfile);
   cResult servicecmd();

private:
   cResult _runserviceRunnerCommand(const CommandLine & serviceCmd) const;
   cResult _dstop(const CommandLine & operation) const;

   servicelua::luafile mServiceLua;
   std::unique_ptr<serviceVars> mServiceVarsPtr;
};



#endif

