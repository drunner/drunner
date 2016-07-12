#ifndef __SERVICE_VARIABLES_H
#define __SERVICE_VARIABLES_H

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

class keyval {
public:
   keyval() {}
   keyval(std::string k, std::string v) : key(k), value(v) {}

   std::string key;
   std::string value;

   // --- serialisation --
private:
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(key, value); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(key, value); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(keyval, 1);


class variables {
public:
   variables() {}
   variables(const variables & other);
   variables(const variables & other1, const variables & other2);

   bool hasKey(std::string key) const;
   std::string getVal(std::string key) const;
   void addkey(keyval kv);

protected:
   const std::vector<keyval> & getAll() const { return mVariables; }

private:
   std::vector<keyval> mVariables;

   // --- serialisation --
private:
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mVariables); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mVariables); }
   // --- serialisation --

};
CEREAL_CLASS_VERSION(variables, 1);


#endif