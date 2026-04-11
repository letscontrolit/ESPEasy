//
//    FILE: unit_test_001.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 2022-05-28
// PURPOSE: unit tests for the AS5600 library
//          https://github.com/RobTillaart/AS5600
//          https://github.com/Arduino-CI/arduino_ci/blob/master/REFERENCE.md
//

// supported assertions
// ----------------------------
// assertEqual(expected, actual);               // a == b
// assertNotEqual(unwanted, actual);            // a != b
// assertComparativeEquivalent(expected, actual);    // abs(a - b) == 0 or (!(a > b) && !(a < b))
// assertComparativeNotEquivalent(unwanted, actual); // abs(a - b) > 0  or ((a > b) || (a < b))
// assertLess(upperBound, actual);              // a < b
// assertMore(lowerBound, actual);              // a > b
// assertLessOrEqual(upperBound, actual);       // a <= b
// assertMoreOrEqual(lowerBound, actual);       // a >= b
// assertTrue(actual);
// assertFalse(actual);
// assertNull(actual);

// // special cases for floats
// assertEqualFloat(expected, actual, epsilon);    // fabs(a - b) <= epsilon
// assertNotEqualFloat(unwanted, actual, epsilon); // fabs(a - b) >= epsilon
// assertInfinity(actual);                         // isinf(a)
// assertNotInfinity(actual);                      // !isinf(a)
// assertNAN(arg);                                 // isnan(a)
// assertNotNAN(arg);                              // !isnan(a)


#include <ArduinoUnitTests.h>

#include "AS5600.h"
#include "Wire.h"


unittest_setup()
{
  fprintf(stderr, "AS5600_LIB_VERSION: %s\n", (char *) AS5600_LIB_VERSION);
}


unittest_teardown()
{
}


unittest(test_constants_base)
{
  assertEqual(0, AS5600_CLOCK_WISE);
  assertEqual(1, AS5600_COUNTERCLOCK_WISE);

  assertEqual(0, AS5600_MODE_DEGREES);
  assertEqual(1, AS5600_MODE_RADIANS);
  assertEqual(2, AS5600_MODE_RPM);

  assertEqualFloat(360.0/4096,    AS5600_RAW_TO_DEGREES, 0.0001);
  assertEqualFloat(4096/360.0,    AS5600_DEGREES_TO_RAW, 0.0001);
  assertEqualFloat((PI*2.0)/4096, AS5600_RAW_TO_RADIANS, 0.0001);
  assertEqualFloat(60.0/4096,     AS5600_RAW_TO_RPM,     0.0001);

  assertEqual(0x36, AS5600_DEFAULT_ADDRESS);
  assertEqual(0x40, AS5600L_DEFAULT_ADDRESS);

  assertEqual(255, AS5600_SW_DIRECTION_PIN);
}


unittest(test_constants_configuration)
{
  assertEqual(0, AS5600_OUTMODE_ANALOG_100);
  assertEqual(1, AS5600_OUTMODE_ANALOG_90);
  assertEqual(2, AS5600_OUTMODE_PWM);

  assertEqual(0, AS5600_POWERMODE_NOMINAL);
  assertEqual(1, AS5600_POWERMODE_LOW1);
  assertEqual(2, AS5600_POWERMODE_LOW2);
  assertEqual(3, AS5600_POWERMODE_LOW3);

  assertEqual(0, AS5600_PWM_115);
  assertEqual(1, AS5600_PWM_230);
  assertEqual(2, AS5600_PWM_460);
  assertEqual(3, AS5600_PWM_920);

  assertEqual(0, AS5600_HYST_OFF);
  assertEqual(1, AS5600_HYST_LSB1);
  assertEqual(2, AS5600_HYST_LSB2);
  assertEqual(3, AS5600_HYST_LSB3);

  assertEqual(0, AS5600_SLOW_FILT_16X);
  assertEqual(1, AS5600_SLOW_FILT_8X);
  assertEqual(2, AS5600_SLOW_FILT_4X);
  assertEqual(3, AS5600_SLOW_FILT_2X);

  assertEqual(0, AS5600_FAST_FILT_NONE);
  assertEqual(1, AS5600_FAST_FILT_LSB6);
  assertEqual(2, AS5600_FAST_FILT_LSB7);
  assertEqual(3, AS5600_FAST_FILT_LSB9);
  assertEqual(4, AS5600_FAST_FILT_LSB18);
  assertEqual(5, AS5600_FAST_FILT_LSB21);
  assertEqual(6, AS5600_FAST_FILT_LSB24);
  assertEqual(7, AS5600_FAST_FILT_LSB10);

  assertEqual(0, AS5600_WATCHDOG_OFF);
  assertEqual(1, AS5600_WATCHDOG_ON);
}


