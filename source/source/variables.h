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

// If configtypes are added then to_configtype needs to be updated.
enum configtype
{
   kCF_port,
   kCF_path,
   kCF_existingpath,
   kCF_string,
   kCF_bool,
   kCF_URL,
   kCF_password,
};

configtype to_configtype(std::string s);

class Configuration
{
public:
   Configuration(std::string n, std::string dflt, std::string desc, configtype t, bool rqd, bool user) : name(n), defaultval(dflt), description(desc), type(t), required(rqd), usersettable(user) {}
   
   std::string name;
   std::string defaultval;
   std::string description;
   configtype type;
   bool required;
   bool usersettable;
};


class variables {
public:
   variables() {}
   //variables(const variables & other);
   variables(const variables & other1, const variables & other2);

   bool hasKey(std::string key) const;
   bool isDefined(std::string key) const; // hasKey and is not empty string.
   std::string getVal(std::string key) const;
   bool getBool(std::string key) const;
   void setVal(std::string key, std::string val);
   void delKey(std::string key);

   std::string substitute(std::string s) const;
   const tKeyVals & getAll() const { return mVariables; }

private:
   tKeyVals mVariables;

   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mVariables); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { mVariables.clear(); ar(mVariables); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(variables, 1);


class persistvariables {
public:
   persistvariables(std::string name, Poco::Path path, const std::vector<Configuration> config);
   cResult loadvariables();
   cResult savevariables() const;
   cResult checkRequired() const;
   cResult setVal(std::string key, std::string val);
   void setVal_mem(std::string key, std::string val);

   // expose other methods.
   bool hasKey(std::string key) const;
   bool isDefined(std::string key) const; // hasKey and is not empty string.
   std::string getVal(std::string key) const;
   bool getBool(std::string key) const;
   std::string substitute(std::string s) const;
   const tKeyVals getAll() const;
   const variables getVariables() const;
   bool exists() const;
   
   cResult handleConfigureCommand(CommandLine cl);


protected:
   cResult _showconfiginfo() const;
   cResult _checkvalid(std::string key, std::string val, Configuration config);
   void _addConfig(const Configuration & c);

   std::string mName; // name of service (or whatever) to print nice messages.
   Poco::Path mPath;
   std::vector<Configuration> mConfig;

   variables mVariables;
   variables mVariables_Mem;
};

#endif