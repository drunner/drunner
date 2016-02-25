#include <map>

#ifndef __SETTINGSBASH_H
#define __SETTINGSBASH_H

class settingsbash
{
public:
   settingsbash(std::string settingspath); // sets defaults, reads from config.sh if present.

   bool readSettings();
   bool writeSettings() const;

   std::string getSetting(const std::string &  key) const;
   bool getSettingb(const std::string &  key) const;
   void setSetting(const std::string &  key, const std::string & value);
   std::string getPath() const;
private:
   const std::string mPath;
   std::map< std::string, std::string > mSettings;
   bool istrue(const std::string & s) const;
   bool parse(std::string line, std::string & left, std::string & right);
   std::string dequote(const std::string & s);
};

#endif
