#include <string>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <utility>
#include <errno.h>
#include <stdlib.h>
#include <system_error>

#include <Poco/String.h>
#include <Poco/Process.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Util/SystemConfiguration.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/DirectoryIterator.h>

#include <sys/stat.h>

#include "utils.h"
#include "exceptions.h"
#include "service_log.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "enums.h"
#include "chmod.h"
#include "drunner_paths.h"
#include "dassert.h"

namespace utils
{

   bool fileexists(const Poco::Path& name)
   {
      struct stat buffer;
      return (stat (name.toString().c_str(), &buffer) == 0);
   }

   bool stringisame(const std::string & s1, const std::string &s2 )
   {
      return (0 == Poco::icompare(s1, s2)); // http://pocoproject.org/slides/040-StringsAndFormatting.pdf
      //return boost::iequals(s1,s2);
   }

   // trim from left
   inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
   {
       s.erase(0, s.find_first_not_of(t));
       return s;
   }

   // trim from right
   inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
   {
       s.erase(s.find_last_not_of(t) + 1);
       return s;
   }

   // trim from left & right
   std::string& trim(std::string& s, const char* t)
   {
       return ltrim(rtrim(s, t), t);
   }

   std::string doquote(std::string s)
   {
      return "\""+s+"\"";
   }

   // copying versions

   inline std::string ltrim_copy(std::string s, const char* t = " \t\n\r\f\v")
   {
       return ltrim(s, t);
   }

   inline std::string rtrim_copy(std::string s, const char* t = " \t\n\r\f\v")
   {
       return rtrim(s, t);
   }

   std::string trim_copy(std::string s, const char* t)
   {
       return utils::trim(s, t);
   }

   int runcommand(const CommandLine & operation, std::string &out, tFlags rcf)
   {
      int rval;

      if (rcf & kRC_LogCmd)
      { // log the command
         std::ostringstream oss;
         oss << "Runcommand: " << operation.command;
         for (auto x : operation.args) oss << " " << x;
         logmsg(kLDEBUG, oss.str());
      }

      try
      {
         Poco::Pipe outpipe;
         Poco::ProcessHandle ph = Poco::Process::launch(operation.command, operation.args, 0, &outpipe, &outpipe); // use the one pipe for both stdout and stderr.
         Poco::PipeInputStream istrout(outpipe);
         Poco::StreamCopier::copyToString(istrout, out);

         rval = ph.wait();
      }
      catch (Poco::SystemException & se)
      {
         fatal(se.displayText());
      }

      if (rcf & kRC_Trim)
         Poco::trimInPlace(out);
      return rval;
   }


   int runcommand_stream(const CommandLine & operation, edServiceOutput outputMode, Poco::Path initialDirectory, const Poco::Process::Env & env)
   { // streaming as the command runs.

      // sanity check parameters.
      Poco::Path bfp(operation.command);
      //poco_assert(utils::fileexists(bfp));
      poco_assert(bfp.isFile());

      // log the command, getting the args right is non-trivial in some cases so this is useful.
      std::string cmd = operation.command;
      for (const auto & entry : operation.args)
         cmd += " [" + entry + "]";
      logmsg(kLDEBUG, "runcommand_stream: " + cmd);

      Poco::Pipe outpipe;
      Poco::ProcessHandle ph = Poco::Process::launch(operation.command, operation.args, 
         initialDirectory.toString(), 0, &outpipe, &outpipe, env);
      Poco::PipeInputStream istrout(outpipe);

      if (outputMode == kORaw)
         Poco::StreamCopier::copyStreamUnbuffered(istrout, std::cout);
      else if (outputMode != kOSuppressed)
         dServiceLog(istrout);

      int rval = ph.wait();
      std::ostringstream oss;
      oss << bfp.getFileName() << " returned " << rval;
      logmsg(kLDEBUG, oss.str());
      return rval;
   }

   std::string getabsolutepath(std::string path)
   {
      Poco::Path p(path);
      if (!p.isAbsolute())
         p.makeAbsolute();
      return p.toString(Poco::Path::PATH_NATIVE);
   }


   std::string replacestring(std::string subject, const std::string& search,
        const std::string& replace)
   {
      size_t pos = 0;
      if (search.empty() || subject.empty())
         return "";
      while((pos = subject.find(search, pos)) != std::string::npos)
      {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
      }
      return subject;
   }


