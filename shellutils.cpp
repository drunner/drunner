#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <errno.h>

#include "pstream.h"
#include "shellutils.h"

namespace utils
{
   std::string getVersion()    {
      return "0.1 dev";
   }
}
