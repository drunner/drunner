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

eProtocol ProtoParse(std::string protocol);
std::string unParseProto(eProtocol p);

cResult CaptainCopy(eProtocol protocol, std::string url, std::string path, Poco::Path target, eCopyMode mode);

#endif