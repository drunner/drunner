#ifndef __EXCEPTIONS_H
#define __EXCEPTIONS_H

#include <exception>
#include <string.h>

class eExit: public std::exception
{
   public:
      eExit(const char * msg, int exitCode=1) : mMsg(msg), mExitCode(exitCode)
      {
      }
   
      virtual const char* what() const throw()
      {
         return mMsg;
      }
      
      int exitCode() const throw() // we guarentee not to throw an exception.
      {
         return mExitCode;
      }
      
      bool hasMsg() const
      {
         return (strlen(mMsg)>0);
      }
      
   private:
      const char * mMsg;
      int mExitCode;
};


#endif