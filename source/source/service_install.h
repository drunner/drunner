#ifndef __SERVICE_INSTALL
#define __SERVICE_INSTALL

#include <string>
#include "cresult.h"
#include "service_paths.h"

class service_install : public servicePaths
{
public:
   service_install(std::string servicename);
   service_install(std::string servicename, std::string imagename);

   cResult obliterate();
   cResult uninstall();
   cResult recover();
   cResult update();
   cResult install();
   cResult service_restore(const std::string & backupfile);

private:
   void _createVolumes(std::vector<std::string> & volumes);
   void _ensureDirectoriesExist() const;
   cResult _recreate(bool updating);

   std::string mImageName;
};


#endif

