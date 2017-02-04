#include <fstream>

#include "Poco/String.h"
#include "Poco/File.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/URI.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"

#include "captaincopy.h"
#include "utils.h"


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

cResult _CopyGit(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyLocal(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyDocker(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyHTTP(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopySSH(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   if (mode == kCM_File)
      return downloadfile(s.mURL, target);

   return kRNotImplemented;
}



eProtocol ProtoParse(std::string protocol)
{
   switch (s2i(Poco::toLower(protocol).c_str()))
   {
   case s2i("git"):
      return kP_Git;
   case s2i("http"):
      return kP_HTTP; 
   case s2i("docker"):
      return kP_Docker;
   case s2i("local"):
      return kP_Local;
   case s2i("ssh"):
      return kP_SSH;
   default:
      return kP_ERROR;
   }
}

std::string unParseProto(eProtocol p)
{
   switch (p)
   {
   case kP_Git:
      return "git";
   case kP_Local:
      return "local";
   case kP_Docker:
      return "docker";
   case kP_HTTP:
      return "http";
   case kP_SSH:
      return "ssh";

   default:
      fatal("CaptainCopy : Unknown protocol.");
   }
}

cResult CaptainCopy(SourceInfo s, Poco::Path target, eCopyMode mode)
{
   switch (s.mProtocol)
   {
   case kP_Git:
      return _CopyGit(s, target, mode);

   case kP_Local:
      return _CopyLocal(s, target, mode);

   case kP_Docker:
      return _CopyDocker(s, target, mode);

   case kP_HTTP:
      return _CopyHTTP(s, target, mode);

   case kP_SSH:
      return _CopySSH(s, target, mode);

   default:
      fatal("CaptainCopy : Unknown protocol.");
   }
   return cError("CaptainCopy : Logic err");
}
