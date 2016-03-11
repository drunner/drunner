#ifndef __SETTINGSBASH_H
#define __SETTINGSBASH_H

#include <map>
#include <memory>

#include "params.h"


class bashline;

class sbelement
{
   public:
//      sbelement(const sbelement & other) : mKey(other.mKey), mValue(other.mValue), mType(other.mType) {}
      sbelement(const std::string & key) {mKey=key;}
      virtual ~sbelement() {}
      virtual bashline getBashLine() const;
      virtual std::shared_ptr<sbelement> clone() const =0;

      bool getBool() const;
      std::string getString() const;
      const std::vector<std::string> & getVec() const;
      const std::string & getKey() const {return mKey;}

   private:
      sbelement();

   protected:
      sbelement(const sbelement & other) {mKey=other.mKey;}
      std::string mKey;
};

class bashline
{
public:
   bashline(const std::string & k, const std::string & v);
   bashline(const std::string & bashline_str);
   std::string str() {return key+"="+value;}

   bool valid() {return key.length()>0;}

   std::shared_ptr<sbelement> getElement();

   const std::string & getkey() const {return key;}
   const std::string & getvalue() const {return value;}

private:
   bashline();
   std::string key,value;
};

class sb_bool : public sbelement
{
   public:
      sb_bool(const bashline & b);
      sb_bool(const std::string & key , bool b);
      bool get() const {return mValue;}
      bashline getBashLine() const {return bashline(mKey,mValue?"yes":"no");}
      std::shared_ptr<sbelement> clone() const override { return std::make_shared<sb_bool>(*this); }
   private:
      bool mValue;
      bool istrue(const std::string & s) const;
};

class sb_string : public sbelement
{
   public:
      sb_string(const bashline & b) : sbelement(b.getkey()), mValue(b.getvalue()) {}
      sb_string(const std::string & key, const std::string & s) : sbelement(key),mValue(s) {}
      const std::string & get() const {return mValue;}
      bashline getBashLine() const {return bashline(mKey,"\""+mValue+"\"");}
      std::shared_ptr<sbelement> clone() const override { return std::make_shared<sb_string>(*this); }
   private:
      std::string mValue;
};

class sb_vec : public sbelement
{
   public:
      sb_vec(const bashline & b);
      sb_vec(const std::string & key );
      sb_vec(const std::string & key , std::vector<std::string> v);
      const std::vector<std::string> & get() const {return mValue;}
      bashline getBashLine() const;
      std::shared_ptr<sbelement> clone() const override { return std::make_shared<sb_vec>(*this); }
   private:
      std::vector<std::string> mValue;
};


class settingsbash
{
public:
   settingsbash(const params & par, std::string settingspath); // sets defaults, reads from config.sh if present.

   bool readSettings();
   bool writeSettings(const std::string & fullpath="") const;

   bool getBool(const std::string & key) const;
   std::string getString(const std::string & key) const;
   const std::vector<std::string> & getVec(const std::string &  key) const;

   void setBool(const std::string & key, bool b);
   void setString(const std::string & key, const std::string & s );
   void setVec(const std::string & key, const std::vector<std::string> & v);

   void setSetting(std::shared_ptr<sbelement> value);

   std::string getPath() const;
private:
   settingsbash();
   const std::string mPath;
//   std::map< std::string, std::string > mSettings;
//   void checkkeyexists(const std::string & key) const;
   std::shared_ptr<const sbelement> getElement(const std::string & key) const;

   const params & p;
   std::vector<std::shared_ptr<sbelement>> mElements;

protected:
   void logmsg(eLogLevel level, std::string s) const;
};

#endif
