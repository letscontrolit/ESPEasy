// Copyright 2017-2020 David Conran

#include <string>
#include "ir_Samsung.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"


// General housekeeping
TEST(TestSamsung, Housekeeping) {
  ASSERT_EQ("SAMSUNG", typeToString(decode_type_t::SAMSUNG));
  ASSERT_EQ(decode_type_t::SAMSUNG, strToDecodeType("SAMSUNG"));
  ASSERT_FALSE(hasACState(decode_type_t::SAMSUNG));
  ASSERT_EQ(kSamsungBits, IRsend::defaultBits(decode_type_t::SAMSUNG));
}

// Tests for sendSAMSUNG().

// Test sending typical data only.
TEST(TestSendSamsung, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);  // Samsung TV Power On.
  EXPECT_EQ(
      "f38000d33"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040",
      irsend.outputStr());

  irsend.reset();
}

// Test sending with different repeats.
TEST(TestSendSamsung, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 1);  // 1 repeat.
  EXPECT_EQ(
      "f38000d33"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040",
      irsend.outputStr());
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 2);  // 2 repeats.
  EXPECT_EQ(
      "f38000d33"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040"
      "m4480s4480"
      "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
      "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
      "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
      "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
      "m560s560m560s1680m560s1680m560s560m560s47040",
      irsend.outputStr());
}

// Tests for encodeSAMSUNG().

TEST(TestEncodeSamsung, NormalEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0xFF, irsend.encodeSAMSUNG(0, 0));
  EXPECT_EQ(0x8080807F, irsend.encodeSAMSUNG(1, 1));
  EXPECT_EQ(0xF8F805FA, irsend.encodeSAMSUNG(0x1F, 0xA0));
  EXPECT_EQ(0xA0A0CC33, irsend.encodeSAMSUNG(0x05, 0x33));
  EXPECT_EQ(0xFFFFFF00, irsend.encodeSAMSUNG(0xFF, 0xFF));
  EXPECT_EQ(0xE0E09966, irsend.encodeSAMSUNG(0x07, 0x99));
}

// Tests for decodeSAMSUNG().

// Decode normal Samsung messages.
TEST(TestDecodeSamsung, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);

  // Synthesised Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x07, 0x99));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);

  // Synthesised Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x8080807F, irsend.capture.value);
  EXPECT_EQ(0x1, irsend.capture.address);
  EXPECT_EQ(0x1, irsend.capture.command);
}

// Decode normal repeated Samsung messages.
TEST(TestDecodeSamsung, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);
}

// Decode unsupported Samsung messages.
TEST(TestDecodeSamsung, DecodeWithNonStrictValues) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0x0);  // Illegal value Samsung 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);

  irsend.reset();
  irsend.sendSAMSUNG(0x12345678);  // Illegal value Samsung 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                   false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);
  EXPECT_EQ(0x48, irsend.capture.address);
  EXPECT_EQ(0x6A, irsend.capture.command);

  // Illegal over length (36-bit) message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0, 0), 36);
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    true));
  // Shouldn't pass if strict off and wrong expected bit size.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    false));
  // Re-decode with correct bit size.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, 36, true));
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, 36, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(36, irsend.capture.bits);
  EXPECT_EQ(0xFF, irsend.capture.value);  // We told it to expect 8 bits less.
  EXPECT_EQ(0x00, irsend.capture.address);
  EXPECT_EQ(0x00, irsend.capture.command);

  // Illegal under length (16-bit) message
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x0, 0x0), 16);
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // And it should fail when we expect more bits.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));

  // Should pass if strict off if we ask for correct nr. of bits sent.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, 16, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(16, irsend.capture.bits);
  EXPECT_EQ(0xFF, irsend.capture.value);  // We told it to expect 4 bits less.
  EXPECT_EQ(0x00, irsend.capture.address);
  EXPECT_EQ(0x00, irsend.capture.command);

  // Should fail as we are expecting less bits than there are.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, 12, false));
}

// Decode (non-standard) 64-bit messages.
// Decode unsupported Samsung messages.
TEST(TestDecodeSamsung, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal value & size Samsung 64-bit message.
  irsend.sendSAMSUNG(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    true));
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, 64, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
  EXPECT_EQ(0xFF, irsend.capture.address);
  EXPECT_EQ(0xFF, irsend.capture.command);
}

// Decode a 'real' example via GlobalCache
TEST(TestDecodeSamsung, DecodeGlobalCacheExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung TV Power On from Global Cache.
  uint16_t gc_test[71] = {38000, 1,  1,  172, 172, 22, 64, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  21,  22, 21, 22, 21, 22, 64,  22,
                          64,    22, 64, 22,  21,  22, 21, 22, 21, 22, 21,  22,
                          21,    22, 64, 22,  21,  22, 21, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  64,  22, 21, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  64,  22, 64, 22, 21, 22, 1820};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);
}

// Fail to decode a non-Samsung example via GlobalCache
TEST(TestDecodeSamsung, FailToDecodeNonSamsungExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Modified a few entries to unexpected values, based on previous test case.
  uint16_t gc_test[71] = {38000, 1,  1,  172, 172, 22, 64, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  21,  22, 11, 22, 21, 22, 128, 22,
                          64,    22, 64, 22,  21,  22, 21, 22, 21, 22, 21,  22,
                          21,    22, 64, 22,  21,  22, 21, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  64,  22, 21, 22, 64, 22, 64,  22,
                          21,    22, 21, 22,  64,  22, 64, 22, 21, 22, 1820};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture));
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kStartOffset, kSamsungBits,
                                    false));
}

// General housekeeping
TEST(TestSamsungAC, Housekeeping) {
  ASSERT_EQ("SAMSUNG_AC", typeToString(decode_type_t::SAMSUNG_AC));
  ASSERT_EQ(decode_type_t::SAMSUNG_AC, strToDecodeType("SAMSUNG_AC"));
  ASSERT_TRUE(hasACState(decode_type_t::SAMSUNG_AC));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::SAMSUNG_AC));
  ASSERT_EQ(kSamsungAcBits, IRsend::defaultBits(decode_type_t::SAMSUNG_AC));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::SAMSUNG_AC));
}

// Tests for sendSamsungAC().

// Test sending typical data only.
TEST(TestSendSamsungAC, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();
  uint8_t data[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  irsend.sendSamsungAC(data);
  EXPECT_EQ(
      "f38000d50"
      "m690s17844"
      "m3086s8864"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s436m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s2886"
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s1432m586s436m586s1432"
      "m586s1432m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s1432m586s436m586s1432m586s436m586s1432m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s100000",
      irsend.outputStr());
}

// Test sending extended data.
TEST(TestSendSamsungAC, SendExtendedData) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();
  // "Off" message.
  uint8_t data[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x80, 0x11, 0xC0};
  irsend.sendSamsungAC(data, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "f38000d50"
      "m690s17844"
      "m3086s8864"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s1432m586s436m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s1432m586s1432"
      "m586s2886"
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s1432m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s2886"
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s1432m586s1432m586s1432m586s1432m586s1432m586s1432m586s1432m586s1432"
      "m586s1432m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s1432"
      "m586s1432m586s436m586s436m586s436m586s1432m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s1432m586s1432"
      "m586s100000",
      irsend.outputStr());
}

// Tests for IRSamsungAc class.

TEST(TestIRSamsungAcClass, SetAndGetRaw) {
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x40, 0x11, 0xF0};
  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(expectedState);
  EXPECT_STATE_EQ(expectedState, ac.getRaw(), kSamsungAcBits);
  uint8_t extendedState[kSamsungAcExtendedStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x40, 0x11, 0xF0};
  ac.setRaw(extendedState, kSamsungAcExtendedStateLength);
  // We should NOT get the extended state back.
  EXPECT_STATE_EQ(expectedState, ac.getRaw(), kSamsungAcBits);
}

TEST(TestIRSamsungAcClass, SetAndGetPower) {
  IRSamsungAc ac(kGpioUnused);
  ac.on();
  EXPECT_TRUE(ac.getPower());
  ac.off();
  EXPECT_FALSE(ac.getPower());
  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());
  ac.setPower(false);
  EXPECT_FALSE(ac.getPower());
}

TEST(TestIRSamsungAcClass, SetAndGetSwing) {
  IRSamsungAc ac(kGpioUnused);

  // Vertical
  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());
  EXPECT_FALSE(ac.getSwingH());
  ac.setSwing(false);
  EXPECT_FALSE(ac.getSwing());
  EXPECT_FALSE(ac.getSwingH());
  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());
  EXPECT_FALSE(ac.getSwingH());
  // Horizontal
  ac.setSwingH(true);
  EXPECT_TRUE(ac.getSwing());
  EXPECT_TRUE(ac.getSwingH());
  ac.setSwingH(false);
  EXPECT_TRUE(ac.getSwing());
  EXPECT_FALSE(ac.getSwingH());
  ac.setSwingH(true);
  EXPECT_TRUE(ac.getSwing());
  EXPECT_TRUE(ac.getSwingH());

  ac.setSwing(false);
  EXPECT_FALSE(ac.getSwing());
  EXPECT_TRUE(ac.getSwingH());
  ac.setSwingH(false);
  EXPECT_FALSE(ac.getSwing());
  EXPECT_FALSE(ac.getSwingH());

  // Real examples from:
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/505#issuecomment-424036602
  const uint8_t expected_off[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};
  ac.setRaw(expected_off);
  EXPECT_FALSE(ac.getSwing());
  const uint8_t expected_on[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x80, 0x11, 0xF0};
  ac.setRaw(expected_on);
  EXPECT_TRUE(ac.getSwing());
}

