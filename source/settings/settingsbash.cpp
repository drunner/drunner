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

std::string dequote(const std::string & s, char c)
{
   std::string ss(s);
   if (ss.length()==0) return ss;
   if (ss[0]==c) ss.erase(0,1);
   if (ss.length()==0) return ss;
   if (ss[ss.length()-1]==c) ss.erase(ss.length()-1,std::string::npos);
   return ss;
}

bashline sbelement::getBashLine() const
{
   return bashline("error","error");
}


sb_vec::sb_vec(const std::string & key ) : sbelement(key) //, const std::vector<std::string> & v)
{
}

sb_vec::sb_vec(const std::string & key , std::vector<std::string> v) : sbelement(key) //, const std::vector<std::string> & v)
{
   for (uint i=0;i<v.size();++i)
      mValue.push_back(v[i]);
}

bashline sb_vec::getBashLine() const
{
   std::stringstream ss;
   ss << "(";
   for (uint i=0;i<mValue.size();++i)
   {
      if (i>0) ss << " ";
      ss << "\"" << mValue[i] << "\"";
   }
   ss << ")";
   return bashline(mKey, ss.str());
}
sb_vec::sb_vec(const bashline & b) : sbelement(b.getkey())
{
   std::string astr=b.getvalue();
   astr=dequote(dequote(astr,'('),')');
   std::stringstream ss(astr);
   while (ss.good())
      {
         ss >> astr;
         utils::trim(astr);
         std::string element=dequote(astr,'\"');
         if (element.length()>0)
            mValue.push_back(element);
      }
}

sb_bool::sb_bool(const bashline & b) : sbelement(b.getkey()), mValue(istrue(b.getvalue()))
{
}
sb_bool::sb_bool(const std::string & key , bool b) : sbelement(key),mValue(b)
{
}


settingsbash::settingsbash(const params & par, std::string settingspath) : mPath(settingspath), p(par)
{
}

bashline::bashline(const std::string & bashline_str)
{
   std::size_t end=0;
   if ((end=bashline_str.find("=",0)) != std::string::npos)
      {
         key=utils::trim_copy(bashline_str.substr(0,end));
         value=utils::trim_copy(bashline_str.substr(end+1)); //If this is equal to the string length, substr returns an empty string.
         if (key.length()>0 && key[0]=='#')
            key.clear();
      }
}

bashline::bashline(const std::string & k, const std::string & v)
{
   key = k;
   value = v;
   value = dequote(utils::trim(value),'\"');
}


sbelement bashline::getElement()
{
   // sniff type.
   if (utils::stringisame(value,"yes") ||
      utils::stringisame(value,"no") ||
      utils::stringisame(value,"true") ||
      utils::stringisame(value,"false") )
      return sb_bool(*this);

   if (value.length()>0 && value[0]=='(')
      return sb_vec(*this);

   return sb_string(*this);
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
      setSetting(sbe);
   }
   configfile.close();

   return true;
}

void settingsbash::setSetting(const sbelement & value)
{
   for (uint i=0;i<mElements.size();++i)
      if (utils::stringisame(mElements[i].getKey(),value.getKey()))
         mElements[i] = value;

   mElements.push_back(value);
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
     ofile << entry.getBashLine().str() << std::endl;
  }
  ofile.close();
  return true;
}

bool sb_bool::istrue(const std::string & s) const
{
   if (s.length()==0) return false;
   return (tolower(s[0])=='y' || tolower(s[0])=='t');
}

sbelement settingsbash::getElement(const std::string & key) const
{
   for (uint i=0;i<mElements.size();++i)
      if (utils::stringisame(mElements[i].getKey(),key))
         return mElements[i];
   logmsg(kLERROR,"Couldn't find key "+key,p);
   return sb_string("ERROR","ERROR");
}

std::string settingsbash::getString(const std::string &  key) const
{
   return getElement(key).getString();
}
bool settingsbash::getBool(const std::string &  key) const
{
   return getElement(key).getBool();
}
const std::vector<std::string> & settingsbash::getVec(const std::string &  key) const
{
   return getElement(key).getVec();
}


const std::vector<std::string> & sbelement::getVec() const
{ // needs to be compatible with bash scripts!!
   const sb_vec * b = dynamic_cast<const sb_vec *>(this);
   if (!b) logmsg(kLERROR,"Couldn't interpret "+mKey+" as string array.",kLERROR);
   return b->get();
}

bool sbelement::getBool() const
{
   const sb_bool * b = dynamic_cast<const sb_bool *>(this);
   if (!b) logmsg(kLERROR,"Couldn't interpret "+mKey+" as bool.",kLERROR);
   return b->get();
}

std::string sbelement::getString() const
{
   const sb_string * b = dynamic_cast<const sb_string *>(this);
   if (!b) logmsg(kLERROR,"Couldn't interpret "+mKey+" as string.",kLERROR);
   return b->get();
}

std::string settingsbash::getPath() const
{
   return mPath;
}
