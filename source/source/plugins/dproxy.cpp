#include "dproxy.h"
#include "drunner_paths.h"
#include "generate.h"
#include "enums.h"
#include "service.h"
#include "dassert.h"
#include "utils_docker.h"

static std::string s_dproxycontainer = "drunner_dproxy";

dproxy::dproxy() : configuredplugin("dproxy")
{
}

std::string dproxy::getName() const
{
   return "dproxy";
}

cResult dproxy::runCommand(const CommandLine & cl, const variables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
   case (s2i("update")):
   {
      cResult r = update();
      if (r == kRNoChange)
         logmsg(kLINFO, "dproxy configuration unchanged.");
      else
         logmsg(kLINFO, "dproxy configuration updated.");
      return r;
   }
   case (s2i("start")):
   {
      return start();
   }
   case (s2i("stop")):
   {
      return stop();
   }
   default:
      return cError("Unrecognised command " + cl.command);
   }
}

cResult dproxy::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   if (lf!=NULL && lf->getProxies().size()==0) // no proxy, so changes to this dService don't matter for us.
      return kRNoChange;

   switch (s2i(hook.c_str()))
   {
   case (s2i("configure_end")):
      if (hookparams.size() == 0)
         return kRNoChange;
      // fall through
   case (s2i("install_end")):
   case (s2i("update_end")):
   case (s2i("uninstall_end")):
   case (s2i("obliterate_end")):
   {
      logmsg(kLINFO, "Reconfiguring dproxy.");
      return update();
   }

   default:
      return kRNoChange;
   }

   return kRNotImplemented;
}


cResult dproxy::showHelp() const
{
   std::string help = R"EOF(
NAME
   dproxy

DESCRIPTION
   A dRunner plugin which makes it easy to map virtual hosts to services.

COMMANDS
   dproxy start
   dproxy stop
   dproxy update
)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

Poco::Path dproxy::configurationFilePath() const
{
   return drunnerPaths::getPath_Root().setFileName("dproxy.json");
}

Poco::Path dproxy::haproxyCfgPath()
{
   Poco::Path target = drunnerPaths::getPath_Root().setFileName("dproxy_haproxy.cfg");
   return target;
}

cResult dproxy::update() const
{   
   std::string vdata, http_in, https_in, http_backends, https_backends;
   bool http = false, https = false;

   // loop over all dServices.
   std::vector<std::string> services;
   utils::getAllServices(services);
   for (auto x : services)
   {
      // check if service x needs autoproxy
      service s(x);
      std::vector<servicelua::Proxy> pl = s.getServiceLua().getProxies();
      if (pl.size() > 0)
      { // handle the proxy list.
         for (auto p : pl)
         {
            std::string vhost = s.getServiceVars().substitute(p.vhost);
            std::string dport_http = s.getServiceVars().substitute(p.dport_http);
            std::string dport_https = s.getServiceVars().substitute(p.dport_https);

            drunner_assert(vhost.size() > 0, "Virtual host not specified.");
            if (dport_http.length() > 0)
            {
               http_in += "    acl is_" + x + " hdr_end(host) -i " + vhost + "\n";
               http_in += "    use_backend " + x + "_http if is_" + x + "\n";
               http_backends += "backend " + x + "_http\n";
               http_backends += "    server " + x + +" " + vhost + ":" + dport_http + "\n";
               http_backends += "    mode http\n";

               http = true;
            }
            if (dport_https.length() > 0)
            {
               https_in += "    acl is_" + x + " hdr_end(host) -i " + vhost + "\n";
               https_in += "    use_backend " + x + "_https if is_" + x + "\n";
               https_backends += "backend " + x + "_https\n";
               https_backends += "    server " + x + " "+vhost+":" + dport_https + "\n";
               https_backends += "    mode tcp\n";

               https = true;
            }
            //int port = p.dport_http;
         }
      }
   }


   vdata = R"EOF(
defaults
    retries 3
    timeout connect 5s
    timeout client 1m
    timeout server 1m
)EOF";

   if (http)
   {
      vdata += R"EOF(

frontend http-in
    bind *:80
    mode http
    option http-server-close
    option forwardfor

)EOF";
      vdata += http_in;
   }

   if (https)
   {
      vdata += R"EOF( 

frontend https-in
    bind *:443
    mode tcp

)EOF";

      vdata += https_in;
   }

   if (http)
   {
      vdata += "\n\n";
      vdata += http_backends;
   }

   if (https)
   {
      vdata += "\n\n";
      vdata += https_backends;
   }

   cResult r = generate(haproxyCfgPath(), S_755, vdata);
   return r;
}

cResult dproxy::start() const
{
   CommandLine c;
   std::string out;

   if (utils_docker::dockerContainerExists(s_dproxycontainer))
   {
      logdbg("dproxy is already running.");
      return kRNoChange;
   }

   c.command = "docker";
   c.args = { "run","-d","--name",s_dproxycontainer,"-p","80:80","-p","443:443",
      "-v",haproxyCfgPath().toString() + ":/usr/local/etc/haproxy/haproxy.cfg:ro","haproxy:alpine" };
   int r = utils::runcommand(c, out);

   logmsg(kLDEBUG, out);

   if (r != 0)
      return cError(out);

   return kRSuccess;
}

cResult dproxy::stop() const
{
   CommandLine c;
   std::string out;
   c.command = "docker";

   if (!utils_docker::dockerContainerExists(s_dproxycontainer))
   {
      logdbg("dproxy is not running.");
      return kRNoChange;
   }

   c.args = { "stop",s_dproxycontainer };
   utils::runcommand(c, out);

   c.args = { "rm",s_dproxycontainer };
   int r = utils::runcommand(c, out);
   if (r != 0) return cError(out);
   logmsg(kLDEBUG, out);


   return cResult();
}
