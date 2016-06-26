#ifndef __BACKUPCONFIG_H
#define __BACKUPCONFIG_H

#include <string>
#include <vector>
#include "enums.h"

class backupConfig
{
public:
   backupConfig();
   bool load();
   bool save();
   std::string configfilepath();
   bool isEnabled(std::string servicename);
   eResult enable(std::string servicename);
   eResult disable(std::string servicename);
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