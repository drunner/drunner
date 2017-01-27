#ifndef __SERVICE_VARIABLES_H
#define __SERVICE_VARIABLES_H

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include <Poco/Process.h>
#include <Poco/Path.h>

#include "cresult.h"
#include "utils.h"

typedef std::map<std::string, std::string> tKeyVals;

class keyVals {
public:
   keyVals() {}
   keyVals(const keyVals & other1, const keyVals & other2);

   bool hasKey(std::string key) const;
   bool isDefined(std::string key) const; // hasKey and is not empty string.
   std::string getVal(std::string key) const;
   bool getBool(std::string key) const;
   void setVal(std::string key, std::string val);
   void delKey(std::string key);

   std::string substitute(std::string s) const;
   const tKeyVals & getAll() const { return mKeyVals; }

private:
   tKeyVals mKeyVals;

   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mKeyVals); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { mKeyVals.clear(); ar(mKeyVals); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(keyVals, 1);



const unsigned int ENV_DEFAULTS_MEM = 0x0000; // not user settable, memory only, not secret.
const unsigned int ENV_USERSETTABLE = 0x0001;
const unsigned int ENV_PERSISTS = 0x0002;
const unsigned int ENV_SECRET = 0x0004;


class envFlags {
public:
   envFlags(unsigned int s) : state(s) {}
   unsigned int state;
};


class envDef
{
public:
   envDef(std::string n, std::string dflt, std::string desc, envFlags flags) : name(n), defaultval(dflt), description(desc)
   {
      usersettable = ((flags.state & ENV_USERSETTABLE) != 0);
      persists = ((flags.state & ENV_PERSISTS) != 0);
      secret = ((flags.state & ENV_SECRET) != 0);
   }

   std::string name;
   std::string defaultval;
   std::string description;
   bool usersettable;
   bool persists;
   bool secret;
};


class persistvariables : public keyVals {
public:
   persistvariables(std::string name, Poco::Path path, const std::vector<envDef> config);
   cResult loadvariables();
   cResult savevariables() const;

   bool exists() const;

   // override base class, check it exists in the environment definitions before commiting.
   cResult setVal(std::string key, std::string val);
   
   cResult handleConfigureCommand(CommandLine cl);

protected:
   cResult _showconfiginfo() const;
   void _addConfig(const envDef & c);

   std::unique_ptr<envDef> getDef(std::string key) const;

   std::string mName; // name of service (or whatever) to print nice messages.
   Poco::Path mPath;
   std::vector<envDef> mEnvDefs;
};

#endif