#ifndef __ENUMS_H
#define __ENUMS_H

enum eResult 
{
   kRSuccess=0,
   kRError=1,
   kRNoChange=3
};


enum eLogLevel
{
   kLDEBUG=0,
   kLINFO=1,
   kLWARN=2,
   kLERROR=3
};


enum eCommand {
   c_UNDEFINED,
   c_setup,
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


#endif

