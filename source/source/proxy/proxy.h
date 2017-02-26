#ifndef __PROXY_H
#define __PROXY_H

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include "Poco/Path.h"

#include "cresult.h"

// ---------------------------------------------------------------------------------------

class proxydatum
{
public:
   proxydatum(std::string s, std::string d, std::string c, std::string p, std::string e, std::string m, bool t) :
      servicename(s), domain(d), container(c), port(p), email(e), mode(m), timeouts(t)
   {}

   proxydatum()
   {}

   cResult valid();

   bool operator ==(const proxydatum &b) const;

   std::string servicename;
   std::string domain;
   std::string container;
   std::string port;
   std::string email;
   std::string mode;
   bool timeouts;

private:
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const 
   { 
      ar(servicename, domain, container, port, email, mode, timeouts);
   }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) 
   {
      ar(servicename, domain, container, port, email, mode, timeouts);
   }
};
CEREAL_CLASS_VERSION(proxydatum, 1);

// ---------------------------------------------------------------------------------------

class proxydata
{
public:
   std::vector<proxydatum> mProxyData;

private:
// --- serialisation --
friend class cereal::access;
template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mProxyData); }
template <class Archive> void load(Archive &ar, std::uint32_t const version) { mProxyData.clear(); ar(mProxyData); }
// --- serialisation --
};
CEREAL_CLASS_VERSION(proxydata, 1);

// ---------------------------------------------------------------------------------------

class proxyplugin
{
public:
   proxyplugin(const proxydata & pd) : mProxyData(pd) {}

   virtual cResult restart() = 0;

protected:
   const proxydata & mProxyData;
};


class proxy
{
public:
   proxy();

   cResult proxyenable(proxydatum pd);
   cResult proxydisable(std::string service);
   cResult handledrunnerproxycommand();

   static std::string networkName() { return "drunnerproxy"; }

private:
   cResult proxyconfigchanged();

   std::unique_ptr<proxyplugin> getPlugin();
   cResult load();
   cResult save();
   Poco::Path saveFilePath();

   proxydata mData;
};


#endif
