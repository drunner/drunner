#include <cereal/archives/json.hpp>
#include <fstream>

#include "service_backupvars.h"
#include "dassert.h"

const std::string backupvars::filename("backupvars.json");

backupvars::backupvars(Poco::Path path) : mPath(path)
{
   drunner_assert(path.isFile(),"The backupvars file is not a Poco file.");
}

void backupvars::createFromServiceLua(std::string imagename, const servicelua::luafile & syf)
{
   drunner_assert(mVolumes.size() == 0, "Dirty volumes.");
   drunner_assert(mImageName.length() == 0,"Dirty imagename.");
   drunner_assert(imagename.length() > 0, "Empty imagename.");
   mImageName = imagename;
   syf.getBackupDockerVolumeNames(mVolumes);
}

cResult backupvars::loadvars()
{
   std::ifstream ifs(mPath.toString());
   if (ifs.bad())
      return kRError;
   cereal::JSONInputArchive archive(ifs);
   archive(*this);

   drunner_assert(mImageName.length() > 0, "Empty imagename.");

   return kRSuccess;
}

cResult backupvars::savevars() const
{
   drunner_assert(mImageName.length() > 0, "Empty imagename.");

   std::ofstream os(mPath.toString());
   if (os.bad())
      return kRError;
   cereal::JSONOutputArchive archive(os);
   archive(*this);
   return kRSuccess;
}


const std::vector<std::string> & backupvars::getDockerVolumeNames() const
{
   return mVolumes;
}

std::string backupvars::getImageName() const
{
   return mImageName;
}

