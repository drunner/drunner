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
   setHookCmds();
}


servicehook::servicehook(std::string servicename, std::string actionname) :
   mPaths(servicename), mActionName(actionname)
{
   setHookCmds();
}


cResult servicehook::starthook()
{
   if (mStartCmd.length() > 0)
      return runHook(mStartCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + "_start");
   return kRNoChange;
}

cResult servicehook::endhook()
{
   if (mEndCmd.length()>0)
      return runHook(mEndCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + "_end");
   return kRNoChange;
}

cResult servicehook::runHook(std::string se)
{
   if (!utils::fileexists(mPaths.getPathServiceLua()))
      return cError("Failed to find service.lua at " + mPaths.getPathServiceLua().toString());

   servicelua::luafile lf(mPaths.getName());

   cResult rval = lf.loadlua();
   if (rval != kRSuccess)
      return rval;

   serviceVars sv(mPaths.getName(),lf.getConfigItems());
   rval += sv.loadvariables();
   if (rval != kRSuccess)
      return rval;
   if (sv.getImageName().length() == 0)
      return cError("IMAGENAME was not set in service variables (servicehook::runHook).");

   { // services
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

   { // plugins
      rval += GlobalContext::getPlugins()->runhook(se, mHookParams, lf, sv);
      if (rval.error())
         return cError("Plugin hook "+se+" returned error:\n "+rval.what());
   }

   return rval;
}


void servicehook::setHookCmds()
{      
   //mServiceRunner = mService->getPathServiceRunner();
   mStartCmd = "";
   mEndCmd = "";

   mStartCmd = mActionName + "_start";
   mEndCmd   = mActionName + "_end";

   // some hooks don't make sense because the dService won't exist at that point.
   // spaces are to ensure whole word match.
   if (utils::findStringIC("|install_start|","|"+mStartCmd+"|"))
      mStartCmd = "";
   if (utils::findStringIC("|uninstall_end|obliterate_end|","|"+mEndCmd+"|"))
      mEndCmd = "";
}

