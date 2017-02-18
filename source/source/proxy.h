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
   proxydatum(std::string s, std::string d, std::string c, std::string p, std::string n, std::string e, std::string m, bool t) :
      servicename(s), domain(d), container(c), port(p), network(n), email(e), mode(m), timeouts(t)
   {}

   proxydatum()
   {}

   cResult valid();

   bool operator ==(const proxydatum &b) const;

   std::string servicename;
   std::string domain;
   std::string container;
   std::string port;
   std::string network;
   std::string email;
   std::string mode;
   bool timeouts;

private:
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const 
   { 
      ar(servicename, domain, container, port, network, email, mode, timeouts);
   }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) 
   {
      ar(servicename, domain, container, port, network, email, mode, timeouts);
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


class proxy
{
public:
   proxy();

   cResult proxyenable(proxydatum pd);
   cResult proxydisable(std::string service);
   cResult proxyregen();

private:
   cResult proxyconfigchanged();
   cResult generate();
   cResult restart();

   cResult load();
   cResult save();
   Poco::Path saveFilePath();
   std::string dataVolume() { return "drunner-proxy-dataVolume"; }
   std::string rootVolume() { return "drunner-proxy-rootVolume"; }
   std::string containerName() { return "drunner-proxy"; }
   proxydata mData;
};


#endif
