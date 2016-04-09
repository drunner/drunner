#ifndef __GENERATE_H
#define __GENERATE_H

#include <sstream>
#include <fstream>

#include "enums.h"
#include "params.h"

void generate(
   const std::string & fullpath,
   const params & p,
   const mode_t mode,
   const std::string & content
);

#endif
