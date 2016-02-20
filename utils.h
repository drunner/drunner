#include <vector>
#include <string>
#include "params.h"

#ifndef __UTILS_H
#define __UTILS_H

// Shell utilities
namespace utils
{
   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);

   void die( const params & p, std::string msg, int exit_code=1 );
   void dielow( std::string msg, int exit_code=1 );
}

#endif
