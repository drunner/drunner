#include "enums.h"
#include "service.h"
#include "servicehook.h"
#include "globallogger.h"
#include "utils.h"
#include "cresult.h"

//servicehook::servicehook(std::string servicename, const servicelua::luafile & lfile, std::string actionname, const std::vector<std::string> & hookparams) :
//   mLua(lfile), mPaths(servicename), mActionName(actionname), mHookParams(hookparams)
//{
//   setHookCmds();
//}
//
//servicehook::servicehook(std::string servicename, const servicelua::luafile & lfile, std::string actionname) :
//   mLua(lfile), mPaths(servicename), mActionName(actionname)
//{
//   setHookCmds();
//}

servicehook::servicehook(std::string servicename, std::string actionname, const std::vector<std::string> & hookparams) :
   mPaths(servicename), mLua(servicename), mActionName(actionname),  mHookParams(hookparams)
{
}


servicehook::servicehook(std::string servicename, std::string actionname) :
   mPaths(servicename), mLua(servicename), mActionName(actionname)
{
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
   if (!utils::fileexists(mPaths.getPathServiceLua()))
      return cError("Failed to find service.lua at " + mPaths.getPathServiceLua().toString());

   cResult rval = mLua.loadlua();
   CommandLine serviceCmd(se, mHookParams);
   rval += mLua.runCommand(serviceCmd);

   if (rval.error())
      return cError("dService hook " + se + " returned error: "+rval.what());

   if (rval.notImpl())
   {
      logdbg("dService hook " + se + " is not implemented by " + mPaths.getName());
      rval = kRNoChange; // fine to be not implemented for hooks.
   }
   else
      logdbg("dService hook for " + se + " complete");

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

