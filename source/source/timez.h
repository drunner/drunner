#ifndef __TIMEZ_H
#define __TIMEZ_H

#include <string>
#include <chrono>
#include <Poco/DateTime.h>

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
   std::string getDateTimeStr(); // suitable for filenames.
   std::string getLogTimeStamp();
   Poco::DateTime dateTimeStr2Time(std::string dts);

   std::string getArchiveName(std::string servicename);
   Poco::DateTime archiveName2Time(std::string aname);
};

#endif
