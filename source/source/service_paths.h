#ifndef __SERVICE_PATHS_H
#define __SERVICE_PATHS_H

#include <string>
#include <Poco/Path.h>

#include "utils.h"

class servicePaths
{
public:
   servicePaths(const std::string & servicename);

   Poco::Path getPathdService() const;
   Poco::Path getPathHostVolume() const;
   Poco::Path getPathLaunchScript() const;
   Poco::Path getPathServiceVars() const;
   std::string getName() const;

   // provided by the dService.
   Poco::Path getPathServiceLua() const;

protected:
   const std::string mName;
};

class backupPathManager : private servicePaths   // not just path strings, creates/manages the directories.
{
public:
   backupPathManager(std::string servicename);

   Poco::Path getPathTempFolder() const;  // path to temp folder that backup archive contents will be packed/unpacked in. Created in ctor.
   Poco::Path getPathArchive() const;     // path to folder that stores the final archive (one .tar.enc)
   Poco::Path getPathSubArchives() const; // path to folder thtat stores the contents of the final archive, which includes a bunch of other archives.
   Poco::Path getPathHostVolArchiveFile() const;
   Poco::Path getPathdServiceDefArchiveFile() const;
   Poco::Path getPathArchiveFile() const;

private:
   utils::tempfolder mTempFolder;
};


#endif

