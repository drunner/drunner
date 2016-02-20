#include <vector>
#include <string>
#include "params.h"

#ifndef __UTILS_H
#define __UTILS_H

// Shell utilities
namespace utils
{
      //typedef std::vector<std::string> strvec;
      //int runcommand(std::string cmd, std::vectror args, std::vector & logstdout, std::vector & logstderr);
      std::string replacestring(std::string subject, const std::string& search,
                                const std::string& replace);
}

#endif
