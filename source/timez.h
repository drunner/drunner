#ifndef __TIMEZ_H
#define __TIMEZ_H

#include <string>
#include <chrono>

class timez
{
public:
   timez();
   void restart();

   int getmilliseconds();
   std::string getelpased();

   static std::string getDateTimeStr();

private:
   std::chrono::steady_clock::time_point mStart;
};



#endif
