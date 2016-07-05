#ifndef __BUILDNUM__H
#define __BUILDNUM__H

#include <string>

// e.g. 0.3 r91 - Tue Mar 22 21:19:59 NZDT 2016
std::string getVersionStr();

// e.g. 0.3 r91
std::string getVersionStrShort();

#endif
