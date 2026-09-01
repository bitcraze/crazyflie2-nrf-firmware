#include "ble_crtpdown.h"

#include <string.h>

#define MAX_FRAGMENT_PAYLOAD 19

bool bleCrtpdownBuildFragments(const uint8_t *data, uint16_t length, uint8_t pid,
                               BleCrtpdownFragments *result)
{
  if (!data || !result || length == 0 || length > 32) {
    return false;
  }

  result->count = 1;
  result->fragments[0].length =
    (length > MAX_FRAGMENT_PAYLOAD ? MAX_FRAGMENT_PAYLOAD : length) + 1;
  result->fragments[0].data[0] = 0x80 | ((pid << 5) & 0x60) | (length - 1);
  memcpy(&result->fragments[0].data[1], data, result->fragments[0].length - 1);

  if (length > MAX_FRAGMENT_PAYLOAD) {
    result->count = 2;
    result->fragments[1].length = length - MAX_FRAGMENT_PAYLOAD + 1;
    result->fragments[1].data[0] = (pid << 5) & 0x60;
    memcpy(&result->fragments[1].data[1], &data[MAX_FRAGMENT_PAYLOAD],
           length - MAX_FRAGMENT_PAYLOAD);
  }

  return true;
}

uint8_t bleCrtpdownNextPid(uint8_t pid)
{
  return (pid + 1) & 0x03;
}
