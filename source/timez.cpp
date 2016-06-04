#include "timez.h"

#include <sstream>

timez::timez()
{
   restart();
}

void timez::restart()
{
   mStart = std::chrono::steady_clock::now();
}

int timez::getmilliseconds()
{
   auto end = std::chrono::steady_clock::now();
   int t=std::chrono::duration_cast<std::chrono::microseconds>(end - mStart).count();
   return t;
}

std::string timez::getelpased()
{
   int ms = getmilliseconds();
   std::ostringstream oss;
   oss << 0.01*((ms+5000)/10000) <<"s";
   return oss.str();
}
