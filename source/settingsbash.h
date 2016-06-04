#ifndef __SETTINGSBASH_H
#define __SETTINGSBASH_H

#include <map>
#include <memory>

#include "params.h"


class bashline
{
public:
   bashline();

   void setline(const std::string & bashline_str);
   void setkeyvalue(const std::string &k, const std::string &v); // raw value - includes quotes etc exactly as written to file.

   std::string getline() const { return key + "=" + value; }

   bool valid() { return key.length() > 0; }

   const std::string & getkey() const { return key; }
   const std::string & getvalue() const { return value; }
   bashline(const bashline & other) { key = other.key; value = other.value; }

protected:
   std::string key, value;
};

class settingsbash
{
public:
   settingsbash(bool createonread) : mCreateOnRead(createonread) {}
   
   bool readSettings(const std::string & settingspath);
   bool writeSettings(const std::string & settingspath) const;

   bool getBool(const std::string & key) const;
   std::string getString(const std::string & key) const;
   void getVec(const std::string & key, std::vector<std::string> & vec) const;

   void setBool(const std::string & key, bool b);
   void setString(const std::string & key, const std::string & s);
   void setVec(const std::string & key, const std::vector<std::string> & v);

protected:
   const bool mCreateOnRead;
   void setSetting(bashline bl, bool createOK);
   bashline getElement(const std::string & key) const;
   const params & getParams();

   std::vector<bashline> mElements;

private:
   bool istrue(const std::string & s) const;
};

#endif
