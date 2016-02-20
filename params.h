#include <vector>
#include <string>

#ifndef __PARAMS_H
#define __PARAMS_H

enum eCommand {
   c_clean,
   c_list,
   c_update,
   c_checkimage,
   c_backup,
   c_restore,
   c_install,
   c_recover,
   c_uninstall,
   c_obliterate,
   c_enter,
   c_status
};

enum eOutputmode {
   om_normal,
   om_verbose,
   om_silent
};

class params {
public:
   params(int argc, char **argv);
   std::string substitute( const std::string & source ) const;

   std::string mVersion;
   eCommand mCmd;
   eOutputmode mOMode;
   std::vector<std::string> mOptions;
};


#endif