TEST(TestIRSamsungAcClass, SetAndGetClean) {
  IRSamsungAc ac(kGpioUnused);
  ac.setClean(true);
  EXPECT_TRUE(ac.getClean());
  ac.setClean(false);
  EXPECT_FALSE(ac.getClean());
  ac.setClean(true);
  EXPECT_TRUE(ac.getClean());
}

TEST(TestIRSamsungAcClass, SetAndGetBeep) {
  IRSamsungAc ac(kGpioUnused);
  ac.setBeep(false);
  EXPECT_FALSE(ac.getBeep());
  ac.setBeep(true);
  EXPECT_TRUE(ac.getBeep());
  ac.setBeep(false);
  EXPECT_FALSE(ac.getBeep());
  ac.setBeep(true);
  EXPECT_TRUE(ac.getBeep());
}

TEST(TestIRSamsungAcClass, SetAndGetDisplay) {
  IRSamsungAc ac(0);
  ac.setDisplay(true);
  EXPECT_TRUE(ac.getDisplay());
  ac.setDisplay(false);
  EXPECT_FALSE(ac.getDisplay());
  ac.setDisplay(true);
  EXPECT_TRUE(ac.getDisplay());
}

TEST(TestIRSamsungAcClass, SetAndGetIon) {
  IRSamsungAc ac(0);
  ac.setIon(true);
  EXPECT_TRUE(ac.getIon());
  ac.setIon(false);
  EXPECT_FALSE(ac.getIon());
  ac.setIon(true);
  EXPECT_TRUE(ac.getIon());
}

TEST(TestIRSamsungAcClass, SetAndGetTemp) {
  IRSamsungAc ac(kGpioUnused);
  ac.setTemp(25);
  EXPECT_EQ(25, ac.getTemp());
  ac.setTemp(kSamsungAcMinTemp);
  EXPECT_EQ(kSamsungAcMinTemp, ac.getTemp());
  ac.setTemp(kSamsungAcMinTemp - 1);
  EXPECT_EQ(kSamsungAcMinTemp, ac.getTemp());
  ac.setTemp(kSamsungAcMaxTemp);
  EXPECT_EQ(kSamsungAcMaxTemp, ac.getTemp());
  ac.setTemp(kSamsungAcMaxTemp + 1);
  EXPECT_EQ(kSamsungAcMaxTemp, ac.getTemp());
}

TEST(TestIRSamsungAcClass, SetAndGetMode) {
  IRSamsungAc ac(kGpioUnused);
  ac.setMode(kSamsungAcCool);
  EXPECT_EQ(kSamsungAcCool, ac.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, ac.getFan());
  ac.setMode(kSamsungAcHeat);
  EXPECT_EQ(kSamsungAcHeat, ac.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, ac.getFan());
  ac.setMode(kSamsungAcAuto);
  EXPECT_EQ(kSamsungAcAuto, ac.getMode());
  EXPECT_EQ(kSamsungAcFanAuto2, ac.getFan());
  ac.setMode(kSamsungAcDry);
  EXPECT_EQ(kSamsungAcDry, ac.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, ac.getFan());
}

TEST(TestIRSamsungAcClass, SetAndGetFan) {
  IRSamsungAc ac(kGpioUnused);
  ac.setMode(kSamsungAcCool);  // Most fan modes avail in this setting.
  ac.setFan(kSamsungAcFanAuto);
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());
  ac.setFan(kSamsungAcFanLow);
  EXPECT_EQ(kSamsungAcFanLow, ac.getFan());
  ac.setFan(kSamsungAcFanAuto2);             // Not available in Cool mode.
  EXPECT_EQ(kSamsungAcFanLow, ac.getFan());  // Shouldn't change.
  ac.setMode(kSamsungAcAuto);                // Has special fan setting.
  EXPECT_EQ(kSamsungAcFanAuto2, ac.getFan());
  ac.setFan(kSamsungAcFanLow);  // Shouldn't be available in Auto mode.
  EXPECT_EQ(kSamsungAcFanAuto2, ac.getFan());
  ac.setMode(kSamsungAcHeat);  // Most fan modes avail in this setting.
  ac.setFan(kSamsungAcFanHigh);
  EXPECT_EQ(kSamsungAcFanHigh, ac.getFan());
}

TEST(TestIRSamsungAcClass, SetAndGetQuiet) {
  IRSamsungAc ac(0);
  ac.setQuiet(false);
  EXPECT_FALSE(ac.getQuiet());
  ac.setFan(kSamsungAcFanHigh);
  ac.setQuiet(true);
  EXPECT_TRUE(ac.getQuiet());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());
  ac.setQuiet(false);
  EXPECT_FALSE(ac.getQuiet());

  // Actual quiet on & off states from:
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/734#issuecomment-500071419
  uint8_t on[14] = {
      0x02, 0x82, 0x0F, 0x00, 0x00, 0x20, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};
  ac.setRaw(on, 14);
  EXPECT_TRUE(ac.getQuiet());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());
  uint8_t off[14] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};
  ac.setRaw(off, 14);
  EXPECT_FALSE(ac.getQuiet());
}


TEST(TestIRSamsungAcClass, SetAndGetPowerful) {
  IRSamsungAc ac(0);
  ac.setFan(kSamsungAcFanMed);
  ac.setPowerful(false);
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_EQ(kSamsungAcFanMed, ac.getFan());
  ac.setPowerful(true);
  EXPECT_TRUE(ac.getPowerful());
  EXPECT_EQ(kSamsungAcFanTurbo, ac.getFan());
  ac.setPowerful(false);
  EXPECT_FALSE(ac.getPowerful());

  // Breeze, Econo, and Powerful/Turbo are mutually exclusive.
  ac.setPowerful(true);
  EXPECT_TRUE(ac.getPowerful());
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_FALSE(ac.getEcono());
  ac.setBreeze(true);
  EXPECT_TRUE(ac.getBreeze());
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_FALSE(ac.getEcono());
  ac.setEcono(true);
  EXPECT_TRUE(ac.getEcono());
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_FALSE(ac.getPowerful());
  ac.setPowerful(true);
  EXPECT_TRUE(ac.getPowerful());
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_FALSE(ac.getEcono());

  // Actual powerful on & off states from:
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/734#issuecomment-500120270
  uint8_t on[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xA2, 0xFE, 0x77, 0x00, 0x1F, 0xF0};
  ac.setRaw(on, kSamsungAcStateLength);
  EXPECT_TRUE(ac.getPowerful());
  EXPECT_EQ(kSamsungAcFanTurbo, ac.getFan());
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 7 (Turbo), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: On, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());

  uint8_t off[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};
  ac.setRaw(off, kSamsungAcStateLength);
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_NE(kSamsungAcFanTurbo, ac.getFan());
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

TEST(TestIRSamsungAcClass, QuietAndPowerfulAreMutuallyExclusive) {
  IRSamsungAc ac(0);
  ac.setQuiet(false);
  ac.setPowerful(false);
  EXPECT_FALSE(ac.getQuiet());
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_NE(kSamsungAcFanTurbo, ac.getFan());
  ac.setQuiet(true);
  EXPECT_TRUE(ac.getQuiet());
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());
  ac.setPowerful(true);
  EXPECT_FALSE(ac.getQuiet());
  EXPECT_TRUE(ac.getPowerful());
  EXPECT_EQ(kSamsungAcFanTurbo, ac.getFan());
  ac.setQuiet(true);
  EXPECT_TRUE(ac.getQuiet());
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_NE(kSamsungAcFanTurbo, ac.getFan());
}

TEST(TestIRSamsungAcClass, SetAndGetEcono) {
  IRSamsungAc ac(kGpioUnused);
  ac.begin();
  EXPECT_FALSE(ac.getEcono());
  ac.setFan(kSamsungAcFanMed);
  ac.setSwing(false);
  ac.setEcono(true);
  EXPECT_TRUE(ac.getEcono());
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_FALSE(ac.getPowerful());
  EXPECT_TRUE(ac.getSwing());  // Econo turns on swingv.
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());  // And sets the fan to Auto.
  ac.setEcono(false);
  EXPECT_FALSE(ac.getEcono());
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_FALSE(ac.getPowerful());

  // Breeze, Econo, and Powerful/Turbo are mutually exclusive.
  // But that is tested in `SetAndGetPowerful`

  // Actual econo on state from:
  // https://cryptpad.fr/sheet/#/2/sheet/view/r9k8pmELYEjLyC71cD7EsThEYgKGLJygREZ5pVfNkS8/
  // Row: 33
  uint8_t on[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0xAE, 0x7F, 0x80, 0x11, 0xF0};
  ac.setRaw(on, kSamsungAcStateLength);
  EXPECT_TRUE(ac.getEcono());
  EXPECT_TRUE(ac.getSwing());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 24C, Fan: 0 (Auto), "
      "Swing(V): On, Swing(H): Off, Beep: -, Clean: -, "
      "Quiet: Off, Powerful: Off, Econo: On, Breeze: Off, "
      "Light: On, Ion: Off",
      ac.toString());
}

