#ifndef __SERVICE_VARIABLES_H
#define __SERVICE_VARIABLES_H

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include <Poco/Process.h>

typedef std::map<std::string, std::string> tKeyVals;

class variables {
public:
   variables() {}
   variables(const variables & other);
   variables(const variables & other1, const variables & other2);

   bool hasKey(std::string key) const;
   std::string getVal(std::string key) const;
   void setVal(std::string key, std::string val);

   std::string substitute(std::string s) const;
   const Poco::Process::Env & getEnv() const;

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


#endif