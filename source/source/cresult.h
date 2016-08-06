#ifndef __C_RESULT_H
#define __C_RESULT_H

#include <Poco/Foundation.h>
#include <string>

#include "globallogger.h"

enum _eResult
{
   kRSuccess = 0,
   kRError = 1,
   kRNoChange = 3,
   kRNotImplemented = 127,
};

#define cError(MESSAGE) (cResult(kRError, MESSAGE, std::string(__FILE__) + " (" + std::string( __func__ )+ "), line " + std::to_string(__LINE__))) 

class cResult
{
public:

   cResult()
   {
      mResult = kRNoChange;
   }

   cResult(_eResult x) : mResult(x)
   {
   }

   cResult(_eResult x, std::string msg, std::string context) : mResult(x), mMessage(msg), mContext(context)
   {
   }

   cResult(int x)
   {
      mResult = kRError;
      if (x == 0 || x == 3 || x == 127)
         mResult = (_eResult)x;
   }

   cResult & operator += (const cResult & rhs)
   {
      if (error())
         return *this;
      if (rhs.error())
         return *this = rhs;

      if (notImpl())
         return *this;
      if (rhs.notImpl())
         return *this = rhs;

      if (noChange() && !rhs.noChange())
         return *this = rhs;

      return *this;
   }

   std::string what() 
   {
      if (mMessage.length()>0)
         return mMessage; 
      if (noChange()) return "No Change";
      if (error()) return "Error";
      if (notImpl()) return "Not Implemented";
      return "Success";
   }
   std::string context()
   {
      if (mContext.length()>0)
         return mContext;
      return "No Context Provided";
   }

   operator _eResult() const { return mResult; }
   bool error() const { return mResult == kRError; }
   bool noChange() const { return mResult == kRNoChange; }
   bool notImpl() const { return mResult == kRNotImplemented; }
   bool success() const { return mResult == kRSuccess;  }
private:
   _eResult mResult;
   std::string mMessage;
   std::string mContext;
};


#endif 