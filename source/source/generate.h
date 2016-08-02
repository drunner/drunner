#ifndef __GENERATE_H
#define __GENERATE_H

#include <sstream>
#include <fstream>

#include <Poco/Path.h>

#include "enums.h"
#include "params.h"

void generate(
   Poco::Path fullpath,
   const mode_t mode,
   const std::string & content
);

#endif
