#ifndef __CAPTAIN_COPY_YEEEEAAAHHHH_
#define __CAPTAIN_COPY_YEEEEAAAHHHH_

#include "Poco/Path.h"
#include "cresult.h"

enum eProtocol
{
   kP_Git,
   kP_Local,
   kP_Docker,
   kP_HTTP,
   kP_SSH,
   kP_ERROR
};

enum eCopyMode
{
   kCM_File,
   kCM_Files,
   kCM_Tree
};

class SourceInfo
{
public:
   SourceInfo(eProtocol p, std::string url, std::string tag) :
      mProtocol(p), mURL(url), mTag(tag) {}

   eProtocol   mProtocol;
   std::string mURL;
   std::string mTag;
};

eProtocol ProtoParse(std::string protocol);
std::string unParseProto(eProtocol p);

cResult CaptainCopy(SourceInfo s, Poco::Path target, eCopyMode mode);

#endif