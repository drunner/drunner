#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <sstream>

#include "settingsbash.h"
#include "utils.h"
#include "logmsg.h"


settingsbash::settingsbash(std::string settingspath) : mPath(settingspath)
{
}

bool settingsbash::parse(std::string line, std::string & left, std::string & right) const
{
   std::size_t end=0;
   if ((end=line.find("=",0)) != std::string::npos)
      {
         left=utils::trim_copy(line.substr(0,end));
         right=utils::trim_copy(line.substr(end+1)); //If this is equal to the string length, substr returns an empty string.
         if (left.length()==0) return false;    // empty key
         if (left[0]=='#') return false;        // comment line
         return true;
      }
   return false;
}

std::string settingsbash::dequote(const std::string & s, char c) const
{
   std::string ss(s);
   if (ss.length()==0) return ss;
   if (ss[0]==c) ss.erase(0,1);
   if (ss.length()==0) return ss;
   if (ss[ss.length()-1]==c) ss.erase(ss.length()-1,std::string::npos);
   return ss;
}

bool settingsbash::readSettings()
{
   if (! utils::fileexists(mPath))
      return false;

   std::string line,left,right;
   std::ifstream configfile( mPath.c_str() );
   while (std::getline(configfile, line))
   {
      if (parse(line,left,right))
         {
         if (mSettings.find(left)!=mSettings.end()) // we only update entries, not add random stuff.
            mSettings[left]=dequote(right,'\"');
         }
   }
   configfile.close();

   return true;
}

bool settingsbash::writeSettings() const
{
   if (utils::fileexists(mPath))
      std::remove(mPath.c_str());

   std::ofstream ofile( mPath.c_str() );
   if (!ofile.is_open()) return false; // can't open the file.

   ofile << "# dRunner configuration file." << std::endl;
   ofile << "# You can edit this file - user changes are preserved on update." << std::endl << std::endl;

  // iterate through map. C++11 style.
  for (auto const &entry : mSettings) {
     ofile << entry.first << "=\"" << entry.second << "\"" << std::endl;
  }
  ofile.close();
  return true;
}

bool settingsbash::istrue(const std::string & s) const
{
   if (s.length()==0) return false;
   return (tolower(s[0])=='y' || s[0]=='1' || tolower(s[0])=='t');
}

std::string settingsbash::getSetting(const std::string &  key) const
{
   if (mSettings.find(key)!=mSettings.end()) // we only update entries, not add random stuff.
      return mSettings.at(key);
   logmsg(kLWARN,"Couldn't find setting "+key,kLWARN);
   return "";
}
bool settingsbash::getSettingb(const std::string &  key) const
{
   if (mSettings.find(key)!=mSettings.end()) // we only update entries, not add random stuff.
      return istrue(mSettings.at(key));
   logmsg(kLWARN,"Couldn't find setting "+key,kLWARN);
   return false;
}
void settingsbash::getSettingv(const std::string &  key, std::vector<std::string> & vec) const
{ // needs to be compatible with bash scripts!!
   std::string astr=dequote(dequote(getSetting(key),'('),')');
   std::stringstream ss(astr);
   while (ss.good())
      {
         ss >> astr;
         vec.push_back(dequote(astr,'\"'));
      }
}

void settingsbash::setSetting(const std::string &  key, const std::string & value)
{
   mSettings[key]=dequote(value,'\"');
}
void settingsbash::setSettingb(const std::string &  key, bool value)
{
   mSettings[key] = value ? "yes" : "no";
}
void settingsbash::setSettingv(const std::string &  key, const std::vector<std::string> vec)
{
   std::stringstream ss;
   ss << key << "=(";
   for (uint i=0;i<vec.size();++i)
   {
      if (i>0) ss << " ";
      ss << "\"" << vec[i] << "\"";
   }
   ss << ")";
   setSetting(key,ss.str());
}


std::string settingsbash::getPath() const
{
   return mPath;
}
