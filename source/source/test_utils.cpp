#include <Poco/String.h>

#include "catch/catch.h"
#include "utils.h"

TEST_CASE("split_in_args works", "[utils.h]") {
//   bool split_in_args(std::vector<std::string>& qargs, std::string command);

   SECTION("Test simple cases")
   {
      std::string command = R"EOF(     a ab  abc  " abcd"  "dog=\"pony club\""   dog="pony club" )EOF";
      std::vector<std::string> args;
      utils::split_in_args(command, args);

      //for (auto x : args)
      //   std::cout << ">> "<<x<< " <<" << std::endl;

      REQUIRE(args.size() == 7);
      REQUIRE(args[0].compare("a") == 0);
      REQUIRE(args[1].compare("ab") == 0);
      REQUIRE(args[2].compare("abc") == 0);
      REQUIRE(args[3].compare(" abcd") == 0);
      REQUIRE(args[4].compare(R"EOF(dog="pony club")EOF") == 0);
      REQUIRE(args[5].compare(R"EOF(dog="pony)EOF") == 0);
      REQUIRE(args[6].compare(R"EOF(club")EOF") == 0);
   }


   SECTION("Test More Complex Cases")
   {
      std::string command = R"EOF(     a ab  abc   abcd    abcde "a \"ab\"  abc"  abcdefg sponge="_"''"_"_dave"    )EOF";
      std::vector<std::string> args;
      utils::split_in_args(command, args);

      //for (auto x : args)
      //   std::cout << ">> "<<x<< " <<" << std::endl;

      REQUIRE(args.size() == 8);
      REQUIRE(args[0].compare("a") == 0);
      REQUIRE(args[1].compare("ab") == 0);
      REQUIRE(args[2].compare("abc") == 0);
      REQUIRE(args[3].compare("abcd") == 0);
      REQUIRE(args[4].compare("abcde") == 0);
      REQUIRE(args[5].compare(R"EOF(a "ab"  abc)EOF") == 0);  //    "a \"ab\"  abc"   ->   a "ab"  abc
      REQUIRE(args[6].compare("abcdefg") == 0);
      REQUIRE(args[7].compare(R"EOF(sponge="_"''"_"_dave")EOF") == 0);
   }

   //SECTION("Test false functions")
   //{
   //}
}

