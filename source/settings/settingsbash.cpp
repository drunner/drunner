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

// ----------------------------------------------------------------------------------------------------------------------

bashline::bashline(const std::string & bashline_str)
{
   std::size_t end=0;
   if ((end=bashline_str.find("=",0)) != std::string::npos)
      {
         key=utils::trim_copy(bashline_str.substr(0,end));
         value=utils::trim_copy(bashline_str.substr(end+1)); //If this is equal to the string length, substr returns an empty string.
         value = dequote(utils::trim(value), '\"');
         if (key.length()>0 && key[0]=='#')
            key.clear();
   }
}

bashline::bashline(const std::string & k, const std::string & v)
{
   key = k;
   value = v;
}


std::shared_ptr<sbelement> bashline::getElement()
{
   // sniff type.
   if (utils::stringisame(value,"yes") ||
      utils::stringisame(value,"no") ||
      utils::stringisame(value,"true") ||
      utils::stringisame(value,"false") )
      return std::make_shared<sb_bool>( sb_bool(*this) );

   if (value.length()>0 && value[0]=='(')
      return std::make_shared<sb_vec>( sb_vec(*this) );

   return std::make_shared<sb_string>( sb_string(*this) );
}

// ----------------------------------------------------------------------------------------------------------------------

bool settingsbash::readSettings(const std::string & settingspath)
{
   if (settingspath.length() == 0)
      fatal("settingsbash::readSettings - empty string passed as path to read from.");

   if (! utils::fileexists(settingspath))
      return false;

   std::string line,left,right;
   std::ifstream configfile(settingspath.c_str() );
   while (std::getline(configfile, line))
   {
      bashline bl(line);
	  if (bl.valid())
	      setSetting( bl.getElement() , false );
   }
   configfile.close();

   return true;
}

void settingsbash::setSetting(std::shared_ptr<sbelement> value, bool createOK)
{
   auto newelement = value->clone();

   for (uint i=0;i<mElements.size();++i)
      if (utils::stringisame(mElements[i]->getKey(),newelement->getKey()))
      {
         mElements[i] = newelement;
         return;
      }
   if (!createOK)
      fatal("Trying to create setting " + newelement->getKey() + " but not permitted.");
   mElements.push_back(newelement);
}


bool settingsbash::writeSettings(const std::string & settingspath) const
{
   if (settingspath.length() == 0)
      fatal("settingsbash::writeSettings - empty string passed as path to write to.");

   if (settingspath.find(".sh")==std::string::npos)
      fatal("All bash style settings files should have .sh file extension. Offender: "+ settingspath);

   if (utils::fileexists(settingspath))
      std::remove(settingspath.c_str());

   std::ofstream ofile(settingspath.c_str() );
   if (!ofile.is_open()) return false; // can't open the file.

   ofile << "# "+ settingspath << std::endl;
   ofile << "# dRunner bash configuration file." << std::endl;

  // iterate through map. C++11 style.
  for (auto const &entry : mElements) {
     ofile << entry->getBashLine().str() << std::endl;
  }
  ofile.close();
  return true;
}

bool sb_bool::istrue(const std::string & s) const
{
   if (s.length()==0) return false;
   return (tolower(s[0])=='y' || tolower(s[0])=='t');
}

std::shared_ptr<const sbelement> settingsbash::getElement(const std::string & key) const
{
	for (auto const &entry : mElements)
		if (utils::stringisame(entry->getKey(), key))
			return entry;

   fatal("Couldn't find key "+key);
   return std::make_shared<sb_string>(sb_string("ERROR","ERROR"));
}

const std::vector<std::string> & settingsbash::getVec(const std::string &  key) const
{
   const auto b = dynamic_cast<const sb_vec *>(getElement(key).get());
   if (!b) fatal("Couldn't interpret "+key+" as string array.");
   return b->get();
}
bool settingsbash::getBool(const std::string &  key) const
{
   const auto b = dynamic_cast<const sb_bool *>(getElement(key).get());
   if (!b) fatal("Couldn't interpret "+key+" as string array.");
   return b->get();
}
const std::string & settingsbash::getString(const std::string &  key) const
{
   const auto b = dynamic_cast<const sb_string *>(getElement(key).get());
   if (!b) fatal("Couldn't interpret "+key+" as string.");
   return b->get();
}

void settingsbash::setBool(const std::string & key, bool b)
{
	setSetting(std::make_shared<sb_bool>(sb_bool(key, b)), true);
}
void settingsbash::setString(const std::string & key, const std::string & s )
{
	setSetting(std::make_shared<sb_string>(sb_string(key, s)), true);
}
void settingsbash::setVec(const std::string & key, const std::vector<std::string> & v)
{
	setSetting(std::make_shared<sb_vec>(sb_vec(key, v)), true);
}

//-------------------------------------------------------------------------------------------------

settingsbash_reader::settingsbash_reader(std::string settingspath) :
   mPath(settingspath), mReadOkay(false)
{
}
bool settingsbash_reader::read()
{
   mReadOkay = false;
   if (mPath.length()>0)
      mReadOkay = readSettings(mPath);
   return mReadOkay;
}
const std::string & settingsbash_reader::getPath() const 
{ 
   return mPath; 
}
bool settingsbash_reader::readOkay() const
{ 
   return mReadOkay; 
}

