#include <Poco/String.h>

#include "catch/catch.h"
#include "utils.h"
#include "dbackup.h"

void dbackup_incrementdata(std::vector<int> & testdata)
{
   testdata.insert(testdata.begin(), -1);
   for (auto & c : testdata)
   {
      c += 1;
   }
}

TEST_CASE("Test that dbackup works", "[dbackup.h]") {
   SECTION("Sanity check exp")
   {
      REQUIRE(fabs(2.71828182845904523536028747135266250 - exp(1)) < 1e-8);
   }

   SECTION("Test objective function")
   {
      //static double dbackup::objfn(double di, double i, double maxdays, double n);
      REQUIRE(fabs(2.86675 - dbackup::objfn(17, 5, 100, 10)) < 0.00001);
   }

   SECTION("Test droptest")
   {
      std::vector<int> td2 = { 0, 1, 2, 3, 5, 11, 19, 31, 36, 40, 51, 91 };
      double droptestresult = dbackup::droptest(td2, 0, 100, td2.size());
      REQUIRE(fabs(750.543 - droptestresult) < 0.001);
   }

   SECTION("Test refine algorithm - simple")
   {
      std::vector<int> testdata = { 0,1,2,4,10,18,30,35,39,50,90 };
      int maxdays = 100;
      int maxbackups = 10;
      int alwayskeep = 2;

      dbackup_incrementdata(testdata);

      // test our increment test routine is okay.
      std::vector<int> incremented = { 0,1,2,3,5,11,19,31,36,40,51,91 };
      REQUIRE(std::equal(testdata.begin(), testdata.end(), incremented.begin()));

      // test a single iteration with a too-long initial vector.
      unsigned int drop = dbackup::refine(testdata, maxdays, maxbackups, alwayskeep);
      REQUIRE(drop == 7);
      testdata.erase(testdata.begin() + drop);
      std::vector<int> testtarget1 = { 0,1,2,3,5,11,19,36,40,51,91 };
      REQUIRE(std::equal(testdata.begin(), testdata.end(), testtarget1.begin()));

      // test we drop another.
      drop = dbackup::refine(testdata, maxdays, maxbackups, alwayskeep);
      REQUIRE(drop == 3);
      std::vector<int> testtarget1b = { 0, 1, 2, 5, 11, 19, 36, 40, 51, 91 }; // we drop the 3 (index 3).
      testdata.erase(testdata.begin() + drop);
      REQUIRE(std::equal(testdata.begin(), testdata.end(), testtarget1b.begin()));

      // test we don't drop anymore (since we have 10 backups now).
      drop = dbackup::refine(testdata, maxdays, maxbackups, alwayskeep);
      REQUIRE(drop == testdata.size());
   }

   SECTION("Test refine algorithm - 200 iterations")
   {
      int maxdays = 100;
      int maxbackups = 10;
      int alwayskeep = 2;

      // test 200 iterations, adding one backup per day.
      std::vector<int> t = { 0, 1, 2, 4, 10, 18, 30, 39, 50, 90 };
      for (int i = 0; i < 200; ++i)
      {
         dbackup_incrementdata(t);
         unsigned int d = dbackup::refine(t, maxdays, maxbackups, alwayskeep);
         t.erase(t.begin() + d);
      }
      std::vector<int> testtarget2 = { 0, 1, 2, 5, 8, 18, 34, 44, 54, 83 };
      REQUIRE(std::equal(t.begin(), t.end(), testtarget2.begin()));
   }

   //SECTION("Test false functions")
   //{
   //}
}



