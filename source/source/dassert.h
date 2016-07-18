#ifndef _D_ASSERT_H
#define _D_ASSERT_H

#include <sstream>

#include "globallogger.h"
#include "globalcontext.h"
#include "exceptions.h"

inline void _dassert(const char* expression, const std::string message, const char* file, int line)
{
   std::ostringstream oss;
   oss << "Assertion " << expression;
   oss << " failed, file " << file << " line " << line;
   
   if (!GlobalContext::hasParams())
   {
      std::cerr << oss.str() << std::endl;
      std::cerr << message << std::endl;
      throw eExit();
   }
   else
   {
      logmsg(kLDEBUG, oss.str());
      logmsg(kLERROR, message);
   }
}

#define drunner_assert(EXPRESSION,MESSAGE) ((EXPRESSION) ? (void)0 : _dassert(#EXPRESSION, MESSAGE, __FILE__, __LINE__)) 


#endif

