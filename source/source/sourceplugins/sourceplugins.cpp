#include "sourceplugins.h"
#include "registries.h"
#include "localdir.h"
#include "dockercontainer.h"


namespace sourceplugins
{


   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string & imagename, const servicePaths & sp)
   {
      std::vector< std::unique_ptr<sourceplugin> > plugins;
      plugins.push_back(std::unique_ptr<sourceplugin>(new localdir()));
      plugins.push_back(std::unique_ptr<sourceplugin>(new registries()));
      plugins.push_back(std::unique_ptr<sourceplugin>(new dockercontainer()));

      for (auto p = plugins.begin(); p != plugins.end(); ++p)
      {
         cResult r = p->get()->install(imagename, sp);
         if (!r.noChange())
            return r;
      }

      logmsg(kLWARN, imagename + " wasn't found in any known source.");
      return cError("Failed to install " + imagename);
   }


} // namespace 