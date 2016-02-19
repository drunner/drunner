#include <vector>

#ifndef __UTILS_H
#define __UTILS_H

// Shell utilities
namespace utils
{
      typedef std::vector<std::string> strvec;
      //int runcommand(std::string cmd, std::vectror args, std::vector & logstdout, std::vector & logstderr);

      std::string getVersion();
}

#endif
