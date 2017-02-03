#include <fstream>
#include <ios>

#include "Poco/String.h"
#include "Poco/File.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/URI.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"

#include "registry.h"
#include "drunner_paths.h"
#include "utils.h"
#include "dassert.h"

cResult downloadfile(std::string url, Poco::Path dest)
{
   logmsg(kLINFO, "Downloading " + url);
   try
   {
      Poco::Net::HTTPStreamFactory::registerFactory(); // Must register the HTTP factory to stream using HTTP
      if (utils::fileexists(dest))
         Poco::File(dest).remove();

      std::ofstream fileStream;
      fileStream.open(dest.toString(), std::ios::out | std::ios::trunc | std::ios::binary);
      Poco::URI uri(url);
      std::auto_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));
      Poco::StreamCopier::copyStream(*pStr.get(), fileStream);
      fileStream.close();
   }

   catch (Poco::Exception & e)
   {
      return cError(e.what());
   }
   return kRSuccess;
}

sourceplugins::registry::registry(std::string fullurl)
{
   Poco::Path f = drunnerPaths::getPath_Temp().setFileName("registry.tmp");
   cResult r = downloadfile(fullurl, f);
   if (!r.success())
      fatal(r.what());

   std::ifstream infile(f.toString());
   if (!infile.is_open())
      fatal("Couldn't open registry "+fullurl+".");

   std::string line;
   while (std::getline(infile, line))
   {
      Poco::trimInPlace(line);
      if (line.length() > 0 && line[0] != '#')
      {
         registryitem ri;
         cResult r = loadline(line, ri).success();
         if (r.success())
            mRegistryItems.push_back(ri);
         else
            fatal(r.what());
      }
   }
}

cResult sourceplugins::registry::get(const std::string nicename, registryitem & item)
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
   bool q = false;
   for (int i = 0; i < l.length(); ++i)
   {
      if (iswspace(l[i]) && !q)
         return i;
      if (l[i] == '"' && q)
         q = false;
   }
   return l.length();
}

cResult sourceplugins::registry::loadline(const std::string line, registryitem & ri)
{
   // expect three whitespace separated strings.
   std::vector<std::string> chunks;
   std::string l(line);
   while (l.length() > 0)
   {
      int i = getchunk(l);
      drunner_assert(i > 0, "loadline : coding err");
      chunks.push_back(l.substr(0, i));
      l.erase(0, i + 1);
   }
   if (chunks.size()!=4)
      return cError("Registry lines must be of form: nicename protocol URL description:\n"+line);

   ri.nicename = chunks[0];
   ri.protocol = ProtoParse(chunks[1]);
   ri.url = chunks[2];
   ri.description = chunks[3];

   if (ri.protocol == kP_ERROR)
      return cError("Unknown protocol " + chunks[1] + " in:\n" + line);

   return kRSuccess;
}
