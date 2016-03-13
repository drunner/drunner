#include "enums.h"
#include "service.h"
#include "servicehook.h"
#include "logmsg.h"
#include "utils.h"
#include "sh_servicecfg.h"
#include "cresult.h"

servicehook::servicehook(const service * const svc, std::string actionname, const std::vector<std::string> & hookparams, const params & p) :
   mActionName(actionname), mHookParams(hookparams), mParams(p)
{
   setNeedsHook(svc);
}

cResult servicehook::starthook()
{
   if (mStartCmd.length() > 0)
      return runHook(mStartCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " start",mParams);
   return kRNoChange;
}

cResult servicehook::endhook()
{
   if (mEndCmd.length()>0)
      return runHook(mEndCmd);

   logmsg(kLDEBUG, "dService doesn't require hook for " + mActionName + " end", mParams);
   return kRNoChange;
}

cResult servicehook::runHook(std::string se)
{
   if (!utils::fileexists(mServiceRunner))
   {
      logmsg(kLWARN, "Couldn't run hook " + se + " because dService's servicerunner is not installed.", mParams);
      return kRError;
   }

   std::vector<std::string> args;
   args.push_back("servicerunner");
   args.push_back(se);
   for (const auto & entry : mHookParams)
      args.push_back(entry);

   cResult rval = utils::dServiceCmd(mServiceRunner, args, mParams);
   if (rval == kRNotImplemented)
      rval=kRNoChange; // not implemented is perfectly fine for hooks.

   if (rval == kRError)
      logmsg(kLWARN, "dService hook " + se + " returned error.", mParams);
   else
      logmsg(kLDEBUG, "dService hook for " + se + " complete", mParams);

   return rval;
}


void servicehook::setNeedsHook(const service * const svc)
{
   sh_servicecfg sc(svc->getPathServiceCfg());
   if (!sc.readOkay())
      logmsg(kLWARN,"Service is broken (can't read servicecfg.sh: " + svc->getName()+")", mParams);

   mServiceRunner = svc->getPathServiceRunner();
   mStartCmd = "";
   mEndCmd = "";

   if (sc.getVersion() == 1)
   { // cope with old version - it expected a bunch of manual hooks to be present.
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
      mStartCmd = mActionName + "_start";
      mStartCmd = mActionName + "_end";

      // some hooks don't make sense because the dService won't exist at that point.
      if (utils::findStringIC(mStartCmd, "install_start"))
         mStartCmd = "";
      if (utils::findStringIC(mEndCmd, "uninstall_end obliterate_end enter_end"))
         mEndCmd = "";
   }
}