TEST(TestIRSamsungAcClass, ChecksumCalculation) {
  IRSamsungAc ac(kGpioUnused);

  const uint8_t originalstate[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  uint8_t examplestate[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  const uint8_t extendedstate[kSamsungAcExtendedStateLength] = {
      0x02, 0xA9, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xC9, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xF9, 0xCE, 0x71, 0xE0, 0x41, 0xC0};

  EXPECT_TRUE(IRSamsungAc::validChecksum(examplestate));

  examplestate[8] = 0x12;  // Set an incorrect checksum.
  EXPECT_FALSE(IRSamsungAc::validChecksum(examplestate));
  ac.setRaw(examplestate);
  // Extracting the state from the object should have a correct checksum.
  EXPECT_TRUE(IRSamsungAc::validChecksum(ac.getRaw()));
  EXPECT_STATE_EQ(originalstate, ac.getRaw(), kSamsungAcBits);
  examplestate[8] = 0x02;  // Restore old checksum value.

  // Change the state to force a different checksum.
  examplestate[11] = 0x01;
  EXPECT_FALSE(IRSamsungAc::validChecksum(examplestate));

  // Check an extended state is valid.
  EXPECT_TRUE(IRSamsungAc::validChecksum(extendedstate, 21));
}

TEST(TestIRSamsungAcClass, HumanReadable) {
  IRSamsungAc ac(kGpioUnused);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 2 (Low), "
      "Swing(V): On, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, Econo: Off, "
      "Breeze: Off, Light: On, Ion: Off",
      ac.toString());
  ac.setTemp(kSamsungAcMaxTemp);
  ac.setMode(kSamsungAcHeat);
  ac.off();
  ac.setFan(kSamsungAcFanHigh);
  ac.setSwing(false);
  ac.setBeep(true);
  ac.setClean(true);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Heat), Temp: 30C, Fan: 5 (High), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: Toggle, Clean: Toggle, Quiet: Off, Powerful: Off, Econo: Off, "
      "Breeze: Off, Light: On, Ion: Off",
      ac.toString());
  ac.setQuiet(true);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Heat), Temp: 30C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, Beep: Toggle, "
      "Clean: Toggle, Quiet: On, Powerful: Off, Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off",
      ac.toString());
  ac.setQuiet(false);
  ac.setPowerful(true);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Heat), Temp: 30C, Fan: 7 (Turbo), "
      "Swing(V): Off, Swing(H): Off, Beep: Toggle, "
      "Clean: Toggle, Quiet: Off, Powerful: On, Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off",
      ac.toString());
  ac.setIon(true);
  ac.setDisplay(false);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Heat), Temp: 30C, Fan: 7 (Turbo), "
      "Swing(V): Off, Swing(H): Off, Beep: Toggle, "
      "Clean: Toggle, Quiet: Off, Powerful: On, Econo: Off, Breeze: Off, "
      "Light: Off, Ion: On",
      ac.toString());
}

TEST(TestIRSamsungAcClass, GeneralConstruction) {
  IRSamsungAc ac(kGpioUnused);

  uint8_t OnCoolFAutoBOffCOffQOffT20Soff[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x40, 0x11, 0xF0};

  ac.setPower(true);
  ac.setMode(kSamsungAcCool);
  ac.setFan(kSamsungAcFanAuto);
  ac.setSwing(false);
  ac.setBeep(false);
  ac.setClean(false);
  ac.setQuiet(false);
  ac.setTemp(20);
  EXPECT_STATE_EQ(OnCoolFAutoBOffCOffQOffT20Soff, ac.getRaw(),
                  kSamsungAcBits);

  uint8_t OnHeatFAutoBOffCOffQOffT17Son[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x10, 0x41, 0xF0};
  ac.setPower(true);
  ac.setMode(kSamsungAcHeat);
  ac.setFan(kSamsungAcFanAuto);
  ac.setSwing(true);
  ac.setBeep(false);
  ac.setClean(false);
  ac.setQuiet(false);
  ac.setTemp(17);
  EXPECT_STATE_EQ(OnHeatFAutoBOffCOffQOffT17Son, ac.getRaw(),
                  kSamsungAcBits);
}
// Tests for decodeSamsungAC().

