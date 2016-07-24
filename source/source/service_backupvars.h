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

class backupvars
{
public:
   // reading ctor
   backupvars(Poco::Path path);
   
   // creates from drunnerCompose.
   void createFromServiceLua(std::string imagename, const servicelua::luafile & syf);

   const std::vector<std::string> & getDockerVolumeNames() const;
   std::string getImageName() const;

   cResult loadvars();
   cResult savevars() const;
   
   static const std::string filename;

private:
   std::vector<std::string> mVolumes;
   std::string mImageName;
   Poco::Path mPath;

   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mVolumes,mImageName); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mVolumes, mImageName); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(backupvars, 1);

#endif
