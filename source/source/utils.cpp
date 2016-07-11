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

#include <sys/stat.h>

#include "utils.h"
#include "exceptions.h"
#include "service_log.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "enums.h"
#include "chmod.h"
#include "drunner_paths.h"

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

   int runcommand(std::string command, std::vector<std::string> args)
   {
      std::string out;
      return runcommand(command, args, out, false);
   }

   int runcommand(std::string command, std::vector<std::string> args, std::string &out, bool trim)
   {
      int rval;

      try
      {
         Poco::Pipe outpipe;
         Poco::ProcessHandle ph = Poco::Process::launch(command, args, 0, &outpipe, &outpipe); // use the one pipe for both stdout and stderr.
         Poco::PipeInputStream istrout(outpipe);
         Poco::StreamCopier::copyToString(istrout, out);

         rval = ph.wait();
      }
      catch (Poco::SystemException & se)
      {
         std::ostringstream oss;
         oss << "Runcomand: " << command;
         for (auto x : args) oss << " " << x;
         oss << std::endl << se.displayText();
         fatal(oss.str());
      }

      if (trim)
         Poco::trimInPlace(out);
      return rval;
   }

   int bashcommand(std::string bashline, std::string & op, bool trim)
   {
      std::vector<std::string> args = { "-c", bashline };
      return runcommand("/bin/bash", args, op, trim);
   }

   int bashcommand(std::string bashline)
   {
      std::string op;
      return bashcommand(bashline, op, false);
   }

   int dServiceCmd(std::string command, const std::vector<std::string> & args, bool isServiceCmd)
   { // streaming as the command runs.

      // sanity check parameters.
      Poco::Path bfp(command);
      poco_assert(utils::fileexists(bfp));
      poco_assert(bfp.isFile());
      poco_assert(bfp.getFileName().compare(args[0]) != 0);

      // log the command, getting the args right is non-trivial in some cases so this is useful.
      std::string cmd = command;
      for (const auto & entry : args)
         cmd += " [" + entry + "]";
      logmsg(kLDEBUG, "dServiceCmd: " + cmd);

      Poco::Pipe outpipe;
      Poco::ProcessHandle ph = Poco::Process::launch(command, args, 0, &outpipe, &outpipe);
      Poco::PipeInputStream istrout(outpipe);

      // stream the output to the logger.
      dServiceLog(istrout, isServiceCmd);

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


   bool commandexists(std::string command)
   {
      return (0 == bashcommand("command -v " + command));
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

      std::vector<std::string> args = { "pull",imagename };
      int rval = runcommand("docker", args);

      if (rval==0 && op.find("Image is up to date",0) != std::string::npos)
         return kRNoChange;

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

   void makesymlink(Poco::Path file, Poco::Path link)
   {
	if (!utils::fileexists(file))
		logmsg(kLERROR, "Can't link to " + file.toString() + " because it doesn't exist");
   if (utils::fileexists(link))
      utils::delfile(link);
	std::string cmd = "ln -s " + file.toString() + " " + link.toString();
	std::string op;
	if (utils::bashcommand(cmd, op, false) != 0)
		logmsg(kLERROR, "Failed to create symbolic link for drunner. "+op);
   else
      logmsg(kLDEBUG, "Created symlink at " + link.toString());
   }

   void deltree(Poco::Path s)
   {
      Poco::File f(s);
      if (f.exists())
      {
         f.remove(true);
         logmsg(kLDEBUG, "Recursively deleted " + s.toString());
      }
      else
         logmsg(kLDEBUG, "Directory " + s.toString() + " does not exist (no need to delete).");

//      logmsg(kLERROR, "Unable to remove existing directory at "+s+" - "+op);
   }

   void movetree(const std::string &src, const std::string &dst)
   {
      //std::error_code & ec;
      if (0!= std::rename(src.c_str(), dst.c_str()))
         logmsg(kLERROR, "Unable to move " + src + " to " + dst);
   }

   void delfile(Poco::Path fullpath)
   {
      Poco::File f(fullpath);
      if (f.exists())
      {
         f.remove();
         logmsg(kLDEBUG, "Deleted " + fullpath.toString());
      }
   }

   std::string getHostIP()
   {
      std::string hostIP;
      if (utils::bashcommand("ip route get 1 | awk '{print $NF;exit}'", hostIP, true) != 0)
         return "";
      return hostIP;
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

   bool dockerVolExists(const std::string & vol)
   { // this could be better - will match substrings rather than whole volume name. :/ 
      // We name with big unique names so unlikely to be a problem for us.
      int rval = utils::bashcommand("docker volume ls | grep \"" + vol + "\"");
      return (rval == 0); 
   }

   std::string getenv(std::string envParam)
   {
      const char * cstr = std::getenv(envParam.c_str());
      std::string r;
      if (cstr != NULL)
         r = std::string(cstr);
      return r;
   }

   bool copyfile(std::string src, std::string dest)
   {
      // boost bug makes copy_file grumpy with c++11x.
      // also can't copy to another filesystem.
      // so we just use bash.
      int r = bashcommand("cp -a " + src + " " + dest);
      return (r == 0);
   }

   void downloadexe(std::string url, Poco::Path filepath)
   {
      // only download if server has newer version.      
      int rval = utils::bashcommand("curl " + url + " -z " + filepath.toString() + " -o " + filepath.toString() + " --silent --location 2>&1 && chmod 0755 " + filepath.toString());
      if (rval != 0)
         logmsg(kLERROR, "Unable to download " + url);
      if (!utils::fileexists(filepath))
         logmsg(kLERROR, "Failed to download " + url + ", curl return value is success but expected file "+ filepath.toString() +" does not exist.");
      logmsg(kLDEBUG, "Successfully downloaded " + filepath.toString());
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



   dockerrun::dockerrun(const std::string & cmd, const std::vector<std::string> & args, std::string dockername)
      : mDockerName(dockername)
   {
      int rval = utils::dServiceCmd(cmd, args, false);
      if (rval != 0)
      {
         std::ostringstream oss;
         for (auto entry : args)
            oss << entry << " ";
         logmsg(kLDEBUG, oss.str());
         tidy(); // throwing from ctor does not invoke dtor!
         std::ostringstream oss2;
         oss2 << "Docker command failed. Return code=" << rval;
         logmsg(kLERROR, oss2.str());
      }
   }
   dockerrun::~dockerrun()
   {
      tidy();
   }

   void dockerrun::tidy()
   {
      int rval = utils::bashcommand("docker rm " + mDockerName);
      if (rval != 0)
         std::cerr << "failed to remove " + mDockerName << std::endl; // don't throw on dtor.
      else
         logmsg(kLDEBUG, "Deleted docker container " + mDockerName);
   }


} // namespace utils
