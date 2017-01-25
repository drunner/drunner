#include "enums.h"
#include "service.h"
#include "servicehook.h"
#include "globallogger.h"
#include "utils.h"
#include "cresult.h"
#include "service_vars.h"
#include "globalcontext.h"

servicehook::servicehook(std::string servicename, std::string actionname, const std::vector<std::string> & hookparams) :
   mPaths(servicename), mActionName(actionname),  mHookParams(hookparams)
{
}

cResult servicehook::starthook()
{
   return _runHook(mActionName + "_start");
}

cResult servicehook::endhook()
{
   return _runHook(mActionName + "_end");
}

cResult servicehook::_runHook(std::string se)
{
   // handle cases where there is no service expected.
   if (utils::findStringIC("|install_start|uninstall_end|obliterate_end|", "|" + se + "|"))
      return GlobalContext::getPlugins()->runhook(se, mHookParams);

   // we expect a working service, so load the service.lua and service variables.
   if (!utils::fileexists(mPaths.getPathServiceLua()))
      return cError("Failed to find service.lua at " + mPaths.getPathServiceLua().toString());

   servicelua::luafile lf(mPaths.getName());
   cResult rval = lf.loadlua();
   if (rval != kRSuccess)
      return rval;

   serviceVars sv(mPaths.getName(),lf.getLuaConfigurationDefinitions());
   rval += sv.loadvariables();
   if (rval != kRSuccess)
      return rval;

   { // allow service to hook the command
      CommandLine serviceCmd(se, mHookParams);
      rval += lf.runCommand(serviceCmd, &sv);

      if (rval.error())
         return cError("dService hook " + se + " returned error:\n " + rval.what());

      if (rval.notImpl())
      {
         logdbg("dService hook " + se + " is not implemented by " + mPaths.getName());
         rval = kRNoChange; // fine to be not implemented for hooks.
      }
      else
         logdbg("dService hook for " + se + " complete");
   }

   { // allow plugins to hook the command
      rval += GlobalContext::getPlugins()->runhook(se, mHookParams, &lf, &sv);
      if (rval.error())
         return cError("Plugin hook "+se+" returned error:\n "+rval.what());
   }

   return rval;
}