// Decode normal SamsungAC messages.
TEST(TestDecodeSamsungAC, SyntheticDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  // Synthesised Normal Samsung A/C message.
  irsend.sendSamsungAC(expectedState);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Decode a real Samsung A/C example from Issue #505
TEST(TestDecodeSamsungAC, DecodeRealExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung A/C example from Issue #505
  uint16_t rawData[233] = {
      690, 17844, 3084, 8864, 606, 406,  586, 1410, 580, 436,  570,  424,
      570, 426,   570,  404,  596, 418,  580, 416,  584, 410,  586,  1402,
      588, 408,   586,  410,  584, 1380, 610, 408,  586, 408,  586,  1404,
      586, 1404,  586,  1408, 594, 1396, 596, 1394, 602, 418,  582,  410,
      586, 408,   584,  408,  586, 408,  586, 410,  586, 408,  586,  410,
      586, 408,   586,  408,  586, 408,  586, 408,  586, 410,  584,  436,
      558, 436,   570,  424,  570, 424,  574, 420,  578, 416,  582,  412,
      586, 410,   586,  408,  584, 410,  586, 408,  586, 410,  584,  410,
      584, 408,   586,  408,  586, 410,  586, 408,  586, 412,  584,  436,
      556, 1410,  592,  1396, 602, 1390, 608, 1384, 608, 2886, 3086, 8858,
      610, 1380,  610,  410,  586, 408,  586, 410,  586, 408,  586,  410,
      586, 408,   586,  436,  558, 436,  554, 1410, 594, 426,  572,  422,
      578, 418,   582,  412,  586, 410,  584, 410,  586, 1380, 610,  1382,
      608, 1404,  586,  1404, 586, 408,  586, 1432, 558, 436,  554,  1414,
      590, 1398,  602,  418,  580, 414,  586, 410,  584, 1382, 606,  1382,
      608, 1382,  608,  408,  586, 408,  586, 408,  586, 408,  586,  410,
      584, 436,   560,  434,  570, 426,  566, 430,  568, 1400, 600,  416,
      584, 1406,  586,  410,  584, 1384, 606, 410,  586, 410,  584,  408,
      586, 408,   586,  408,  586, 408,  588, 410,  584, 1408, 590,  1400,
      592, 1398,  602,  1388, 612};
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 2 (Low), "
      "Swing(V): On, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

// Decode a real Samsung A/C example from Issue #505
TEST(TestDecodeSamsungAC, DecodeRealExample2) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung A/C example from Issue #505
  uint16_t rawData[233] = {
      668, 17834, 3092, 8862, 608, 410,  586, 1378, 612, 410,  584,  410,
      586, 410,   584,  410,  586, 408,  586, 408,  586, 410,  586,  1404,
      588, 436,   558,  436,  570, 1398, 592, 424,  576, 420,  578,  1388,
      608, 1382,  610,  1382, 608, 1380, 610, 1384, 606, 408,  586,  408,
      588, 408,   588,  408,  586, 436,  558, 436,  570, 424,  570,  426,
      572, 422,   578,  418,  582, 412,  586, 408,  586, 410,  584,  410,
      584, 410,   584,  410,  586, 410,  586, 408,  586, 408,  586,  408,
      586, 408,   586,  408,  586, 438,  558, 436,  568, 426,  570,  424,
      574, 422,   576,  418,  582, 414,  584, 410,  586, 410,  584,  410,
      586, 1380,  610,  1382, 608, 1404, 586, 1404, 602, 2872, 3096, 8878,
      582, 1432,  570,  426,  568, 426,  574, 420,  578, 416,  582,  412,
      586, 410,   584,  410,  586, 410,  586, 1382, 608, 410,  586,  410,
      586, 408,   586,  1404, 586, 1408, 582, 1410, 590, 428,  568,  1400,
      598, 1394,  606,  1382, 610, 1382, 608, 1378, 612, 1382, 608,  1384,
      606, 1404,  586,  408,  586, 414,  582, 436,  558, 1410, 590,  1422,
      576, 1390,  608,  410,  586, 410,  586, 410,  584, 410,  584,  410,
      586, 410,   586,  410,  584, 410,  586, 1404, 586, 1404, 588,  436,
      560, 436,   486,  510,  566, 1400, 598, 420,  576, 418,  582,  414,
      586, 410,   584,  410,  584, 410,  586, 410,  584, 1382, 608,  1384,
      606, 1384,  606,  1408, 600};
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 24C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

// Decode a real Samsung A/C example from:
// https://github.com/crankyoldgit/IRremoteESP8266/issues/505#issuecomment-424036602
TEST(TestDecodeSamsungAC, DecodePowerOnSample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[349] = {
      662, 17870, 3026, 8966, 540, 484,  514,  1482, 518, 482,  514,  482,
      518, 482,   516,  510,  490, 508,  490,  508,  572, 428,  576,  1398,
      542, 482,   514,  484,  514, 1460, 540,  482,  518, 482,  516,  1456,
      544, 1480,  518,  1480, 518, 1480, 518,  1484, 514, 510,  566,  432,
      576, 424,   574,  426,  540, 458,  516,  482,  516, 482,  516,  482,
      518, 480,   518,  482,  518, 482,  518,  482,  516, 482,  518,  482,
      516, 482,   518,  480,  516, 508,  492,  508,  490, 508,  572,  428,
      576, 422,   572,  428,  542, 456,  514,  484,  518, 480,  518,  480,
      518, 480,   516,  482,  516, 482,  520,  478,  518, 482,  518,  480,
      518, 1480,  518,  1480, 516, 1484, 594,  1428, 518, 2964, 3032, 8964,
      540, 1458,  542,  480,  518, 480,  520,  480,  518, 482,  520,  480,
      520, 478,   518,  480,  520, 478,  520,  1478, 522, 478,  518,  506,
      494, 1484,  594,  426,  574, 1400, 564,  1434, 540, 1454, 544,  1478,
      520, 1454,  544,  1458, 540, 480,  520,  480,  518, 480,  520,  480,
      518, 508,   490,  506,  568, 432,  572,  426,  576, 424,  544,  454,
      518, 480,   516,  482,  520, 478,  520,  478,  522, 478,  518,  480,
      520, 478,   520,  478,  520, 478,  520,  478,  520, 478,  518,  478,
      522, 506,   494,  504,  566, 432,  576,  424,  576, 424,  570,  428,
      518, 482,   518,  480,  518, 482,  520,  478,  520, 478,  520,  480,
      520, 478,   520,  478,  520, 2964, 3032, 8986, 520, 1478, 520,  506,
      492, 506,   492,  506,  568, 430,  574,  424,  546, 454,  516,  482,
      518, 482,   518,  1456, 544, 478,  546,  452,  520, 478,  544,  1432,
      542, 1478,  520,  1478, 520, 478,  520,  1482, 586, 1412, 598,  1400,
      564, 1432,  540,  1458, 544, 1454, 544,  1454, 544, 1456, 542,  480,
      518, 480,   520,  480,  520, 1462, 536,  1482, 588, 1410, 598,  424,
      572, 426,   542,  456,  518, 482,  520,  478,  520, 478,  522,  478,
      520, 478,   520,  1456, 542, 1458, 540,  478,  520, 478,  520,  478,
      520, 1482,  540,  482,  568, 430,  576,  424,  570, 428,  542,  458,
      518, 480,   520,  480,  520, 1454, 568,  1430, 566, 1432, 566,  1454,
      594};
  uint8_t expectedState[kSamsungAcExtendedStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};

  irsend.sendRaw(rawData, 349, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcExtendedBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 24C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

// Decode a real Samsung A/C example from:
// https://github.com/crankyoldgit/IRremoteESP8266/issues/505#issuecomment-424036602
TEST(TestDecodeSamsungAC, DecodePowerOffSample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[349] = {
      670, 17802, 3096, 8898, 602, 420,  580,  1418, 582, 418,  582,  416,
      582, 416,   584,  442,  550, 448,  568,  430,  570, 430,  576,  1396,
      600, 424,   546,  452,  578, 1394, 604,  1396, 600, 420,  580,  1398,
      602, 1416,  586,  1412, 582, 1420, 576,  1422, 592, 428,  574,  424,
      576, 422,   578,  420,  548, 452,  578,  420,  578, 420,  584,  416,
      580, 418,   580,  418,  582, 418,  580,  418,  582, 414,  584,  414,
      584, 416,   582,  418,  584, 442,  558,  442,  568, 430,  576,  424,
      578, 420,   576,  424,  576, 422,  580,  420,  584, 414,  584,  416,
      584, 414,   582,  418,  580, 418,  582,  416,  582, 416,  584,  414,
      584, 414,   586,  442,  554, 1420, 570,  1452, 578, 2884, 3120, 8898,
      596, 1400,  602,  422,  582, 418,  584,  414,  582, 416,  584,  414,
      584, 416,   582,  416,  584, 416,  584,  1410, 586, 414,  582,  444,
      556, 1420,  590,  432,  572, 1402, 602,  1396, 600, 1398, 606,  1414,
      582, 1394,  604,  1394, 604, 414,  584,  414,  586, 412,  586,  410,
      586, 442,   556,  440,  544, 456,  568,  430,  576, 424,  578,  420,
      578, 420,   576,  424,  584, 412,  586,  412,  586, 412,  584,  414,
      586, 412,   584,  414,  586, 412,  586,  412,  586, 414,  586,  412,
      584, 442,   558,  442,  558, 440,  566,  432,  574, 424,  578,  422,
      576, 422,   578,  420,  586, 414,  586,  414,  586, 412,  584,  414,
      586, 414,   586,  414,  586, 2902, 3096, 8900, 600, 1416, 586,  442,
      556, 442,   558,  440,  564, 434,  572,  428,  578, 420,  580,  420,
      578, 420,   584,  1392, 608, 414,  586,  414,  582, 414,  586,  412,
      586, 412,   586,  414,  584, 1394, 606,  1416, 580, 1418, 568,  1432,
      594, 1402,  602,  1398, 606, 1392, 606,  1390, 608, 1390, 608,  414,
      584, 414,   586,  414,  584, 1412, 586,  1398, 600, 1418, 590,  430,
      566, 432,   576,  422,  578, 420,  578,  422,  582, 416,  586,  414,
      586, 412,   584,  1390, 606, 1392, 608,  414,  586, 412,  584,  412,
      588, 1410,  586,  442,  558, 440,  568,  430,  566, 434,  574,  426,
      578, 420,   578,  420,  582, 416,  586,  412,  586, 1390, 608,  1390,
      608};

  uint8_t expectedState[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x80, 0x11, 0xC0};

  irsend.sendRaw(rawData, 349, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcExtendedBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: Off, Mode: 1 (Cool), Temp: 24C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

TEST(TestDecodeSamsungAC, DecodeHeatSample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[233] = {
      650, 16260, 3014, 8934, 534, 486,  508, 1478, 514, 484,  510,  486,
      508, 512,   484,  510,  562, 432,  572, 422,  540, 454,  514,  1452,
      534, 486,   510,  484,  510, 1456, 536, 484,  510, 484,  510,  1454,
      536, 1478,  512,  1476, 514, 1482, 508, 1482, 592, 428,  570,  424,
      538, 456,   508,  486,  510, 484,  512, 484,  510, 486,  510,  484,
      510, 484,   510,  486,  510, 484,  510, 484,  510, 484,  510,  484,
      510, 484,   510,  486,  508, 510,  484, 510,  568, 428,  570,  424,
      538, 458,   512,  482,  510, 486,  510, 484,  510, 484,  510,  484,
      510, 484,   510,  484,  510, 484,  510, 484,  510, 484,  510,  484,
      510, 1474,  516,  1502, 534, 1432, 594, 1398, 536, 2954, 3018, 8932,
      536, 1458,  532,  484,  510, 484,  512, 484,  510, 484,  510,  484,
      510, 484,   512,  484,  510, 484,  510, 1480, 508, 510,  530,  464,
      568, 426,   568,  426,  514, 480,  508, 486,  508, 1456, 534,  1478,
      514, 1452,  538,  1478, 512, 484,  510, 1456, 534, 486,  510,  1478,
      512, 1480,  570,  450,  570, 424,  540, 454,  512, 1452, 534,  1458,
      534, 1454,  536,  484,  512, 482,  512, 484,  512, 484,  512,  482,
      512, 1474,  514,  484,  512, 510,  486, 508,  534, 1430, 594,  426,
      512, 482,   512,  482,  510, 484,  512, 482,  512, 1452, 538,  482,
      512, 482,   512,  482,  510, 484,  510, 484,  510, 1478, 512,  1504,
      488, 1480,  560,  1454, 514};

  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x10, 0x41, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ(
      "Power: On, Mode: 4 (Heat), Temp: 17C, Fan: 0 (Auto), "
      "Swing(V): On, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

TEST(TestDecodeSamsungAC, DecodeCoolSample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[233] = {
      690, 17854, 3086, 8862, 610, 410,  584, 1382, 610, 410,  586,  408,
      586, 408,   586,  408,  588, 410,  584, 436,  558, 436,  570,  1396,
      598, 422,   576,  418,  580, 1384, 612, 410,  586, 410,  584,  1380,
      612, 1382,  608,  1384, 606, 1404, 586, 1404, 586, 436,  558,  438,
      566, 428,   568,  426,  570, 424,  576, 418,  578, 416,  584,  410,
      586, 408,   584,  410,  586, 408,  586, 408,  586, 410,  586,  408,
      586, 408,   588,  408,  586, 408,  588, 408,  586, 438,  558,  436,
      568, 426,   568,  428,  568, 426,  576, 418,  578, 416,  584,  412,
      584, 410,   586,  408,  586, 410,  586, 410,  584, 410,  586,  408,
      586, 1384,  606,  1402, 588, 1410, 580, 1410, 608, 2864, 3108, 8864,
      594, 1394,  604,  416,  584, 410,  586, 410,  586, 410,  586,  410,
      584, 410,   586,  410,  586, 408,  586, 1404, 588, 408,  586,  408,
      586, 436,   560,  1408, 592, 1400, 596, 1396, 600, 416,  584,  1382,
      608, 1380,  610,  1404, 586, 1384, 608, 1384, 606, 1402, 588,  1408,
      582, 1410,  564,  452,  568, 428,  572, 424,  576, 1414, 582,  1386,
      608, 1382,  608,  410,  584, 410,  584, 410,  586, 408,  586,  408,
      586, 408,   586,  408,  588, 1408, 582, 436,  540, 1426, 590,  428,
      574, 420,   578,  418,  580, 1384, 610, 410,  584, 410,  584,  410,
      584, 412,   584,  408,  586, 410,  586, 408,  586, 1404, 586,  1408,
      582, 1410,  562,  1426, 610};

  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x40, 0x11, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

TEST(TestDecodeSamsungAC, Issue604DecodeExtended) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t sendOff[349] = {
      642, 17730, 3056, 8916, 542, 448,  552,  1440, 552, 444,  552,  444,
      552, 444,   552,  440,  556, 440,  556,  440,  556, 1436, 552,  444,
      552, 444,   552,  1440, 548, 470,  526,  1464, 470, 526,  516,  1470,
      552, 1440,  552,  1440, 550, 1436, 556,  1434, 552, 444,  552,  444,
      552, 444,   552,  442,  552, 444,  546,  470,  526, 470,  526,  470,
      470, 524,   518,  474,  548, 448,  552,  444,  552, 442,  552,  444,
      550, 444,   552,  440,  556, 440,  556,  438,  556, 440,  552,  442,
      552, 444,   552,  442,  552, 444,  550,  470,  526, 466,  524,  470,
      470, 524,   470,  524,  518, 476,  548,  444,  552, 444,  556,  440,
      552, 442,   552,  444,  550, 1436, 556,  1436, 552, 2946, 3026, 8918,
      550, 1440,  552,  444,  548, 468,  526,  468,  470, 526,  470,  526,
      542, 452,   548,  444,  552, 1440, 550,  444,  552, 444,  552,  1436,
      556, 438,   552,  442,  552, 1440, 552,  1440, 552, 1460, 526,  1464,
      470, 1516,  548,  1444, 552, 444,  552,  442,  552, 444,  552,  438,
      556, 440,   556,  440,  552, 444,  552,  444,  552, 444,  552,  444,
      552, 444,   548,  448,  546, 470,  526,  468,  526, 470,  470,  524,
      520, 470,   548,  448,  552, 444,  552,  444,  552, 444,  552,  444,
      552, 438,   556,  440,  556, 438,  552,  444,  552, 442,  552,  444,
      552, 444,   552,  444,  552, 470,  526,  466,  526, 470,  470,  524,
      518, 478,   546,  448,  552, 2920, 3052, 8916, 552, 1434, 556,  440,
      556, 438,   552,  444,  552, 442,  552,  442,  552, 442,  552,  444,
      548, 1444,  548,  470,  526, 470,  522,  1466, 470, 1520, 548,  1438,
      556, 1436,  552,  1440, 552, 442,  552,  1436, 552, 1440, 552,  1440,
      552, 442,   552,  470,  522, 1466, 526,  1466, 470, 1516, 552,  444,
      552, 442,   552,  444,  552, 1436, 556,  1436, 552, 1440, 550,  444,
      552, 444,   552,  444,  548, 448,  546,  448,  548, 470,  526,  1462,
      474, 1518,  548,  1440, 552, 1438, 556,  440,  550, 444,  552,  444,
      552, 444,   552,  440,  556, 1436, 552,  444,  552, 444,  552,  444,
      550, 470,   522,  470,  524, 470,  470,  524,  518, 1474, 548,  1440,
      556};

  uint8_t expectedState[kSamsungAcExtendedStateLength] = {
      0x02, 0xA9, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xC9, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xF9, 0xCE, 0x71, 0xE0, 0x41, 0xC0};

  irsend.sendRaw(sendOff, 349, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcExtendedBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(kGpioUnused);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Heat), Temp: 30C, Fan: 0 (Auto), "
      "Swing(V): On, Swing(H): On, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

TEST(TestSendSamsung36, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendSamsung36(0);
  EXPECT_EQ(
      "f38000d50"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490"
      "m512s26880",
      irsend.outputStr());
  irsend.sendSamsung36(0x400E00FF);
  EXPECT_EQ(
      "f38000d50"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880",
      irsend.outputStr());
  irsend.reset();
}

// General housekeeping
TEST(TestSamsung36, Housekeeping) {
  ASSERT_EQ("SAMSUNG36", typeToString(decode_type_t::SAMSUNG36));
  ASSERT_EQ(decode_type_t::SAMSUNG36, strToDecodeType("SAMSUNG36"));
  ASSERT_FALSE(hasACState(decode_type_t::SAMSUNG36));
  ASSERT_EQ(kSamsung36Bits, IRsend::defaultBits(decode_type_t::SAMSUNG36));
}

// Test sending with different repeats.
TEST(TestSendSamsung36, SendWithRepeats) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendSamsung36(0x400E00FF, kSamsung36Bits, 1);  // 1 repeat.
  EXPECT_EQ(
      "f38000d50"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880",
      irsend.outputStr());
      irsend.sendSamsung36(0x400E00FF, kSamsung36Bits, 2);  // 2 repeats.
  EXPECT_EQ(
      "f38000d50"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880"
      "m4515s4438"
      "m512s490m512s490m512s490m512s490m512s490m512s1468m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s490m512s490m512s490m512s490"
      "m512s4438"
      "m512s1468m512s1468m512s1468m512s490m512s490m512s490m512s490m512s490"
      "m512s490m512s490m512s490m512s490m512s1468m512s1468m512s1468m512s1468"
      "m512s1468m512s1468m512s1468m512s1468"
      "m512s26880",
      irsend.outputStr());
}

TEST(TestDecodeSamsung36, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[77] = {
      4542, 4438, 568, 432, 562, 436, 536, 462, 538, 460, 538, 460, 564, 1434,
      564, 434, 534, 464, 536, 462, 562, 436, 536, 464, 564, 432, 538, 462, 536,
      464, 534, 464, 564, 420, 566, 4414, 538, 1462, 566, 1432, 562, 1436, 536,
      462, 564, 436, 562, 436, 560, 436, 562, 436, 562, 436, 560, 438, 536, 462,
      562, 436, 562, 1436, 562, 1434, 536, 1462, 564, 1434, 562, 1436, 564,
      1436, 534, 1462, 534, 1464, 536};  // UNKNOWN E4CD1208

  irsend.sendRaw(rawData, 77, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG36, irsend.capture.decode_type);
  EXPECT_EQ(kSamsung36Bits, irsend.capture.bits);
  EXPECT_EQ(0x400E00FF, irsend.capture.value);
  EXPECT_EQ(0xE00FF, irsend.capture.command);
  EXPECT_EQ(0x400, irsend.capture.address);
}

TEST(TestDecodeSamsung36, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();

  irsend.sendSamsung36(0x400E00FF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSamsung36(&irsend.capture));
  ASSERT_EQ(SAMSUNG36, irsend.capture.decode_type);
  EXPECT_EQ(kSamsung36Bits, irsend.capture.bits);
  EXPECT_EQ(0x400E00FF, irsend.capture.value);
  EXPECT_EQ(0xE00FF, irsend.capture.command);
  EXPECT_EQ(0x400, irsend.capture.address);
}

// https://github.com/crankyoldgit/IRremoteESP8266/issues/604
// This has been superceeded in a way by the ability to calculate extended msg
// checksums correctly. See #1484, #1538, & #1554
TEST(TestIRSamsungAcClass, Issue604SendPowerHack) {
  IRSamsungAc ac(0);
  ac.begin();
  ac.stateReset(false);  // Disable the initial forced sending of a power mesg.

  std::string freqduty = "f38000d50";

  std::string settings_section1 =
      "m690s17844"
      "m3086s8864"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s436m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s2886";
  std::string settings_section2 =
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s1432m586s1432"
      "m586s436m586s1432m586s1432m586s1432m586s436m586s1432m586s436m586s1432"
      "m586s1432m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s436"
      "m586s1432m586s436m586s436m586s1432m586s1432m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s100000";
  std::string extended_section =
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s1432m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s2886";
  std::string text = "Power: On, Mode: 1 (Cool), Temp: 23C, Fan: 4 (Med), "
                     "Swing(V): On, Swing(H): Off, "
                     "Beep: -, Clean: -, Quiet: Off, "
                     "Powerful: Off, Econo: Off, Breeze: Off, "
                     "Light: On, Ion: Off";
  // Don't do a setPower()/on()/off() as that will trigger the special message.
  // So it should only be the normal "settings" message.
  ac.setTemp(23);
  ac.setMode(kSamsungAcCool);
  ac.setFan(kSamsungAcFanMed);
  ac.send();
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + settings_section2,
            ac._irsend.outputStr());
  // Ensure the power state is changed by changing it and sending it.
  ac.off();
  ac.send();
  ac._irsend.reset();  // Clear the capture buffer.
  // Now trigger a special power message by using a power method.
  ac.on();
  ac.send();  // This should result in an extended message.
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + extended_section + settings_section2,
            ac._irsend.outputStr());
  ac._irsend.reset();  // Clear the capture buffer.
  // Subsequent sending should be just the "settings" message.
  ac.send();
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + settings_section2,
            ac._irsend.outputStr());
  ac._irsend.reset();  // Clear the capture buffer.
  ac.setPower(true);  // Note: The power state hasn't changed from previous.
  ac.send();  // This should result in a normal setting message.
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + settings_section2,
            ac._irsend.outputStr());

  ac._irsend.reset();  // Clear the capture buffer.
  ac.setPower(false);
  ac.setPower(true);  // Note: The power state hasn't changed from the last sent
  ac.send();  // This should result in a normal setting message.
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + settings_section2,
            ac._irsend.outputStr());

  ac.stateReset();  // Normal `stateReset` defaults to send the power message
                    // on first send.
  ac._irsend.reset();  // Clear the capture buffer.
  ac.setTemp(23);
  ac.setMode(kSamsungAcCool);
  ac.setFan(kSamsungAcFanMed);
  ac.send();
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + extended_section + settings_section2,
            ac._irsend.outputStr());
  ac._irsend.reset();  // Clear the capture buffer.
  ac.send();  // Subsequent send() should just be a settings message.
  EXPECT_EQ(text, ac.toString());
  EXPECT_EQ(freqduty + settings_section1 + settings_section2,
            ac._irsend.outputStr());
}

TEST(TestIRSamsungAcClass, toCommon) {
  IRSamsungAc ac(0);
  ac.setPower(true);
  ac.setMode(kSamsungAcCool);
  ac.setTemp(20);
  ac.setFan(kSamsungAcFanAuto);
  ac.setSwing(true);
  ac.setBeep(true);
  ac.setClean(true);
  ac.setQuiet(true);
  ac.setDisplay(true);
  ac.setIon(true);
  // Now test it.
  ASSERT_EQ(decode_type_t::SAMSUNG_AC, ac.toCommon().protocol);
  ASSERT_EQ(-1, ac.toCommon().model);
  ASSERT_TRUE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(20, ac.toCommon().degrees);
  ASSERT_EQ(stdAc::opmode_t::kCool, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kAuto, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kAuto, ac.toCommon().swingv);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_TRUE(ac.toCommon().quiet);
  ASSERT_TRUE(ac.toCommon().light);
  ASSERT_TRUE(ac.toCommon().filter);
  ASSERT_TRUE(ac.toCommon().clean);
  ASSERT_TRUE(ac.toCommon().beep);
  // Unsupported.
  ASSERT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);
}

TEST(TestDecodeSamsungAC, Issue734QuietSetting) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  // QUIET MODE ON data from:
  //  https://github.com/crankyoldgit/IRremoteESP8266/issues/734#issuecomment-499791618
  uint16_t quietOn[233] = {
      624, 17360, 3076, 8902, 520, 476, 520, 1472, 520, 476, 520, 474, 520, 476,
      520, 476, 520, 474, 522, 476, 520, 478, 518, 1476, 516, 500, 494, 502,
      548, 448, 546, 450, 544, 452, 522, 1468, 520, 1474, 520, 1472, 520, 1472,
      520, 1472, 520, 476, 520, 476, 518, 478, 516, 480, 516, 500, 496, 500,
      494, 502, 550, 446, 546, 450, 544, 452, 524, 472, 522, 474, 518, 476, 520,
      476, 520, 474, 522, 474, 520, 474, 520, 476, 520, 474, 520, 476, 518, 478,
      518, 480, 516, 480, 516, 502, 494, 502, 548, 1444, 524, 472, 522, 472,
      520, 474, 518, 478, 518, 476, 520, 476, 520, 1472, 520, 1470, 520, 1472,
      520, 1474, 516, 2980, 2998, 8980, 498, 1498, 548, 448, 526, 470, 544, 452,
      524, 472, 520, 474, 520, 476, 520, 476, 520, 476, 520, 1472, 520, 474,
      520, 476, 520, 1474, 518, 1476, 516, 1496, 496, 1498, 548, 446, 546, 1446,
      524, 1468, 518, 1474, 520, 1472, 520, 1472, 520, 1472, 520, 1474, 518,
      1476, 518, 480, 516, 500, 496, 528, 520, 1446, 544, 1446, 524, 1470, 518,
      476, 520, 476, 520, 474, 520, 476, 520, 474, 520, 476, 520, 474, 520, 476,
      520, 476, 518, 1476, 516, 482, 514, 502, 548, 448, 548, 1442, 544, 452,
      522, 474, 518, 476, 518, 476, 520, 476, 520, 474, 520, 476, 520, 1472,
      520, 1470, 522, 1474, 518, 1476, 536};

  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x82, 0x0F, 0x00, 0x00, 0x20, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};

  irsend.sendRaw(quietOn, 233, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(0);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: On, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());

  // Make sure the ac class state is in something wildly different first.
  ac.stateReset();
  ac.setPower(false);
  ac.setMode(kSamsungAcAuto);
  ac.setTemp(30);
  ac.setSwing(true);
  ac.setBeep(true);
  ac.setClean(true);
  ac.setQuiet(false);
  // See if we can build the state from scratch.
  ac.setPower(true);
  ac.setMode(kSamsungAcCool);
  ac.setTemp(16);
  ac.setSwing(false);
  ac.setBeep(false);
  ac.setClean(false);
  ac.setQuiet(true);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: On, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
  // Check it matches the known good/expected state.
  EXPECT_STATE_EQ(expectedState, ac.getRaw(), kSamsungAcBits);
}

