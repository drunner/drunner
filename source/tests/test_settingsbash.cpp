#include "catch.h"
#include "settingsbash.h"
#include "utils.h"

TEST_CASE("read/write bool works","[settingsbash]") {
   params p(kLDEBUG);
   settingsbash sb(p,"/tmp/sbtest");

   SECTION("Test true functions")
   {
      sb_bool e("bool",true);
      REQUIRE(e.getBashLine().str()=="bool=yes");

      sb.setSetting(e);
      REQUIRE(sb.getBool("bool"));
   }

   SECTION("Test false functions")
   {
      sb_bool e("bool",false);
      REQUIRE(e.getBashLine().str() == "bool=no");

      sb.setSetting(e);
      REQUIRE_FALSE(sb.getBool("bool"));
   }

   SECTION("Test bunch'o'Stuff")
   {
      sb.setSetting(sb_string("ROOTPATH","/home/j"));
      sb.setSetting(sb_string("ROOTUTILIMAGE","drunner/install-rootutils"));
      sb.setSetting(sb_string("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF"));
      sb.setSetting(sb_string("DRUNNERINSTALLTIME",utils::getTime()));
      sb.setSetting(sb_bool("PULLIMAGES",true));

      REQUIRE( sb.getString("ROOTUTILIMAGE")=="drunner/install-rootutils" );
   }

}
