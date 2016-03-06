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


settingsbash::settingsbash(const params & par, std::string settingspath) : mPath(settingspath), p(par)
{
}

bool sbelement::parse(std::string line, std::string & left, std::string & right) const
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

std::string sbelement::dequote(const std::string & s, char c) const
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
      sbelement sbe(line);
      mElements.push_back(sbe);
   }
   configfile.close();

   return true;
}

bool settingsbash::writeSettings(const std::string & fullpath) const
{
   std::string path = (fullpath.length()==0  ? mPath : fullpath );

   if (path.find(".sh")==std::string::npos)
      logmsg(kLERROR,"All bash style settings files should have .sh file extension. Offender: "+fullpath,p);

   if (utils::fileexists(path))
      std::remove(path.c_str());

   std::ofstream ofile( path.c_str() );
   if (!ofile.is_open()) return false; // can't open the file.

   ofile << "# "+path << std::endl;
   ofile << "# dRunner configuration file." << std::endl;
   ofile << "# You can edit this file - user changes are preserved on update." << std::endl << std::endl;

  // iterate through map. C++11 style.
  for (auto const &entry : mElements) {
     ofile << entry.getBashLine() << std::endl;
  }
  ofile.close();
  return true;
}

bool sbelement::istrue(const std::string & s) const
{
   if (s.length()==0) return false;
   return (tolower(s[0])=='y' || tolower(s[0])=='t');
}

const sbelement & settingsbash::getElement(const std::string & key) const
{
   for (uint i=0;i<mElements.size();++i)
      if (utils::stringisame(mElements[i].getKey(),key))
         return mElements[i];
   logmsg(kLERROR,"Couldn't find key "+key,p);
}

std::string settingsbash::getString(const std::string &  key) const
{
   return getElement(key).getString();
}
bool settingsbash::getSettingb(const std::string &  key) const
{
   return getElement(key).getBool();
}
void settingsbash::getSettingv(const std::string &  key, std::vector<std::string> & vec) const
{ // needs to be compatible with bash scripts!!
   getElement(key).getVec(vec);
}

void settingsbash::setSetting(const std::string &  key, const std::string & value)
{
   mSettings[key]='\"'+value+'\"';//dequote(value,'\"');
}
void settingsbash::setSettingb(const std::string &  key, bool value)
{
   mSettings[key] = value ? "yes" : "no";
}
void settingsbash::setSettingv(const std::string &  key, const std::vector<std::string> vec)
{
}


std::string settingsbash::getPath() const
{
   return mPath;
}

sbelement(const std::string & bashline)
{
   std::string left,right;
   if (parse(bashline,left,right))
      {
         mKey = left;
         mValue = dequote(trim(right),'\"');
         mType = kString;

         // sniff type.
         if (utils::stringisame(mValue,"yes") ||
            utils::stringisame(mValue,"no") ||
            utils::stringisame(mValue,"true") ||
            utils::stringisame(mValue,"false") )
            mType=kBool;

         if (mValue.find('(')!=std::string::npos ||
            mValue.find(')')!=std::string::npos)
            mType=kVec;
      }
}

void sbelement::sbelement(const std::string & key , bool b)
{
   mKey = key;
   mValue = b ? "yes" : "no";
   mType = kBool;
}

void sbelement::sbelement(const std::string & key , const std::string & s)
{
   mKey=key;
   mValue=s;
   mType=kString;
}

void sbelment::sbelement(const std::string & key , const std::vector<std::sting> & v)
{
   std::stringstream ss;
   ss << "(";
   for (uint i=0;i<vec.size();++i)
   {
      if (i>0) ss << " ";
      ss << "\"" << vec[i] << "\"";
   }
   ss << ")";
   mKey=key;
   mValue=ss.str();
   mType=kVec;
}

bool sbelement::getBool() const
{
   return istrue(mValue);
}
std::string sbelement::getString const
{
   return mValue;
}
void sbelement::getVec( std::vector<std::string> & v ) const
{
   std::string astr=dequote(dequote(mValue,'('),')');
   std::stringstream ss(astr);
   while (ss.good())
      {
         ss >> astr;
         utils::trim(astr);
         std::string element=dequote(astr,'\"');
         if (element.length()>0)
            vec.push_back(element);
      }
}