TEST(TestDecodeSamsungAC, Issue734PowerfulOff) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  // 1st Powerful off data from:
  //  https://github.com/crankyoldgit/IRremoteESP8266/issues/734#issuecomment-500114580
  uint16_t powerfulOff[233] = {
      652, 17336, 3078, 8910, 562, 456, 546, 1448, 550, 446, 552, 444, 552, 444,
      550, 446, 550, 446, 552, 446, 550, 446, 552, 1440, 550, 446, 550, 446,
      550, 1470, 478, 518, 502, 492, 536, 1458, 542, 1450, 552, 1440, 552, 1442,
      552, 1442, 550, 446, 550, 446, 550, 446, 552, 444, 550, 446, 550, 446,
      550, 472, 524, 472, 480, 516, 510, 488, 538, 458, 542, 452, 548, 448, 550,
      446, 550, 446, 550, 444, 552, 444, 552, 444, 552, 444, 552, 444, 552, 444,
      552, 444, 550, 446, 550, 446, 550, 472, 524, 472, 482, 514, 510, 486, 536,
      460, 542, 454, 546, 450, 550, 446, 552, 1442, 552, 1442, 550, 1442, 552,
      1440, 508, 2994, 3030, 8932, 552, 1638, 450,  // <= (was 356)
      // Above hack due to poor data.
      470, 526, 470, 506, 492, 510,
      486, 542, 454, 544, 450, 550, 446, 554, 444, 550, 1442, 550, 444, 550,
      446, 550, 1442, 552, 1440, 550, 1442, 550, 1470, 524, 470, 480, 1512, 512,
      1480, 546, 1448, 550, 1442, 552, 1442, 552, 1442, 550, 1440, 552, 1440,
      552, 446, 550, 444, 552, 444, 550, 1468, 484, 1510, 512, 1482, 544, 452,
      550, 446, 552, 442, 554, 444, 552, 444, 554, 442, 554, 442, 552, 444, 554,
      442, 554, 1440, 552, 444, 554, 442, 554, 468, 528, 1466, 508, 488, 512,
      484, 544, 450, 550, 446, 554, 442, 556, 442, 554, 442, 554, 1438, 554,
      1438, 554, 1438, 554, 1438, 562};  // UNKNOWN 7B551B62};

  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};

  irsend.sendRaw(powerfulOff, 233, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSamsungAC(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc ac(0);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 16C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

// Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1062
TEST(TestIRSamsungAcClass, SetAndGetBreeze) {
  IRSamsungAc ac(kGpioUnused);
  ac.setFan(kSamsungAcFanMed);
  ac.setBreeze(false);
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_EQ(kSamsungAcFanMed, ac.getFan());
  ac.setBreeze(true);
  EXPECT_TRUE(ac.getBreeze());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());  // Breeze sets fan to auto.
  ac.setBreeze(false);
  EXPECT_FALSE(ac.getBreeze());
  EXPECT_EQ(kSamsungAcFanAuto, ac.getFan());

  // Breeze and Powerful/Turbo are mutually exclusive.
  ac.setBreeze(true);
  EXPECT_TRUE(ac.getBreeze());
  ac.setPowerful(true);
  EXPECT_FALSE(ac.getBreeze());

  // Check against real messages.
  // MODE FAN, 24C WINDFREE ON
  const uint8_t on[14] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0, 0x01, 0xB2, 0xFE, 0x7B, 0x80,
      0x31, 0xF0};
  ac.setRaw(on);
  ASSERT_TRUE(ac.getBreeze());
  EXPECT_EQ(
      "Power: On, Mode: 3 (Fan), Temp: 24C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: On, Light: On, Ion: Off",
      ac.toString());
  // MODE FAN, 24C WINDFREE OFF, FAN = LOW
  const uint8_t off[14] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0, 0x01, 0xC2, 0xFE, 0x71, 0x80,
      0x35, 0xF0};
  ac.setRaw(off);
  ASSERT_FALSE(ac.getBreeze());
  EXPECT_EQ(
      "Power: On, Mode: 3 (Fan), Temp: 24C, Fan: 2 (Low), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off",
      ac.toString());
}

