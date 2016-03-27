#include "catch.h"
#include "settingsbash.h"
#include "utils.h"

TEST_CASE("read/write bool works","[settingsbash]") {
   //params p(kLDEBUG);
   settingsbash sb;
   
   SECTION("Test true functions")
   {
      sb_bool e("bool",true);
      REQUIRE(e.getBashLine().str()=="bool=yes");

      sb.setBool("bool",true);
      REQUIRE(sb.getBool("bool"));
   }

   SECTION("Test false functions")
   {
      sb_bool e("bool",false);
      REQUIRE(e.getBashLine().str() == "bool=no");

      sb.setBool("bool",false);
      REQUIRE_FALSE(sb.getBool("bool"));
   }

   SECTION("Test bunch'o'Stuff")
   {
      sb.setString("ROOTPATH","/home/j");
      sb.setString("ROOTUTILIMAGE","drunner/rootutils");
      sb.setString("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
      sb.setString("DRUNNERINSTALLTIME",utils::getTime());
      sb.setBool("PULLIMAGES",true);

      REQUIRE( sb.getString("ROOTUTILIMAGE")=="drunner/rootutils" );
   }

}
