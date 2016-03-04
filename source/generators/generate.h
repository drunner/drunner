#ifndef __GENERATE_H
#define __GENERATE_H

#include <sstream>
#include <fstream>
#include <sys/stat.h>

#include "enums.h"
#include "params.h"

static const mode_t S_ALLEXEC = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
static const mode_t S_ALLREAD = S_IRUSR | S_IRGRP | S_IROTH;

void generate(
   const std::string & fullpath,
   const params & p,
   const mode_t mode,
   const std::string & content
);

#endif
