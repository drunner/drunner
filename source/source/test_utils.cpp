#include <Poco/String.h>

#include "catch/catch.h"
#include "utils.h"

TEST_CASE("split_in_args works", "[utils.h]") {
//   bool split_in_args(std::vector<std::string>& qargs, std::string command);

   SECTION("Test simple cases")
   {
      std::string command = "a ab  abc   abcd    abcde 'a ab  abc'";
      bool rval;
      std::vector<std::string> args;
      rval = utils::split_in_args(command, args);

      REQUIRE(rval);
      REQUIRE(args.size() == 6);
      REQUIRE(args[0].compare("a") == 0);
      REQUIRE(args[1].compare("ab") == 0);
      REQUIRE(args[2].compare("abc") == 0);
      REQUIRE(args[3].compare("abcd") == 0);
      REQUIRE(args[4].compare("abcde") == 0);
      REQUIRE(args[5].compare("'a ab  abc'") == 0);
   }

   //SECTION("Test false functions")
   //{
   //}
}

