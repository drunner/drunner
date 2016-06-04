#ifndef __GLOBALLOGGER_H
#define __GLOBALLOGGER_H

#include "enums.h"
#include "params.h"
#include <string>
#include <memory>

class GlobalLogger
{
public:
   static std::shared_ptr<GlobalLogger> instance();
   static void init(const params & p);
   static void init(); // no logging
   GlobalLogger(eLogLevel logcutoff, bool loggingenabled);

   void logmsg(eLogLevel level, std::string s);
   void logverbatim(eLogLevel level, std::string s);

   static std::string timestamp();
   static std::string levelname(eLogLevel level);
   static std::string getheader(eLogLevel level);
private:
   void logmsg_low(eLogLevel level, std::string s);

   static std::shared_ptr<GlobalLogger> s_instance;
   const eLogLevel mLogCutoff;
   const bool mLoggingEnabled;
};

void logmsg(eLogLevel level, std::string s)
{
   GlobalLogger::instance()->logmsg(level, s);
}
void logverbatim(eLogLevel level, std::string s)
{
   GlobalLogger::instance()->logverbatim(level, s);
}
void fatal(std::string s)
{
   GlobalLogger::instance()->logmsg(kLFATAL, s);
}



#endif
