#ifndef __BACKUPCONFIG_H
#define __BACKUPCONFIG_H

#include <string>
#include <vector>
#include <Poco/Path.h>

#include "cresult.h"

class backupConfig
{
public:
   backupConfig();
   bool load();
   bool save();
   Poco::Path configfilepath();
   bool isEnabled(std::string servicename);
   cResult enable(std::string servicename);
   cResult disable(std::string servicename);
   void update(std::string backuppath);
   std::string getBackupPath();

private:
   std::vector<std::string> mDisabledServices;
   std::string mBackupPath;
   bool mIsConfigured;

   static const std::string s_DisabledServices;
   static const std::string s_BackupPath;
};

#endif