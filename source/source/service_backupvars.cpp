#include <cereal/archives/json.hpp>
#include <fstream>

#include "service_backupvars.h"

const std::string backupvars::filename("backupvars.json");

backupvars::backupvars(Poco::Path path) : mPath(path)
{
   poco_assert(path.isFile());
}

void backupvars::createFromServiceYml(std::string imagename, const serviceyml::file & syf)
{
   poco_assert(mVolumes.size() == 0);
   poco_assert(mImageName.length() == 0);
   mImageName = imagename;
   for (const auto & entry : syf.getVolumes())
      mVolumes.push_back(entry.name());
}

cResult backupvars::loadvars()
{
   std::ofstream ifs(mPath.toString());
   if (ifs.bad())
      return kRError;
   cereal::JSONOutputArchive archive(ifs);
   archive(*this);
   return kRSuccess;
}

cResult backupvars::savevars() const
{
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

