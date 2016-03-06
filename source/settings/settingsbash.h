#include <map>

#include "params.h"

#ifndef __SETTINGSBASH_H
#define __SETTINGSBASH_H

enum sbtype
{
   kBool,
   kString,
   kVec
};

class sbelement
{
   public:
      sbelement() {}
      sbelement(const std::string & bashline);
      sbelement(const std::string & key , bool b);
      sbelement(const std::string & key , const std::string & s);
      sbelement(const std::string & key , const std::vector<std::string> & v);

      std::string getBashLine() const;// {return mKey+"="+mValue;}

      bool getBool() const;
      std::string getString() const;
      void getVec( std::vector<std::string> & v ) const;
      const std::string & getKey() const {return mKey;}

   private:
      bool istrue(const std::string & s) const;
      bool parse(std::string line, std::string & left, std::string & right) const;
      std::string dequote(const std::string & s, char c) const;

      std::string mKey;
      std::string mValue;
      sbtype mType;
};

class settingsbash
{
public:
   settingsbash(const params & par, std::string settingspath); // sets defaults, reads from config.sh if present.

   bool readSettings();
   bool writeSettings(const std::string & fullpath="") const;

   bool getBool(const std::string & key) const;
   std::string getString(const std::string & key) const;
   void getVec( const std::string & key, std::vector<std::string> & v ) const;

   void setSetting(const sbelement & value);

   std::string getPath() const;
private:
   const std::string mPath;
//   std::map< std::string, std::string > mSettings;
//   void checkkeyexists(const std::string & key) const;
   const sbelement & getElement(const std::string & key) const;

   const params & p;
   std::vector<sbelement> mElements;
};

#endif
