#include <fstream>
#include <ios>

#include "Poco/String.h"
#include "Poco/File.h"

#include "registry.h"
#include "drunner_paths.h"
#include "utils.h"
#include "dassert.h"
#include "sourcecopy.h"
#include "gitcache.h"

sourcecopy::registry::registry(registrydefinition r)
{
   gitcache gc(r.mURL);
   logmsg(kLDEBUG, "Reading registry.");
   Poco::Path p;
   cResult rslt = gc.get(p, false);
   if (!rslt.success())
      fatal(rslt.what());

   std::ifstream infile( p.toString() + "/registry");
   if (!infile.is_open())
      fatal("Couldn't open registry "+r.mNiceName+".");

   std::string line;
   while (std::getline(infile, line))
   {
      Poco::trimInPlace(line);
      if (line.length() > 0 && line[0] != '#')
      {
         registryitem ri;
         rslt +=loadline(line, ri);
         if (rslt.success())
            mRegistryItems.push_back(ri);
         else
            fatal(rslt.what());
      }
   }
}

cResult sourcecopy::registry::get(const std::string nicename, registryitem & item)
{
   for (unsigned int i = 0; i<mRegistryItems.size(); ++i)
      if (Poco::icompare(mRegistryItems[i].nicename, nicename) == 0)
      {
         item = mRegistryItems[i];
         return kRSuccess;
      }

   return cError(nicename + " does not exist in registry.");
}

// returns index of separating space.
int getchunk(std::string l)
{
   drunner_assert(l.length() > 0, "getchunk passed empty string.");
   drunner_assert(!iswspace(l[0]), "gitchunk given untrimmed string.");
   bool q = (l[0]=='"');
   for (unsigned int i = q ? 1 : 0; i < l.length(); ++i)
   {
      if (iswspace(l[i]) && !q)
         return i;
      if (l[i] == '"')
         q = false;
   }
   return l.length();
}

cResult sourcecopy::registry::loadline(const std::string line, registryitem & ri)
{
   // expect three whitespace separated strings.
   std::vector<std::string> chunks;
   std::string l(line);
   Poco::trimLeftInPlace(l);
   while (l.length() > 0)
   {
      int i = getchunk(l);
      drunner_assert(i > 0, "loadline : coding err");
      chunks.push_back(l.substr(0, i));
      l.erase(0, i + 1);
      Poco::trimLeftInPlace(l);
   }
   if (chunks.size()!=3)
      return cError("Registry lines must be of form: nicename GitURI description:\n"+line);

   ri.nicename = chunks[0];
   ri.url = chunks[1];
   ri.description = chunks[2];

   logmsg(kLDEBUG, "<" + ri.nicename + "><" + ri.url + "><" + ri.description + ">");

   return kRSuccess;
}

