#ifndef __SERVICE_HOOK_H
#define __SERVICE_HOOK_H

#include <string>
#include <Poco/Path.h>

#include "enums.h"
#include "params.h"
#include "cresult.h"
#include "service_yml.h"

class service;

// class to allow dService to hook into any supported action.
class servicehook
{
public:
   servicehook(const service * const svc, std::string actionname, const std::vector<std::string> & hookparams);
   servicehook(const service * const svc, std::string actionname);
   cResult starthook();
   cResult endhook();
private:
   void setHookCmds();
   cResult runHook(std::string se);

   const service * const mService;
   std::string mActionName;
   std::vector<std::string> mHookParams;

   //Poco::Path mServiceRunner;
   std::string mStartCmd;
   std::string mEndCmd;
};

#endif
