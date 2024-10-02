// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------

#include <stdio.h>
#include <gtest/gtest.h>
#include "rapidassist/environment.h"

int main(int argc, char **argv)
{
  //define default values for xml output report
  if (ra::environment::IsConfigurationDebug())
    ::testing::GTEST_FLAG(output) = "xml:anyrtttl_unittest.debug.xml";
  else
    ::testing::GTEST_FLAG(output) = "xml:anyrtttl_unittest.release.xml";

  ::testing::GTEST_FLAG(filter) = "*";
  ::testing::InitGoogleTest(&argc, argv);

  int wResult = RUN_ALL_TESTS(); //Find and run all tests

  return wResult; // returns 0 if all the tests are successful, or 1 otherwise
}
