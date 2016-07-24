#include "enums.h"
#include "service.h"
#include "servicehook.h"
#include "globallogger.h"
#include "utils.h"
#include "cresult.h"

servicehook::servicehook(const service * const svc, std::string actionname, const std::vector<std::string> & hookparams) :
   mService(svc), mActionName(actionname), mHookParams(hookparams)
{
   setHookCmds();
}

servicehook::servicehook(const service * const svc, std::string actionname) :
   mService(svc), mActionName(actionname)
{
   setHookCmds();
}

cResult servicehook::starthook()
{
   if (mStartCmd.length() > 0)
      return runHook(mStartCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " start");
   return kRNoChange;
}

cResult servicehook::endhook()
{
   if (mEndCmd.length()>0)
      return runHook(mEndCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " end");
   return kRNoChange;
}

cResult servicehook::runHook(std::string se)
{
   if (!utils::fileexists(mService->getPathServiceLua()))
   {
      logmsg(kLDEBUG, "Failed to find service.lua at " + mService->getPathServiceLua().toString());
      logmsg(kLWARN, "Couldn't run hook " + se + " because dService's service.lua is not installed.");
      return kRError;
   }

   CommandLine serviceCmd(se, mHookParams);
   cResult rval = mService->serviceRunnerCommand(serviceCmd);

   if (rval.isError())
      logmsg(kLWARN, "dService hook " + se + " returned error.");
   else if (rval.isNOIMPL())
   {
      logmsg(kLDEBUG, "dService hook " + se + " is not implemented by " + mService->getImageName());
      rval = kRNoChange; // fine to be not implemented for hooks.
   }
   else
      logmsg(kLDEBUG, "dService hook for " + se + " complete");

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
   if (utils::findStringIC(" install_start "," "+mStartCmd+" "))
      mStartCmd = "";
   if (utils::findStringIC(" uninstall_end obliterate_end "," "+mEndCmd+" "))
      mEndCmd = "";
}