TEST(TestDecodeSamsungAC, Issue1227VeryPoorSignal) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  // Data from row 5 of:
  // https://cryptpad.fr/sheet/#/2/sheet/view/e1PSfhwjfTGCAPbse4h28RjvvG+YCiLgH3SxhxTNFQQ/
  uint16_t rawData[233] = {
      600, 17900,
      2962, 8986,
      472, 536, 454, 1542, 440, 532, 462, 530, 484, 508, 490, 500, 496, 496,
      496, 494, 500, 500, 490, 1512, 470, 506, 492, 502, 488, 1490, 498, 522,
      468, 526, 468, 1516, 466, 1544, 438, 1546, 446, 1526, 484, 1516, 470, 512,
      480, 520, 474, 510, 478, 522, 468, 526, 466, 532, 460, 528, 466, 528, 464,
      536, 460, 552, 436, 560, 430, 562, 432, 564, 430, 558, 438, 556, 450, 544,
      458, 536, 462, 536, 454, 530, 462, 530, 462, 536, 458, 534, 460, 532, 458,
      534, 462, 546, 446, 556, 434, 548, 444, 566, 428, 564, 430, 560, 428, 566,
      432, 562, 432, 1568, 436, 1554, 430, 1562, 426, 1554,
      434, 3060,
      2902, 9020,
      432, 1550, 434, 548, 444, 562, 434, 564, 422, 568, 430, 566, 420, 572,
      426, 574, 422, 560, 434, 1574, 432, 542, 448, 548, 444, 1556, 436, 1552,
      426, 1544, 440, 1558, 428, 564, 434, 1560, 428, 1578, 408, 1582, 408,
      1582, 412, 1570, 430, 1546, 444, 1548, 442, 1548, 436, 542, 448, 540, 440,
      556, 448, 554, 434, 1566, 426, 1588, 402, 570, 422, 568, 426, 582, 414,
      568, 426, 570, 424, 1576, 426, 548, 442, 562, 426, 568, 430, 1564, 424,
      566, 416, 558, 438, 578, 410, 578, 428, 566, 416, 576, 410, 1604, 394,
      592, 392, 604, 402, 578, 416, 590, 404, 1586, 404, 1584, 408, 1578, 412,
      1572, 412};  // UNKNOWN 1C646112

  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xF2, 0xFE, 0x61, 0x10, 0x81, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();
  // Data is so bad we need to tweak the tolerance massively to get a match.
  irrecv.setTolerance(40);
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 0 (Auto), Temp: 17C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: Off, Ion: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

