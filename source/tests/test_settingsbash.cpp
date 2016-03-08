#include "catch.h"
#include "settingsbash.h"
#include "utils.h"

TEST_CASE("read/write bool works","[settingsbash]") {
   params p(kLDEBUG);
   settingsbash sb(p,"/tmp/sbtest");

   SECTION("Test true functions")
   {
      sbelement e("bool",true);
      REQUIRE(e.getBashLine()=="bool=yes");

      sb.setSetting(e);
      REQUIRE(sb.getBool("bool"));
   }

   SECTION("Test false functions")
   {
      sbelement e("bool",false);
      REQUIRE(e.getBashLine() == "bool=no");

      sb.setSetting(e);
      REQUIRE_FALSE(sb.getBool("bool"));
   }

   SECTION("Test bunch'o'Stuff")
   {
      sb.setSetting(sbelement("ROOTPATH","/home/j"));
      sb.setSetting(sbelement("ROOTUTILIMAGE","drunner/install-rootutils"));
      sb.setSetting(sbelement("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF"));
      sb.setSetting(sbelement("DRUNNERINSTALLTIME",utils::getTime()));
      sb.setSetting(sbelement("PULLIMAGES","yes"));

      REQUIRE( sb.getString("ROOTUTILIMAGE")=="drunner/install-rootutils" );
   }

}
