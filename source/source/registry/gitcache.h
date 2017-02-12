#ifndef __GITCACHE_H
#define __GITCACHE_H

#include <string>
#include "Poco/Path.h"
#include "cresult.h"

class gitcache
{
public:
   gitcache(std::string url, std::string tag = "");

   cResult get(Poco::Path & p, bool forceUpdate = false) const;
   
   static cResult recursiveCopyContents(Poco::Path src, Poco::Path dest, std::string skipDir=".git");

private:
   std::string hash(std::string url) const;
   cResult runGitCommand(std::vector<std::string> args) const;
   Poco::Path getCachePath() const;

   std::string mURL, mTag;
};

#endif