TEST(TestIRSamsungAcClass, SectionChecksums) {
  // Normal (14 bytes)
  const uint8_t on[14] = {
      0x02, 0x82, 0x0F, 0x00, 0x00, 0x20, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};
  EXPECT_EQ(0xF8, IRSamsungAc::calcSectionChecksum(on));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(on),
            IRSamsungAc::calcSectionChecksum(on));
  EXPECT_EQ(0xEF, IRSamsungAc::calcSectionChecksum(on + 7));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(on + 7),
            IRSamsungAc::calcSectionChecksum(on + 7));
  const uint8_t off[14] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xF2, 0xFE, 0x71, 0x00, 0x11, 0xF0};
  EXPECT_EQ(0xF9, IRSamsungAc::calcSectionChecksum(off));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(off),
            IRSamsungAc::calcSectionChecksum(off));
  EXPECT_EQ(0xEF, IRSamsungAc::calcSectionChecksum(off + 7));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(off + 7),
            IRSamsungAc::calcSectionChecksum(off + 7));

  // Extended (21 bytes)
  const uint8_t extended_on[kSamsungAcExtendedStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};
  EXPECT_EQ(0xF9, IRSamsungAc::calcSectionChecksum(extended_on));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_on),
            IRSamsungAc::calcSectionChecksum(extended_on));
  EXPECT_EQ(0xFD, IRSamsungAc::calcSectionChecksum(extended_on + 7));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_on + 7),
            IRSamsungAc::calcSectionChecksum(extended_on + 7));
  EXPECT_EQ(0xEE, IRSamsungAc::calcSectionChecksum(extended_on + 14));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_on + 14),
            IRSamsungAc::calcSectionChecksum(extended_on + 14));
  const uint8_t extended_off[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x80, 0x11, 0xC0};
  EXPECT_EQ(0xFB, IRSamsungAc::calcSectionChecksum(extended_off));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_off),
            IRSamsungAc::calcSectionChecksum(extended_off));
  EXPECT_EQ(0xFD, IRSamsungAc::calcSectionChecksum(extended_off + 7));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_off + 7),
            IRSamsungAc::calcSectionChecksum(extended_off + 7));
  EXPECT_EQ(0xF0, IRSamsungAc::calcSectionChecksum(extended_off + 14));
  EXPECT_EQ(IRSamsungAc::getSectionChecksum(extended_off + 14),
            IRSamsungAc::calcSectionChecksum(extended_off + 14));
}

TEST(TestIRSamsungAcClass, Issue1648) {
  IRSamsungAc ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  const uint8_t onState[kSamsungAcExtendedStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xC2, 0xFE, 0x71, 0x90, 0x15, 0xF0};
  const String onText = "Power: On, Mode: 1 (Cool), Temp: 25C, Fan: 2 (Low), "
                        "Swing(V): Off, Swing(H): Off, "
                        "Beep: -, Clean: -, Quiet: Off, "
                        "Powerful: Off, Econo: Off, Breeze: Off, "
                        "Light: On, Ion: Off";
  const uint8_t extended_offState[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x90, 0x15, 0xC0};
  const uint8_t short_offState[kSamsungAcStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xE2, 0xFE, 0x71, 0x90, 0x15, 0xC0};
  const String offText = "Power: Off, Mode: 1 (Cool), Temp: 25C, Fan: 2 (Low), "
                         "Swing(V): Off, Swing(H): Off, "
                         "Beep: -, Clean: -, Quiet: Off, "
                         "Powerful: Off, Econo: Off, Breeze: Off, "
                         "Light: On, Ion: Off";
  const uint8_t coolState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xC2, 0xFE, 0x71, 0x90, 0x15, 0xF0};

  // "setup()"" from provided code.
  ac.begin();  // User code
  ac.off();  // User code
  ac.setFan(kSamsungAcFanLow);  // User code
  ac.setMode(kSamsungAcCool);  // User code
  ac.setTemp(25);  // User code
  ac.setSwing(false);  // User code

  // Go through "loop()" from provided code.
  for (uint8_t i = 0; i < 2; i++) {
    ac.on();  // User code
    ac.send();  // User code

    // Verify what was sent.
    ac._irsend.makeDecodeResult();
    EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
    EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
    EXPECT_EQ(kSamsungAcExtendedBits, ac._irsend.capture.bits);
    EXPECT_STATE_EQ(onState, ac._irsend.capture.state, ac._irsend.capture.bits);
    EXPECT_EQ(onText, IRAcUtils::resultAcToString(&ac._irsend.capture));
    EXPECT_TRUE(ac._lastsentpowerstate);
    ac._irsend.reset();

    ac.setMode(kSamsungAcCool);  // User code
    ac.send();  // User code

    // Verify what was sent.
    ac._irsend.makeDecodeResult();
    EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
    EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
    EXPECT_EQ(kSamsungAcBits, ac._irsend.capture.bits);
    EXPECT_STATE_EQ(coolState, ac._irsend.capture.state,
                   ac._irsend.capture.bits);
    EXPECT_EQ(onText, IRAcUtils::resultAcToString(&ac._irsend.capture));
    ac._irsend.reset();
    EXPECT_TRUE(ac._lastsentpowerstate);
    EXPECT_FALSE(ac._forceextended);

    ac.off();  // User code
    ac.send();  // User code

    // Verify what was sent.
    ac._irsend.makeDecodeResult();
    EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
    EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
    EXPECT_EQ(kSamsungAcExtendedBits, ac._irsend.capture.bits);
    EXPECT_STATE_EQ(extended_offState, ac._irsend.capture.state,
                    ac._irsend.capture.bits);
    EXPECT_EQ(offText, IRAcUtils::resultAcToString(&ac._irsend.capture));
    EXPECT_FALSE(ac._lastsentpowerstate);
    ac._irsend.reset();

    ac.off();  // User code
    ac.send();  // User code

    // Verify what was sent.
    ac._irsend.makeDecodeResult();
    EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
    EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
    EXPECT_EQ(kSamsungAcBits, ac._irsend.capture.bits);
    EXPECT_STATE_EQ(short_offState, ac._irsend.capture.state,
                    ac._irsend.capture.bits);
    EXPECT_EQ(offText, IRAcUtils::resultAcToString(&ac._irsend.capture));
    EXPECT_FALSE(ac._lastsentpowerstate);
    ac._irsend.reset();
    // End of "loop()" code.
  }

  // Data from https://github.com/crankyoldgit/IRremoteESP8266/issues/1648#issuecomment-950822399
  const uint8_t expectedState[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x12, 0xAF, 0x71, 0x80, 0x15, 0xC0};
  const String expectedText = "Power: Off, Mode: 1 (Cool), Temp: 24C, "
                              "Fan: 2 (Low), Swing(V): On, Swing(H): Off, "
                              "Beep: -, Clean: -, "
                              "Quiet: Off, Powerful: Off, Econo: Off, "
                              "Breeze: Off, Light: On, Ion: Off";

  ac.stateReset();
  ac.setRaw(expectedState, kSamsungAcExtendedStateLength);
  EXPECT_EQ(expectedText, ac.toString());

  // Try to generate the same message.
  ac.stateReset();
  ac.off();
  ac.setMode(kSamsungAcCool);
  ac.setTemp(24);
  ac.setFan(kSamsungAcFanLow);
  ac.setSwing(true);
  ac.setBeep(false);
  ac.setClean(false);
  ac.setQuiet(false);
  ac.setPowerful(false);
  ac.setBreeze(false);
  ac.setDisplay(true);
  ac.setIon(false);
  EXPECT_EQ(expectedText, ac.toString());

  ac.send();
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcExtendedBits, ac._irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, ac._irsend.capture.state,
                  ac._irsend.capture.bits);
  EXPECT_EQ(expectedText, IRAcUtils::resultAcToString(&ac._irsend.capture));
  ac._irsend.reset();
}

