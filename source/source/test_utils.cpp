#include <Poco/String.h>

#include "catch/catch.h"
#include "utils.h"

TEST_CASE("split_in_args works", "[utils.h]") {
//   bool split_in_args(std::vector<std::string>& qargs, std::string command);

   SECTION("Test simple cases")
   {
      std::string command = R"EOF(     a ab  abc  " abcd"  "dog=\"pony club\""   dog="pony club"  "dog=pony club"  )EOF";
      CommandLine cl;
      utils::split_in_args(command, cl);

      //for (auto x : args)
      //   std::cout << ">> "<<x<< " <<" << std::endl;

      REQUIRE(cl.args.size() == 7);
      REQUIRE(cl.command.compare("a") == 0);
      REQUIRE(cl.args[0].compare("ab") == 0);
      REQUIRE(cl.args[1].compare("abc") == 0);
      REQUIRE(cl.args[2].compare(" abcd") == 0);
      REQUIRE(cl.args[3].compare(R"EOF(dog="pony club")EOF") == 0);
      REQUIRE(cl.args[4].compare(R"EOF(dog="pony)EOF") == 0);
      REQUIRE(cl.args[5].compare(R"EOF(club")EOF") == 0);
      REQUIRE(cl.args[6].compare(R"EOF(dog=pony club)EOF") == 0);
   }


   SECTION("Test More Complex Cases")
   {
      std::string command = R"EOF(     a ab  abc   abcd    abcde "a \"ab\"  abc"  abcdefg sponge="_"''"_"_dave"    )EOF";
      CommandLine cl;
      utils::split_in_args(command, cl);

      //for (auto x : args)
      //   std::cout << ">> "<<x<< " <<" << std::endl;

      REQUIRE(cl.args.size() == 7);
      REQUIRE(cl.command.compare("a") == 0);
      REQUIRE(cl.args[0].compare("ab") == 0);
      REQUIRE(cl.args[1].compare("abc") == 0);
      REQUIRE(cl.args[2].compare("abcd") == 0);
      REQUIRE(cl.args[3].compare("abcde") == 0);
      REQUIRE(cl.args[4].compare(R"EOF(a "ab"  abc)EOF") == 0);  //    "a \"ab\"  abc"   ->   a "ab"  abc
      REQUIRE(cl.args[5].compare("abcdefg") == 0);
      REQUIRE(cl.args[6].compare(R"EOF(sponge="_"''"_"_dave")EOF") == 0);
   }


   SECTION("Test base64 encoding.")
   {
      std::string s = "wiffle 231089Z&$hsdfava HJKFSAH adfsjf lwe89r yh32hoh2n3rnz zisdfoifywewq |)@(*#$)&%&^ .... ---- \"sdafasdf '";
      REQUIRE(utils::base64encode(s) != s);
      REQUIRE(utils::base64decode(utils::base64encode(s)) == s);
   }

   //SECTION("Test false functions")
   //{
   //}
}

