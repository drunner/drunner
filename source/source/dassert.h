#ifndef _D_ASSERT_H
#define _D_ASSERT_H

#include <sstream>

#include "globallogger.h"

inline void _dassert(const char* expression, const std::string message, const char* file, int line)
{
   std::ostringstream oss;
   oss << "Assertion " << expression;
   oss << " failed, file " << file << " line " << line;
   logmsg(kLDEBUG, oss.str());
   logmsg(kLERROR, message);
}

#define drunner_assert(EXPRESSION,MESSAGE) ((EXPRESSION) ? (void)0 : _dassert(#EXPRESSION, MESSAGE, __FILE__, __LINE__)) 


#endif

