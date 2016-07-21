#ifndef __C_RESULT_H
#define __C_RESULT_H

#include <Poco/Foundation.h>

#include "enums.h"

class cResult
{
public:

   cResult()
   {
      mResult = kRNoChange;
   }

   cResult(eResult x) : mResult(x)
   {
   }

   cResult(int x)
   {
      mResult = kRError;
      if (x == 0 || x == 3 || x == 127)
         mResult = (eResult)x;
   }

   cResult & operator += (const cResult & rhs)
   {
      if (isError() || rhs.isError())
         mResult = kRError;
      else if (isNOIMPL() || rhs.isNOIMPL())
         mResult = kRNotImplemented;
      else if (isNoChange() && rhs.isNoChange())
         mResult = kRNoChange;
      else
         mResult = kRSuccess;

      return *this;
   }

   operator eResult() const { return mResult; }
   bool isError() const { return mResult == kRError; }
   bool isNoChange() const { return mResult == kRNoChange; }
   bool isNOIMPL() const { return mResult == kRNotImplemented; }
private:
   eResult mResult;
};


#endif 