#ifndef __DRUNNER_PATHS_H
#define __DRUNNER_PATHS_H

#include <Poco/Path.h>

namespace drunnerPaths
{
   Poco::Path getPath_Root();       // the path of the drunner root (for all config). %APPDATA%/drunner [win] or $HOME/.drunner [lin]
   Poco::Path getPath_Exe();        // the path of the drunner execuatble. can be anywhere
   Poco::Path getPath_Exe_Target(); // the target path of the drunner executable. On Linux this is the same as getPath_Exe (we don't move it). On Windows it's .drunner/bin (we do move it). 

   Poco::Path getPath_Bin();
   Poco::Path getPath_dServices();
   Poco::Path getPath_Temp();
   Poco::Path getPath_HostVolumes();
   Poco::Path getPath_Settings();
   Poco::Path getPath_Logs();

   Poco::Path getPath_drunnerSettings_json();

   std::string getdrunnerUtilsImage();
}


#endif
