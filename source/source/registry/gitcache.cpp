#include "Poco/DirectoryIterator.h"
#include "Poco/String.h"
#include "Poco/DigestStream.h"
#include "Poco/MD5Engine.h"
#include "Poco/Environment.h"

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

   tKeyVals env;
   std::string s = Poco::Environment::get("PATH");
   logmsg(kLDEBUG, "PATH = " + s);
   env["PATH"] = s;

   if (utils::fileexists(dest))
   {
      if (forceUpdate)
      {
         logmsg(kLDEBUG, "Updating via git.");

         CommandLine op;
         op.command = "git";
         op.args = { "pull" };
         r+= utils::runcommand_stream(op, kORaw, dest, env, NULL);
      }
   }
   else
   {
      utils::makedirectory(dest, S_700);

      logmsg(kLDEBUG, "Cloning via git.");

      CommandLine op;
      op.command = "git.exe";
      op.args = { "clone","--progress","-b",mTag,
         "--depth","1",mURL,"." };
      r+=utils::runcommand_stream(op, kORaw, dest, env, NULL);
   }


   logmsg(kLDEBUG, "Checking out tag "+mTag+" via git.");

   CommandLine op;
   op.command = "git";
   op.args = { "checkout",mTag };
   r+= utils::runcommand_stream(op, kORaw, dest, env, NULL);
   return r;
}

// copies the contents of src to dest.
cResult recursiveCopy(Poco::Path src, Poco::Path dest)
{
   if (!utils::fileexists(dest))
      return cError("Destination directory does not exist.");

   logmsg(kLDEBUG, "Copying tree contents in " + src.toString() + " to " + dest.toString());

   try
   {
      Poco::DirectoryIterator end;
      for (Poco::DirectoryIterator it(src); it != end; ++it)
      {
         if (it->isDirectory())
         {
            Poco::Path p( it->path() );
            p.makeDirectory();
            drunner_assert(p.depth() > 1, "Asked to copy from root directory. Disallowed.");
            drunner_assert(p.isDirectory(), "Converted path is not a directory.");
            std::string dirname = p[p.depth() - 1];

            if (Poco::icompare(dirname, ".git") != 0)
            {
               drunner_assert(dirname.length() > 0, "Empty path.");
               logmsg(kLDEBUG, "Copying folder "+it->path()+" ("+  dirname + ") to "+dest.toString());

               Poco::Path subdest(dest);
               subdest.pushDirectory(dirname);
               utils::makedirectory(subdest, S_700);
               recursiveCopy(it->path(), subdest);
            }
            else
               logmsg(kLDEBUG, "Skipping .git directory tree");
         }
         else
         {
            drunner_assert(it->path().length() > 0, "Empty file path.");
            it->copyTo(dest.toString());
            logmsg(kLDEBUG, "Copied " + it->path() + " to " + dest.toString());
         }
      }
   }
   catch (Poco::Exception & e)
   {
      fatal(e.what());
   }
   logmsg(kLDEBUG, "Finished copying " + src.toString() + " to " + dest.toString());
   return kRSuccess;
}

cResult gitcache::copyTo(Poco::Path dest, bool forceUpdate) const
{
   logmsg(kLDEBUG, "Copying repo " + mURL + " with tag " + mTag + " to " + dest.toString());

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


