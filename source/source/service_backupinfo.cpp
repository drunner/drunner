#include <cereal/archives/json.hpp>
#include <fstream>

#include "service_backupinfo.h"
#include "dassert.h"

const std::string backupinfo::filename("backupinfo.json");

backupinfo::backupinfo(Poco::Path path) : mPath(path)
{
   drunner_assert(path.isFile(),"The backupinfo file is not a Poco file.");
}

void backupinfo::create(std::string imagename)
{
   drunner_assert(imagename.length() > 0, "Empty imagename.");
   mImageName = imagename;
   mVersion = 2;
}

cResult backupinfo::loadvars()
{
   std::ifstream ifs(mPath.toString());
   if (ifs.bad())
      return cError("Bad input stream to backupinfo::loadvars. :/");
   cereal::JSONInputArchive archive(ifs);
   archive(*this);

   drunner_assert(mImageName.length() > 0, "Empty imagename.");
   drunner_assert(mVersion == 2, "Incompatible backup version.");

   return kRSuccess;
}

cResult backupinfo::savevars() const
{
   drunner_assert(mImageName.length() > 0, "Empty imagename.");

   mVersion = 2;

   std::ofstream os(mPath.toString());
   if (os.bad())
      return cError("Bad output stream.");
   cereal::JSONOutputArchive archive(os);
   archive(*this);
   return kRSuccess;
}

std::string backupinfo::getImageName() const
{
   return mImageName;
}