TEST(TestIRSamsungAcClass, Timers) {
  IRSamsungAc ac(kGpioUnused);
  ac.begin();

  // https://github.com/crankyoldgit/IRremoteESP8266/issues/1277#issuecomment-961836703
  const uint8_t on_timer_30m[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xA2, 0x0F, 0x30, 0x00, 0x02, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x40, 0x11, 0xC0};
  const uint8_t off_timer_1h_on_timer_10m[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0x92, 0x8F, 0x10, 0x00, 0x06, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x40, 0x11, 0xC0};
  const uint8_t off_timer_1h_on_timer_0m[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xA2, 0x8F, 0x00, 0x00, 0x06, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x40, 0x11, 0xC0};

  ac.setRaw(on_timer_30m, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: Off, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, Light: On, Ion: Off, On Timer: 00:30",
      ac.toString());
  ac.setRaw(off_timer_1h_on_timer_10m, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: Off, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off, On Timer: 00:10, Off Timer: 01:00",
      ac.toString());
  ac.setRaw(off_timer_1h_on_timer_0m, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: Off, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off, On Timer: 00:00, Off Timer: 01:00",
      ac.toString());

  // https://cryptpad.fr/sheet/#/2/sheet/view/r9k8pmELYEjLyC71cD7EsThEYgKGLJygREZ5pVfNkS8/
  // Row 155
  const uint8_t off_timer_11h_on_timer_6h[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0x62, 0x8F, 0x05, 0x03, 0x06, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x40, 0x11, 0xC0};
  ac.setRaw(off_timer_11h_on_timer_6h, kSamsungAcExtendedStateLength);
  EXPECT_EQ(
      "Power: Off, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off, On Timer: 06:00, Off Timer: 11:00",
      ac.toString());

  ac.stateReset(false);
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(0);
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOffTimer(0);
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  // On Timer only
  ac.setOnTimer(30);
  EXPECT_EQ(30, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(90);  // 1h30m
  EXPECT_EQ(90, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(85);  // 1h25m -> 1h20m
  EXPECT_EQ(80, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(23 * 60 + 59);  // 23:59
  EXPECT_EQ(23 * 60 + 50, ac.getOnTimer());  // 23:50
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(24 * 60 + 30);  // 24:30
  EXPECT_EQ(24 * 60, ac.getOnTimer());  // 24:00 (Max)
  EXPECT_EQ(0, ac.getOffTimer());

  // Off Timer only
  ac.setOnTimer(0);
  ac.setOffTimer(0);
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOffTimer(30);
  EXPECT_EQ(30, ac.getOffTimer());
  EXPECT_EQ(0, ac.getOnTimer());

  ac.setOffTimer(90);  // 1h30m
  EXPECT_EQ(90, ac.getOffTimer());
  EXPECT_EQ(0, ac.getOnTimer());

  ac.setOffTimer(85);  // 1h25m -> 1h20m
  EXPECT_EQ(80, ac.getOffTimer());
  EXPECT_EQ(0, ac.getOnTimer());

  ac.setOffTimer(23 * 60 + 59);  // 23:59
  EXPECT_EQ(23 * 60 + 50, ac.getOffTimer());  // 23:50
  EXPECT_EQ(0, ac.getOnTimer());

  ac.setOffTimer(24 * 60 + 30);  // 24:30
  EXPECT_EQ(24 * 60, ac.getOffTimer());  // 24:00 (Max)
  EXPECT_EQ(0, ac.getOnTimer());

  // Both Timers
  ac.setOnTimer(24 * 60);  // 24:00
  EXPECT_EQ(24 * 60, ac.getOnTimer());  // 24:00 (Max)
  EXPECT_EQ(24 * 60, ac.getOffTimer());  // 24:00 (Max)

  ac.setOnTimer(1 * 60 + 30);  // 1:30
  ac.setOffTimer(11 * 60);  // 11:00
  EXPECT_EQ(1 * 60 + 30, ac.getOnTimer());
  EXPECT_EQ(11 * 60, ac.getOffTimer());
}

TEST(TestIRSamsungAcClass, Sleep) {
  IRSamsungAc ac(kGpioUnused);
  ac.begin();

  // https://cryptpad.fr/sheet/#/2/sheet/view/r9k8pmELYEjLyC71cD7EsThEYgKGLJygREZ5pVfNkS8/
  const uint8_t sleep_8h[kSamsungAcExtendedStateLength] = {
      0x02, 0x82, 0x0F, 0x00, 0x00, 0x10, 0xF0,
      0x01, 0xA2, 0x0F, 0x04, 0x00, 0x0C, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x40, 0x11, 0xF0};
  ac.setRaw(sleep_8h, kSamsungAcExtendedStateLength);
  EXPECT_EQ(8 * 60, ac.getSleepTimer());
  EXPECT_EQ(0, ac.getOffTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(
      "Power: On, Mode: 1 (Cool), Temp: 20C, Fan: 0 (Auto), "
      "Swing(V): Off, Swing(H): Off, "
      "Beep: -, Clean: -, Quiet: Off, Powerful: Off, "
      "Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off, Sleep Timer: 08:00",
      ac.toString());

  ac.stateReset(false);
  EXPECT_EQ(0, ac.getSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setOnTimer(60);
  ac.setOffTimer(90);
  EXPECT_EQ(0, ac.getSleepTimer());
  EXPECT_EQ(60, ac.getOnTimer());
  EXPECT_EQ(90, ac.getOffTimer());

  ac.setSleepTimer(120);
  EXPECT_EQ(120, ac.getSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffTimer());

  ac.setSleepTimer(24 * 60 + 31);  // 24h31m
  EXPECT_EQ(24 * 60, ac.getSleepTimer());  // 24h (Max)

  ac.setSleepTimer(35);  // 45m
  EXPECT_EQ(30, ac.getSleepTimer());  // 30m (Only stored in 10m increments).

  ac.setOnTimer(60);  // Seting an On Timer should clear the sleep setting.
  EXPECT_EQ(0, ac.getSleepTimer());
  EXPECT_EQ(60, ac.getOnTimer());

  ac.setSleepTimer(120);
  ac.setOffTimer(90);  // Setting an Off Timer will clear the sleep setting.
  EXPECT_EQ(0, ac.getSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(90, ac.getOffTimer());
}

TEST(TestIRSamsungAcClass, BuildKnownSleepSate) {
  // For https://github.com/crankyoldgit/IRremoteESP8266/issues/1277#issuecomment-965047193
  IRSamsungAc ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  ac.begin();
  const uint8_t expectedState[kSamsungAcExtendedStateLength] = {
      0x02, 0x82, 0x0F, 0x00, 0x00, 0x10, 0xF0,
      0x01, 0xA2, 0x0F, 0x04, 0x00, 0x0C, 0x00,
      0x01, 0xD2, 0xFE, 0x71, 0x50, 0x41, 0xF0};
  const char expectedStr[] = "Power: On, Mode: 4 (Heat), Temp: 21C, "
      "Fan: 0 (Auto), Swing(V): Off, Swing(H): Off, Beep: -, Clean: -, "
      "Quiet: Off, Powerful: Off, Econo: Off, Breeze: Off, "
      "Light: On, Ion: Off, Sleep Timer: 08:00";
  ac.setPower(true);
  ac.setMode(kSamsungAcHeat);
  ac.setTemp(21);
  ac.setFan(kSamsungAcFanAuto);
  ac.setSwing(false);
  ac.setSwingH(false);
  ac.setBeep(false);
  ac.setClean(false);
  ac.setQuiet(false);
  ac.setPowerful(false);
  ac.setEcono(false);
  ac.setBreeze(false);
  ac.setDisplay(true);
  ac.setIon(false);
  ac.setSleepTimer(8 * 60);
  EXPECT_EQ(expectedStr, ac.toString());
  ac.send();
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  EXPECT_EQ(SAMSUNG_AC, ac._irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcExtendedBits, ac._irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, ac._irsend.capture.state,
                  ac._irsend.capture.bits);
  EXPECT_EQ(expectedStr, IRAcUtils::resultAcToString(&ac._irsend.capture));
  ac._irsend.reset();
}
