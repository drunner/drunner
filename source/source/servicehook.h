#ifndef __SERVICE_HOOK_H
#define __SERVICE_HOOK_H

#include <string>
#include <Poco/Path.h>

#include "enums.h"
#include "params.h"
#include "cresult.h"
#include "service_lua.h"
#include "service_paths.h"

// class to allow dService to hook into any supported action.
class servicehook
{
public:
   servicehook(std::string servicename, std::string actionname, const std::vector<std::string> & hookparams);
   cResult starthook();
   cResult endhook();
private:
   cResult _runHook(std::string se);
   cResult _runHook_NoService(std::string se);

   // ----
   servicePaths mPaths;
   std::string mActionName;
   std::vector<std::string> mHookParams;
   std::string mStartCmd;
   std::string mEndCmd;
};

#endif
