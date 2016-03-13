#ifndef __C_RESULT_H
#define __C_RESULT_H

#include "enums.h"

class cResult
{
public:
   cResult(int x) { mResult = kRError; if (x == 0 || x == 3) mResult = (eResult)x; }
   cResult(eResult x) : mResult(x) {}

   cResult & operator += (const cResult & rhs)
   {
      if (isError() || rhs.isError())
         mResult = kRError;
      else if (isNoChange() && rhs.isNoChange())
         mResult = kRNoChange;
      else
         mResult = kRSuccess;
      return *this;
   }
   operator eResult() const { return mResult; }
   bool isError() const { return mResult == kRError; }
   bool isNoChange() const { return mResult == kRNoChange; }
private:
   eResult mResult;
};


#endif 