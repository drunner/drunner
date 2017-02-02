#ifndef __REGISTRIES_H
#define __REGISTRIES_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "service_paths.h"
#include "sourceplugins.h"
#include "variables.h"

namespace sourceplugins
{
   enum protocol
   {
      kGit,
      kLocal,
      kDocker
   };

   class sourceinfo
   {
   public:
      sourceinfo() : mSet(false) {}
      sourceinfo(protocol _protocol, std::string _url, std::string _tag, std::string _desc) : 
         mSet(true), mProtocol(_protocol), mURL(_url), mTag(_tag), mDescription(_desc) {}
      
      bool mSet;
      protocol mProtocol; // git, local, docker
      std::string mURL;
      std::string mTag;
      std::string mDescription;
   };

   class registries
   {
   public:
      registries();

      cResult addregistry(std::string nicename, std::string giturl);
      cResult delregistry(std::string nicename);
      cResult showregistries();

      sourceinfo get(const std::string imagename) const;

   private:
      cResult load();
      cResult save();
      cResult splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag) const;

      keyVals mData;
      Poco::Path mPath;
   };
}

#endif