
#ifndef __DRUNNER_SETTINGS_H
#define __DRUNNER_SETTINGS_H

class drunner_settings
{
public:
   drunner_settings(std::string rootpath);

   bool readSettings();
   bool writeSettings();

   std::string mRootPath;
   std::string mSupportImage;
   std::string mRootUtilImage;
   std::string mdrunnerInstallURL;
   std::string mdrunnerInstallTime;

   
}



#endif
