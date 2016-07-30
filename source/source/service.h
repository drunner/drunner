#ifndef __SERVICECLASS_H
#define __SERVICECLASS_H

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
   // will load imagename from variables.sh unless overridden with parameter.
   service(std::string servicename); 

   cResult servicecmd();

   int status();
   void backup(const std::string & backupfile);
   //void enter(); // uses execl, so never returns.

   const std::string getImageName() const;
   const params & getParams() const;

   cResult serviceRunnerCommand(const CommandLine & serviceCmd) const;

   const servicelua::luafile & getServiceLua() const { return mServiceLua; }
   //const serviceVars & getServiceVars() const { return mServiceVars; }
   //cServiceEnvironment & getEnvironment();
   //const cServiceEnvironment & getEnvironmentConst() const;

private:
   static std::string _loadImageName(const std::string & servicename, std::string imagename);
   cResult _handleconfigure(const CommandLine & operation);
   cResult _showconfiginfo();
   cResult _runserviceRunnerCommand(const CommandLine & serviceCmd) const;
   cResult _launchCommandLine(const CommandLine & operation) const;
   cResult _handleStandardCommands(const CommandLine & operation, bool & processed) const;

   cResult _dstop(const CommandLine & operation) const;

   std::string mImageName;
   servicelua::luafile mServiceLua;
};



#endif

