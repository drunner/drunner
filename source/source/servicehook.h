#ifndef __SERVICE_HOOK_H
#define __SERVICE_HOOK_H

#include <string>
#include <Poco/Path.h>

#include "enums.h"
#include "params.h"
#include "cresult.h"
#include "service_lua.h"
#include "service_paths.h"

class service;

// class to allow dService to hook into any supported action.
class servicehook
{
public:
   //servicehook(std::string servicename, const servicelua::luafile & lfile, std::string actionname, const std::vector<std::string> & hookparams);
   //servicehook(std::string servicename, const servicelua::luafile & lfile, std::string actionname);
   servicehook(std::string servicename, std::string actionname, const std::vector<std::string> & hookparams);
   servicehook(std::string servicename, std::string actionname);
   cResult starthook();
   cResult endhook();
private:
   void setHookCmds();
   cResult runHook(std::string se);

   // ----
   servicePaths mPaths;
   servicelua::luafile mLua;
   std::string mActionName;
   std::vector<std::string> mHookParams;
   std::string mStartCmd;
   std::string mEndCmd;
};

#endif
