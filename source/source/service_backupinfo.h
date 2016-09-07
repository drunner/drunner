#ifndef __SH_BACKUPVARIABLES_H
#define __SH_BACKUPVARIABLES_H

#include <string>
#include <vector>

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <Poco/Path.h>

#include "service_lua.h"
#include "cresult.h"

class backupinfo
{
public:
   // reading ctor
   backupinfo(Poco::Path path);
   
   // creates from drunnerCompose.
   //void createFromServiceLua(std::string imagename, const servicelua::luafile & syf);
   void create(std::string imagename, bool devmode);

   //const std::vector<std::string> & getDockerVolumeNames() const;
   std::string getImageName() const;
   bool getDevMode() const;

   cResult loadvars();
   cResult savevars() const;
   
   static const std::string filename;

private:
   //std::vector<std::string> mVolumes;
   std::string mImageName;
   bool mDevMode;

   Poco::Path mPath;

   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mImageName, mDevMode); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) 
   { 
      mDevMode = false;
      if (version == 1)
      {
         int version;
         ar(version, mImageName);
      }
      else if (version == 2)
         ar(mImageName, mDevMode);
      else
         fatal("Bad version in backup save file. Do you need to update dRunner?");
   }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(backupinfo, 2);

#endif