   Poco::Path get_usersbindir()
   {
      Poco::Path h(Poco::Path::home());
      poco_assert(h.isDirectory());
      h.pushDirectory("bin");
      return h;
   }

   bool imageisbranch(const std::string & imagename)
   {
      std::size_t pos = imagename.find_last_of("/:");
      if (pos == std::string::npos || imagename[pos] != ':')
         return false;

      std::string branchname=imagename.substr(pos+1);
      if (stringisame(branchname,"master"))
         return false;

      return true;
   }

   eResult pullimage(const std::string & imagename)
   {
      std::string op;
      CommandLine cl("docker", { "pull",imagename });
      int rval = runcommand_stream(cl, GlobalContext::getParams()->supportCallMode());
      return (rval==0) ? kRSuccess : kRError;
   }

   bool getFolders(const std::string & parent, std::vector<std::string> & folders)
   {
      Poco::File f(parent);
      if (!f.exists())
         return false;

      f.list(folders);
      return true;
   }

   /// Try to find in the Haystack the Needle - ignore case
   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle)
   {
      using namespace std;
      auto it = search(
         strHaystack.begin(), strHaystack.end(),
         strNeedle.begin(),   strNeedle.end(),
         [](char ch1, char ch2) { return toupper(ch1) == toupper(ch2); }
      );
      return (it != strHaystack.end() );
   }

   void makedirectory(Poco::Path d, mode_t mode)
   {
      Poco::File f(d);
      if (!f.exists())
      {
         if (!utils::fileexists(d.parent()))
            fatal("Parent directoy doesn't exist: " + d.parent().toString());
         f.createDirectory();
         logmsg(kLDEBUG, "Created " + d.toString());
      }

      if (xchmod(d.toString().c_str(), mode)!=0)
         logmsg(kLERROR, "Unable to change permissions on "+d.toString());
   }

   eResult _makedirectories(Poco::Path path)
   {
      Poco::File f(path);

      if (f.exists())
         return kRNoChange;

      f.createDirectories();
      return (f.exists() ? kRSuccess : kRError);
   }
   void makedirectories(Poco::Path path, mode_t mode)
   {
      eResult r = _makedirectories(path);
      switch (r)
      {
      case kRSuccess:
         logmsg(kLDEBUG, "Created " + path.toString());

         if (xchmod(path.toString().c_str(), mode) != 0)
            logmsg(kLERROR, "Unable to change permissions on " + path.toString());

         return;
      case kRNoChange:
         return;
      default:
         fatal("Unable to create " + path.toString());
      }
   }

   // recusively delete the path given. we do this manually to set the permissions to writable,
   // so that windows doesn't cause permission denied errors.
   cResult deltree(Poco::Path s)
   {
      drunner_assert(s.isDirectory(), "deltree: asked to delete a file.");
      cResult rval = kRNoChange;

      try
      {
         Poco::DirectoryIterator end;
         for (Poco::DirectoryIterator it(s); it != end; ++it)
            if (it->isFile())
               rval += delfile(it->path());
            else
            {
               Poco::Path subdir(it->path());
               subdir.makeDirectory();
               rval += deltree(subdir);
            }

         Poco::File f(s);
         f.setWriteable(true);
         f.remove();
         logmsg(kLDEBUG, "Deleted " + f.path());
      }
      catch (const Poco::Exception & e) {
         logmsg(kLDEBUG, "Couldn't delete " + s.toString() + " - " + e.what());
         return kRError;
      }
      return kRSuccess;
   }

   void movetree(const std::string &src, const std::string &dst)
   {
      //std::error_code & ec;
      if (0!= std::rename(src.c_str(), dst.c_str()))
         logmsg(kLERROR, "Unable to move " + src + " to " + dst);
   }

   cResult delfile(Poco::Path fullpath)
   {
      drunner_assert(fullpath.isFile(), "delfile: asked to delete a directory: "+fullpath.toString());
      try
      {
         Poco::File f(fullpath);
         if (f.exists())
         {
            f.setWriteable(true);
            f.remove();
            logmsg(kLDEBUG, "Deleted " + fullpath.toString());
         }
      }
      catch (const Poco::Exception & e) {
         logmsg(kLDEBUG, "Couldn't delete "+fullpath.toString()+" - "+e.what());
         return kRError;
      }
      return kRSuccess;
   }

   std::string getTime()
   {
      std::time_t rtime = std::time(nullptr);
      return utils::trim_copy(std::asctime(std::localtime(&rtime)));
   }

   std::string getPWD()
   {
      std::string cwd(Poco::Path::current());
      return cwd;
   }

   std::string getenv(std::string envParam)
   {
      const char * cstr = std::getenv(envParam.c_str());
      std::string r;
      if (cstr != NULL)
         r = std::string(cstr);
      return r;
   }

   std::string alphanumericfilter(std::string s, bool whitespace)
   {
      std::string validchars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      if (whitespace) validchars += " \n";
      size_t pos;
      while ((pos = s.find_first_not_of(validchars)) != std::string::npos)
         s.erase(pos, 1);
      return s;
   }

   bool wordmatch(std::string s, std::string word)
   {
      // find whole word.
      auto pos = s.find(word);
      while (pos != std::string::npos)
      {
         pos += word.length();
         if (pos == s.length())
            return true;
         if (isspace(s[pos]))
            return true;
         pos = s.find(word, pos);
      }
      return false;
   }

   void getAllServices(std::vector<std::string>& services)
   {
      Poco::File f(drunnerPaths::getPath_dServices());
      if (!f.exists())
         logmsg(kLERROR, "Services folder does not exist.");

      f.list(services);
   }


   tempfolder::tempfolder(Poco::Path d) : mPath(d)
   {   // http://stackoverflow.com/a/10232761
      Poco::File f(d);

      if (!f.exists())
      {
         f.createDirectories();
         logmsg(kLDEBUG, "Created " + d.toString());
      }
      else
         die(d.toString() + " already exists. Can't use as temp folder. Aborting.");

      if (xchmod(d.toString().c_str(), S_777) != 0)
         die("Unable to change permissions on " + d.toString());
   }

   tempfolder::~tempfolder() 
   {
      tidy();
   }

   Poco::Path tempfolder::getpath() 
   { 
      poco_assert(mPath.isDirectory());
      return mPath; 
   }

   void tempfolder::die(std::string msg)
   {
      tidy();
      logmsg(kLERROR, msg); // throws. dtor won't be called since die is only called from ctor.
   }

   void tempfolder::tidy()
   {
      Poco::File f(mPath);
      if (f.exists())
         f.remove(true);
      logmsg(kLDEBUG, "Recursively deleted " + mPath.toString());
   }

   std::string _nextword(std::string & command, unsigned int & startpos)
   { 
      // skip leading whitespace.
      while (startpos < command.length() && iswspace(command[startpos]))
         ++startpos;
      
      // if the arg starts with a " then skip ahead to the closing quote, 
      // unescaping any other "'s in the arg as we go.
      int osp = startpos;
      if (startpos < command.length() && command[startpos] == '"')
      {
         ++osp;  ++startpos; // skip opening quote.
         while (startpos < command.length())
         {      // looking for closing quote.
            if (command[startpos] != '"')
               ++startpos;
            else if (startpos > 0 && command[startpos - 1] == '\\')
               command.erase(startpos - 1, 1); // erase the escape char and fall through.
            else
            { // we found the closing quote! :-)
               command.erase(startpos, 1); // erase the quote.
               break;
            }
         }
      }

      // find whitespace or end of string to terminate arg. We allow things like "dave"123  ->  dave123
      while (startpos < command.length() && !iswspace(command[startpos]))
         ++startpos;
      return command.substr(osp, startpos - osp);
   }

   cResult split_in_args(std::string command, CommandLine & cl)
   {
      poco_assert(cl.args.size() == 0);

      unsigned int startpos = 0;

      std::string nw = _nextword(command, startpos);
      if (nw.length() == 0)
         return kRError; // no command.
      cl.command = nw;

      while (true)
      {
         nw = _nextword(command, startpos);
         if (nw.length() == 0)
            return kRSuccess;
         cl.args.push_back(nw);
      }
      return kRError;
   }

} // namespace utils

void CommandLine::logcommand(std::string prefix) const
{
   std::ostringstream oss;
   oss << prefix;
   oss << command;
   for (const auto & a : args)
      oss << " [" << a << "]";
   logmsg(kLDEBUG, oss.str());
}
