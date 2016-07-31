#include <cereal/archives/json.hpp>
#include <fstream>

#include "service_backupinfo.h"
#include "dassert.h"

const std::string backupinfo::filename("backupinfo.json");

backupinfo::backupinfo(Poco::Path path) : mPath(path)
{
   drunner_assert(path.isFile(),"The backupinfo file is not a Poco file.");
}

void backupinfo::createFromServiceLua(std::string imagename, const servicelua::luafile & syf)
{
   drunner_assert(mVolumes.size() == 0, "Dirty volumes.");
   drunner_assert(mImageName.length() == 0,"Dirty imagename.");
   drunner_assert(imagename.length() > 0, "Empty imagename.");
   mImageName = imagename;
   syf.getBackupDockerVolumeNames(mVolumes);
}

cResult backupinfo::loadvars()
{
   std::ifstream ifs(mPath.toString());
   if (ifs.bad())
      return cError("Bad input stream to backupinfo::loadvars. :/");
   cereal::JSONInputArchive archive(ifs);
   archive(*this);

   drunner_assert(mImageName.length() > 0, "Empty imagename.");

   return kRSuccess;
}

cResult backupinfo::savevars() const
{
   drunner_assert(mImageName.length() > 0, "Empty imagename.");

   std::ofstream os(mPath.toString());
   if (os.bad())
      return cError("Bad output stream.");
   cereal::JSONOutputArchive archive(os);
   archive(*this);
   return kRSuccess;
}

const std::vector<std::string> & backupinfo::getDockerVolumeNames() const
{
   return mVolumes;
}

std::string backupinfo::getImageName() const
{
   return mImageName;
}

