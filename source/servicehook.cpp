#include "enums.h"
#include "service.h"
#include "servicehook.h"
#include "logmsg.h"
#include "utils.h"
#include "sh_servicecfg.h"

servicehook::servicehook(const service * const svc, std::string actionname, std::string hookparams, const params & p) :
   mActionName(actionname), mHookParams(hookparams), mParams(p)
{
   setNeedsHook(svc);
}

eResult servicehook::starthook()
{
   if (mStartCmd.length()>0)
     return runHook(mStartCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " start",mParams);
   return kRNoChange;
}

eResult servicehook::endhook()
{
   if (mEndCmd.length()>0)
      return runHook(mEndCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " end", mParams);
   return kRNoChange;
}

eResult servicehook::runHook(std::string se)
{
   std::string op;
   int rval = utils::bashcommand(mServiceRunner + " " + se + " " + mHookParams, op);
   if (rval != 0 && rval != 3)
   {
      logmsg(kLWARN, se + " failed.", mParams);
      return kRError;
   }

   logmsg(kLDEBUG, "dService hook for " + se + " complete", mParams);
   if (mHookParams.length() > 0)
      logmsg(kLDEBUG, "  (params were: " + mHookParams + ")", mParams);
   return eResult(rval);
}


void servicehook::setNeedsHook(const service * const svc)
{
   sh_servicecfg sc(svc->getPathServiceCfg());
   if (!sc.readOkay())
      logmsg(kLERROR,"Service is broken (can't read servicecfg.sh: " + svc->getName(), mParams);

   mServiceRunner = svc->getPathServiceRunner();
   mStartCmd = "";
   mEndCmd = "";

   if (sc.getVersion() == 1)
   { // cope with old version - it expected a bunch of hooks to be present.
      if (utils::stringisame(mActionName, "backup"))
      {
         mStartCmd = "backupstart";
         mEndCmd = "backupend";
      }
      if (utils::stringisame(mActionName, "update"))
      {
         mStartCmd = "updatestart";
         mEndCmd = "updateend";
      }
      if (utils::stringisame(mActionName, "restore"))
         mEndCmd = "restore";
      if (utils::stringisame(mActionName, "install"))
         mEndCmd = "install";
      if (utils::stringisame(mActionName, "obliterate"))
         mStartCmd = "obliterate";
      if (utils::stringisame(mActionName, "uninstall"))
         mStartCmd = "uninstall";
   }
   else
   { // version 2 and up.
      if (sc.hasHook(mActionName) || sc.hasHook(mActionName + "_start"))
         mStartCmd = mActionName + "_start";

      if (sc.hasHook(mActionName) || sc.hasHook(mActionName + "_end"))
         mStartCmd = mActionName + "_end";

      // some hooks don't make sense because the dService won't exist at that point.
      if (utils::findStringIC(mStartCmd, "install_start"))
         logmsg(kLERROR, "dService wants impossible hook " + mStartCmd, mParams);
      if (utils::findStringIC(mEndCmd,"uninstall_end obliterate_end enter_end"))
         logmsg(kLERROR, "dService wants impossible hook " + mStartCmd, mParams);
   }
}

