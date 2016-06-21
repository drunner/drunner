#include "timez.h"

#include <sstream>
#include <ctime>

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

std::string timez::getDateTimeStr()
{
   char s[100];

   auto tnow = std::chrono::system_clock::now();
   std::time_t now_time = std::chrono::system_clock::to_time_t(tnow);

   strftime(s, 99, "%Y_%m_%d__%H_%M_%S", std::localtime(&now_time));
   return std::string(s);
}