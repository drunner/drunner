#ifndef __GENERATE_H
#define __GENERATE_H

#include <sstream>
#include <fstream>

#include <Poco/Path.h>

#include "enums.h"
#include "params.h"
#include "cresult.h"

bool filesameasstring(std::string filename, const std::string & thestring);

// outputs file if it has changed.
cResult generate(
   Poco::Path fullpath,
   const mode_t mode,
   const std::string & content
);

// just dumps out file. logs error if fails.
void generate_low(std::string tfile, const std::string & content);

#endif
