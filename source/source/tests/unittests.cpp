#define CATCH_CONFIG_RUNNER
#include "catch/catch.h"

int UnitTest()
{
   int argc=0;
   const char *argv[1];
   argv[0]="blah";
   int result = Catch::Session().run( argc,argv);
   return result;
}
