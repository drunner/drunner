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

private:
   std::chrono::steady_clock::time_point mStart;
};

namespace timeutils
{
   std::string getDateTimeStr();
   std::chrono::system_clock::time_point dateTimeStr2Time(std::string dts);

   std::string getArchiveName(std::string servicename);
   std::chrono::system_clock::time_point archiveName2Time(std::string aname);
};

#endif
