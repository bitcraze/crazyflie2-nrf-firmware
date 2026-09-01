#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ble_crtpdown.h"

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__,         \
              #condition);                                                      \
      return 1;                                                                 \
    }                                                                           \
  } while (0)

static int testBuildsSingleBytePacket(void)
{
  const uint8_t payload[] = {0xF3};
  const uint8_t expected[] = {0x80, 0xF3};
  BleCrtpdownFragments result = {0};

  CHECK(bleCrtpdownBuildFragments(payload, sizeof(payload), 0, &result));
  CHECK(result.count == 1);
  CHECK(result.fragments[0].length == sizeof(expected));
  CHECK(memcmp(result.fragments[0].data, expected, sizeof(expected)) == 0);

  return 0;
}

static int testBuildsLargestUnfragmentedPacket(void)
{
  const uint8_t payload[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
  };
  const uint8_t expected[] = {
    0xB2, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
  };
  BleCrtpdownFragments result = {0};

  CHECK(bleCrtpdownBuildFragments(payload, sizeof(payload), 1, &result));
  CHECK(result.count == 1);
  CHECK(result.fragments[0].length == sizeof(expected));
  CHECK(memcmp(result.fragments[0].data, expected, sizeof(expected)) == 0);

  return 0;
}

static int testBuildsSmallestFragmentedPacket(void)
{
  const uint8_t payload[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
  };
  const uint8_t expectedStart[] = {
    0xD3, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
  };
  const uint8_t expectedContinuation[] = {0x40, 0x13};
  BleCrtpdownFragments result = {0};

  CHECK(bleCrtpdownBuildFragments(payload, sizeof(payload), 2, &result));
  CHECK(result.count == 2);
  CHECK(result.fragments[0].length == sizeof(expectedStart));
  CHECK(memcmp(result.fragments[0].data, expectedStart, sizeof(expectedStart)) == 0);
  CHECK(result.fragments[1].length == sizeof(expectedContinuation));
  CHECK(memcmp(result.fragments[1].data, expectedContinuation,
               sizeof(expectedContinuation)) == 0);

  return 0;
}

static int testPacketIdWrapsAfterThree(void)
{
  CHECK(bleCrtpdownNextPid(0) == 1);
  CHECK(bleCrtpdownNextPid(1) == 2);
  CHECK(bleCrtpdownNextPid(2) == 3);
  CHECK(bleCrtpdownNextPid(3) == 0);

  return 0;
}

static int testBuilds31BytePacket(void)
{
  const uint8_t payload[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
  };
  BleCrtpdownFragments result = {0};

  CHECK(bleCrtpdownBuildFragments(payload, sizeof(payload), 3, &result));
  CHECK(result.count == 2);
  CHECK(result.fragments[0].length == 20);
  CHECK(result.fragments[0].data[0] == 0xFE);
  CHECK(memcmp(&result.fragments[0].data[1], payload, 19) == 0);
  CHECK(result.fragments[1].length == 13);
  CHECK(result.fragments[1].data[0] == 0x60);
  CHECK(memcmp(&result.fragments[1].data[1], &payload[19], 12) == 0);

  return 0;
}

static int testBuildsLargestPacket(void)
{
  const uint8_t payload[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  };
  BleCrtpdownFragments result = {0};

  CHECK(bleCrtpdownBuildFragments(payload, sizeof(payload), 0, &result));
  CHECK(result.count == 2);
  CHECK(result.fragments[0].length == 20);
  CHECK(result.fragments[0].data[0] == 0x9F);
  CHECK(memcmp(&result.fragments[0].data[1], payload, 19) == 0);
  CHECK(result.fragments[1].length == 14);
  CHECK(result.fragments[1].data[0] == 0x00);
  CHECK(memcmp(&result.fragments[1].data[1], &payload[19], 13) == 0);

  return 0;
}

static int testRejectsInvalidPacketLengths(void)
{
  const uint8_t payload[33] = {0};
  BleCrtpdownFragments result = {0};

  CHECK(!bleCrtpdownBuildFragments(payload, 0, 0, &result));
  CHECK(!bleCrtpdownBuildFragments(payload, 33, 0, &result));

  return 0;
}

int main(void)
{
  if (testBuildsSingleBytePacket()) {
    return 1;
  }
  if (testBuildsLargestUnfragmentedPacket()) {
    return 1;
  }
  if (testBuildsSmallestFragmentedPacket()) {
    return 1;
  }
  if (testPacketIdWrapsAfterThree()) {
    return 1;
  }
  if (testBuilds31BytePacket()) {
    return 1;
  }
  if (testBuildsLargestPacket()) {
    return 1;
  }
  if (testRejectsInvalidPacketLengths()) {
    return 1;
  }
  puts("issue 101 regression tests passed");
  return 0;
}
