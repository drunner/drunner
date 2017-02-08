#include "Poco/DirectoryIterator.h"
#include "Poco/String.h"

#include "gitcache.h"
#include "utils.h"

gitcache::gitcache(std::string url, std::string tag)
{
}

cResult gitcache::get(Poco::Path & p, bool forceUpdate) const
{
   return cResult();
}

cResult recursiveCopy(Poco::Path src, Poco::Path dest)
{
   try
   {
      Poco::DirectoryIterator end;
      for (Poco::DirectoryIterator it(src); it != end; ++it)
      {
         if (it->isDirectory())
         {
            if (Poco::icompare(it->path(), ".git") != 0)
            {
               Poco::Path subdest(dest);
               subdest.pushDirectory(it->path());
               utils::makedirectory(subdest, S_700);
               Poco::Path subsrc(src);
               subsrc.pushDirectory(it->path());
               recursiveCopy(subsrc, subdest);
            }
            else
               logmsg(kLDEBUG, "Skipping .git directory tree");
         }
         else
         {
            it->copyTo(dest.toString());
         }
      }
   }
   catch (Poco::Exception & e)
   {
      fatal(e.what());
   }
}

cResult gitcache::copyTo(Poco::Path dest, bool forceUpdate) const
{
   Poco::Path p;
   cResult r = get(p, forceUpdate);
   if (!r.success())
      return r;



   return cResult();
}

std::string gitcache::hash(std::string url)
{
   return std::string();
}


cResult gitcopy(std::string repoURL, std::string tag, Poco::Path dest)
{
   // checkout repo, copy subfolder if present.
   // git clone --progress -b master --depth 1 https://github.com/drunner/d10_rocketchat
   if (!utils::fileexists(dest))
      return cError("gitcopy: Destination does not exist: " + dest.toString());
   CommandLine op;
   op.command = "git";
   op.args = { "clone","--progress","-b",tag.length() > 0 ? tag : "master",
      "--depth","1",repoURL,"." };
   cResult r = utils::runcommand_stream(op, kORaw, dest, tKeyVals(), NULL);

   return r;
}