unittest(test_constructor)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin(4);
  assertTrue(as5600.isConnected());  //  keep CI happy

  AS5600L asl(0x40);
  asl.begin(5);
  assertTrue(asl.isConnected());     //  keep CI happy
}


unittest(test_address)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin(4);
  assertEqual(0x36, as5600.getAddress());

  AS5600L asl;
  as5600.begin(5);
  assertEqual(0x40, asl.getAddress());

  AS5600L asl2(0x41);
  asl2.begin(6);
  assertEqual(0x41, asl2.getAddress());
}


unittest(test_hardware_direction)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin(4);
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());

  as5600.setDirection();
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());

  as5600.setDirection(AS5600_COUNTERCLOCK_WISE);
  assertEqual(AS5600_COUNTERCLOCK_WISE, as5600.getDirection());

  as5600.setDirection(AS5600_CLOCK_WISE);
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());
}


unittest(test_software_direction)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin(255);
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());

  as5600.setDirection();
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());

  as5600.setDirection(AS5600_COUNTERCLOCK_WISE);
  assertEqual(AS5600_COUNTERCLOCK_WISE, as5600.getDirection());

  as5600.setDirection(AS5600_CLOCK_WISE);
  assertEqual(AS5600_CLOCK_WISE, as5600.getDirection());
}


unittest(test_offset_I)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin();

  for (int of = 0; of < 360; of += 40)
  {
    as5600.setOffset(of);
    assertEqualFloat(of, as5600.getOffset(), 0.05);
  }

  assertTrue(as5600.setOffset(-40.25));
  assertEqualFloat(319.75, as5600.getOffset(), 0.05);

  assertTrue(as5600.setOffset(-400.25));
  assertEqualFloat(319.75, as5600.getOffset(), 0.05);

  assertTrue(as5600.setOffset(753.15));
  assertEqualFloat(33.15, as5600.getOffset(), 0.05);

  assertFalse(as5600.setOffset(36000.1));
  assertFalse(as5600.setOffset(-36000.1));
}


unittest(test_offset_II)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin();

  as5600.setOffset(200);
  assertEqualFloat(200, as5600.getOffset(), 0.05);

  as5600.setOffset(30);
  assertEqualFloat(30, as5600.getOffset(), 0.05);

  as5600.setOffset(200);
  assertEqualFloat(200, as5600.getOffset(), 0.05);

  as5600.setOffset(-30);
  assertEqualFloat(330, as5600.getOffset(), 0.05);


  //  as cummulative error can be larger ==> 0.1 
  as5600.setOffset(200);
  assertEqualFloat(200, as5600.getOffset(), 0.1);

  as5600.increaseOffset(30);
  assertEqualFloat(230, as5600.getOffset(), 0.1);

  as5600.setOffset(200);
  assertEqualFloat(200, as5600.getOffset(), 0.1);

  as5600.increaseOffset(-30);
  assertEqualFloat(170, as5600.getOffset(), 0.1);

  as5600.setOffset(200);
  assertEqualFloat(200, as5600.getOffset(), 0.1);

  as5600.increaseOffset(-300);
  assertEqualFloat(260, as5600.getOffset(), 0.1);
}


unittest(test_failing_set_commands)
{
  AS5600 as5600;

  Wire.begin();

  as5600.begin();

  assertFalse(as5600.setZPosition(4096));
  assertFalse(as5600.setMPosition(4096));
  assertFalse(as5600.setMaxAngle(4096));

  assertFalse(as5600.setConfigure(0x4000));
  assertFalse(as5600.setPowerMode(4));
  assertFalse(as5600.setHysteresis(4));
  assertFalse(as5600.setOutputMode(3));
  assertFalse(as5600.setPWMFrequency(4));
  assertFalse(as5600.setSlowFilter(4));
  assertFalse(as5600.setFastFilter(8));
  assertFalse(as5600.setWatchDog(2));
}


//  FOR REMAINING ONE NEED A STUB


unittest_main()


//  -- END OF FILE --
