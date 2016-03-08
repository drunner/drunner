#include "catch.h"
#include "settingsbash.h"
#include "utils.h"

TEST_CASE("read/write bool works","[settingsbash]") {
   params p(kLDEBUG);
   settingsbash sb(p,"/tmp/sbtest");

   SECTION("Test true functions")
   {
      sbelement e("bool",true);
      sb.setSetting(e);
      REQUIRE(sb.getBool("bool")==true);
      REQUIRE(utils::stringisame(e.getBashLine(),"bool=yes"));
   }

   SECTION("Test false functions")
   {
      sbelement e("bool",false);
      sb.setSetting(e);
      REQUIRE(sb.getBool("bool")==false);
      REQUIRE(utils::stringisame(e.getBashLine(),"bool=no"));
   }

}
