#include "Poco/String.h"

#include "sourceplugins.h"
#include "registries.h"
#include "localdir.h"
#include "dockercontainer.h"
#include "globalcontext.h"

namespace sourceplugins
{
   typedef std::shared_ptr<sourceplugin> pluginptr;
   typedef std::vector < pluginptr > pluginvec;

   void loadplugins(pluginvec & plugins)
   {
      plugins.push_back(pluginptr(new localdir()));
      plugins.push_back(pluginptr(new registries()));
      plugins.push_back(pluginptr(new dockercontainer()));
   }

   pluginptr getPlugin(std::string imagename)
   {
      pluginvec plugins;
      loadplugins(plugins);

      for (unsigned int i = 0; i < plugins.size(); ++i)
         if (plugins[i]->pluginmatch(imagename))
            return plugins[i];
      return NULL;
  }

   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string imagename, const servicePaths & sp)
   {
      pluginptr p = getPlugin(imagename);

      if (!p)
         return cError(imagename + " doesn't match any known source type.");

      cResult r = p->install(imagename, sp);
      return r;
   }

   cResult normaliseNames(std::string & imagename, std::string & servicename)
   {
      pluginptr p = getPlugin(imagename);

      return p->normaliseNames(imagename, servicename);
   }

   cResult registrycommand()
   {
      const params & p(*GlobalContext::getParams());

      registries r;
      if (p.numArgs()<1)
         return r.showregistries();

      if (Poco::icompare(p.getArg(0), "add")==0)
      {
         if (p.numArgs() != 3)
            fatal("drunner registry add NICENAME GITURL");
         else
            return r.addregistry(p.getArg(1), p.getArg(2));
      }

      if (Poco::icompare(p.getArg(0), "del") == 0)
      {
         if (p.numArgs() != 2)
            fatal("drunner registry del NICENAME");
         else
            return r.delregistry(p.getArg(1));
      }

      return cError("Unknown registry command: " + p.getArg(0));
   }


} // namespace 