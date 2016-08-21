#ifndef __ENUMS_H
#define __ENUMS_H

#include <sys/stat.h>

#include <iostream>
#include <numeric>
#include <string>

#include <initializer_list>

#ifdef _WIN32
#include "chmod.h"
#endif

// some statics.

static const mode_t S_ALLREAD = S_IRUSR | S_IRGRP | S_IROTH;
static const mode_t S_500 = S_IRUSR | S_IWUSR;
static const mode_t S_700 = S_IRUSR | S_IWUSR | S_IXUSR;
static const mode_t S_755 = S_700 | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
static const mode_t S_777 = S_755 | S_IWGRP | S_IWOTH;
// enumerators used often.


enum eLogLevel
{
   kLDEBUG=0,
   kLINFO=1,
   kLWARN=2,
   kLERROR=3
};

enum edServiceOutput
{
   kORaw,
   kOSuppressed
};


enum eCommand {
   c_UNDEFINED,
   c_clean,
   c_list,
   c_setup,
   c_update,
   c_checkimage,
   c_backup,
   c_restore,
   c_install,
   c_uninstall,
   c_obliterate,
   c_unittest,
   c_servicecmd,
   c_help,
   c_saveenvironment,
   c_plugin,
   c_configure,
};


// -----------------------------------------------------------------------------


#endif
