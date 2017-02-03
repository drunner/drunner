#include "Poco/String.h"

#include "captaincopy.h"
#include "utils.h"

cResult _CopyGit(std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyLocal(std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyDocker(std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopyHTTP(std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
   return kRNotImplemented;
}
cResult _CopySSH(std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
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
   default:
      return kP_ERROR;
   }
}

cResult CaptainCopy(eProtocol protocol, std::string url, std::string path, Poco::Path target, eCopyMode mode)
{
   switch (protocol)
   {
   case kP_Git:
      return _CopyGit(url, path, target, mode);

   case kP_Local:
      return _CopyLocal(url, path, target, mode);

   case kP_Docker:
      return _CopyDocker(url, path, target, mode);

   case kP_HTTP:
      return _CopyHTTP(url, path, target, mode);

   case kP_SSH:
      return _CopySSH(url, path, target, mode);

   default:
      fatal("CaptainCopy : Unknown protocol.");
   }
   return cError("CaptainCopy : Logic err");
}
