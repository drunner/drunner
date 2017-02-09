#include "Poco/DirectoryIterator.h"
#include "Poco/String.h"
#include "Poco/DigestStream.h"
#include "Poco/MD5Engine.h"

#include "gitcache.h"
#include "utils.h"
#include "drunner_paths.h"
#include "dassert.h"

gitcache::gitcache(std::string url, std::string tag) : mURL(url), mTag(tag)
{
   if (mTag.length() == 0)
      mTag = "master";
}

cResult gitcache::get(Poco::Path & p, bool forceUpdate) const
{
   // checkout repo, copy subfolder if present.
   // git clone --progress -b master --depth 1 https://github.com/drunner/d10_rocketchat
   Poco::Path dest = drunnerPaths::getPath_GitCache();
   dest.pushDirectory(hash(mURL));
   p = dest;
   cResult r;

   if (utils::fileexists(dest))
   {
      if (forceUpdate)
      {
         CommandLine op;
         op.command = "git";
         op.args = { "pull" };
         r+= utils::runcommand_stream(op, kORaw, dest, {}, NULL);
      }
   }
   else
   {
      utils::makedirectory(dest, S_700);

      CommandLine op;
      op.command = "git";
      op.args = { "clone","--progress","-b",mTag,
         "--depth","1",mURL,"." };
      r+=utils::runcommand_stream(op, kORaw, dest, {}, NULL);
   }

   CommandLine op;
   op.command = "git";
   op.args = { "checkout",mTag };
   r+= utils::runcommand_stream(op, kORaw, dest, {}, NULL);
   return r;
}

// copies the contents of src to dest.
cResult recursiveCopy(Poco::Path src, Poco::Path dest)
{
   if (!utils::fileexists(dest))
      return cError("Destination directory does not exist.");

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
            logmsg(kLDEBUG, "Copied " + it->path() + " to " + dest.toString());
         }
      }
   }
   catch (Poco::Exception & e)
   {
      fatal(e.what());
   }
   return kRSuccess;
}

cResult gitcache::copyTo(Poco::Path dest, bool forceUpdate) const
{
   Poco::Path p;
   cResult r = get(p, forceUpdate);
   if (!r.success())
      return r;

   drunner_assert(utils::fileexists(p), "Directory does not exist: " + p.toString());

   return recursiveCopy(p, dest);
}

std::string gitcache::hash(std::string url) const
{
   using Poco::DigestOutputStream;
   using Poco::DigestEngine;
   using Poco::MD5Engine;

   MD5Engine md5;
   DigestOutputStream ostr(md5);
   ostr << url;
   ostr.flush(); // Ensure everything gets passed to the digest engine
   const DigestEngine::Digest& digest = md5.digest(); // obtain result
   std::string result = DigestEngine::digestToHex(digest);
   return result;
}


