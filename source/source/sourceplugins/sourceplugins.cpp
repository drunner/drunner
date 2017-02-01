#include "sourceplugins.h"
#include "registries.h"
#include "localdir.h"
#include "dockercontainer.h"


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

      for (int i = 0; i < plugins.size(); ++i)
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


} // namespace 