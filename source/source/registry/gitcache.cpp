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

   drunner_assert(mURL.length() > 0, "Empty URL given to gitcache");
}

static bool _sGitChecked = false;
static bool _sHasGit = false;

cResult gitcache::runGitCommand(std::vector<std::string> args) const
{
   CommandLine op;
   std::string out;

   op.command = "git";

   tKeyVals env;
   std::string s = Poco::Environment::get("PATH");
   logmsg(kLDEBUG, "PATH = " + s);
   env["PATH"] = s;

   // Git commonly installed on Windows does not support HTTPS (GnuTLS fails to initialize).
   // So we just fall back to the container approach.
#ifndef _WIN32
   if (!_sGitChecked)
   {
      op.args = { "--version" };
      cResult r0 = utils::runcommand_stream(op, kOSuppressed, "", env, &out);
      logmsg(kLDEBUG, out);
      _sHasGit = r0.success();
      if (_sHasGit)
         logmsg(kLINFO, "Using host's git command.");
      else
         logmsg(kLINFO, "Git not found on host, using container.");
   }
#endif

   op.args = args;
   cResult r;

   if (!utils::fileexists(getCachePath()))
      utils::makedirectory(getCachePath(), S_700);
   drunner_assert(utils::fileexists(getCachePath()), "Failed to create " + getCachePath().toString());

   if (_sHasGit)
      r = utils::runcommand_stream(op, kOSuppressed, getCachePath(),env,&out);
   else
   { // run in container.
      fatal("Git running in container not yet implemented. Install git on host for this pre-release.");
   }
   if (r.success())
      logmsg(kLDEBUG, out);
   else
      logmsg(kLERROR, out);

   return r;
}

Poco::Path gitcache::getCachePath() const
{
   Poco::Path dest = drunnerPaths::getPath_GitCache();
   dest.pushDirectory(hash(mURL));
   return dest;
}


cResult gitcache::get(Poco::Path & p, bool forceUpdate) const
{
   // checkout repo, copy subfolder if present.
   // git clone --progress -b master --depth 1 https://github.com/drunner/d10_rocketchat
   drunner_assert(mURL.length() > 0, "Empty URL given to gitcache");

   cResult r;
   p = getCachePath();

   Poco::Path gitfolder = p;
   gitfolder.pushDirectory(".git");

   if (utils::fileexists(gitfolder))
   {
      if (forceUpdate)
      {
         logmsg(kLDEBUG, "Updating via git.");
         r+= runGitCommand({ "pull" });
      }
   }
   else
   {
      logmsg(kLDEBUG, "Cloning via git.");
      r+= runGitCommand({ "clone","--progress","-b",mTag,"--depth","1",mURL,"." });
   }


   logmsg(kLDEBUG, "Checking out tag "+mTag+" via git.");
   r+=runGitCommand({ "checkout",mTag });
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


