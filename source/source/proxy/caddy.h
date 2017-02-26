#ifndef __CADDY_H
#define __CADDY_H

#include "proxy.h"

class caddy : public proxyplugin
{
public:
   caddy(const proxydata & pd) : proxyplugin(pd) {}

   virtual cResult restart();

   static std::string dataVolume() { return "drunner-proxy-dataVolume"; }
   static std::string rootVolume() { return "drunner-proxy-rootVolume"; }
   static std::string containerName() { return "drunner-proxy"; }

private:
   cResult generate();
};


#endif

