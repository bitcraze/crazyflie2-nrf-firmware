/* SPDX-License-Identifier: LGPL-3.0-or-later */
/**
 * @file
 * Native round-trip and invalid-input tests for CRTPUP assembly.
 */
#include "ble_crtpup.h"
#include "ble_crtpdown.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
/**
 * Run exhaustive length/PID round trips and malformed-fragment checks.
 * @return 0 when all assertions pass; abort on a failed assertion.
 */
int main(void)
{
  BleCrtpupState state;
  uint8_t packet[32];
  for (unsigned i = 0; i < sizeof(packet); i++)
  {
    packet[i] = i;
  }
  for (unsigned length = 1; length <= 32; length++)
  {
    for (unsigned pid = 0; pid < 4; pid++)
    {
      BleCrtpdownFragments fragments;
      assert(bleCrtpdownBuildFragments(packet, length, pid, &fragments));
      bleCrtpupReset(&state);
      for (unsigned j = 0; j < fragments.count; j++)
      {
        bool complete = bleCrtpupConsume(&state, fragments.fragments[j].data, fragments.fragments[j].length);
        assert(complete == (j == fragments.count - 1));
      }
      assert(state.length == length);
      assert(memcmp(state.data, packet, length) == 0);
    }
  }
  uint8_t start[] = {0x9e, 1, 2};
  uint8_t bad_pid[] = {0x20, 3};
  bleCrtpupReset(&state);
  assert(!bleCrtpupConsume(&state, start, sizeof(start)));
  assert(!bleCrtpupConsume(&state, bad_pid, sizeof(bad_pid)));
  assert(!state.active);
  uint8_t overflow[] = {0x80, 1, 2};
  assert(!bleCrtpupConsume(&state, overflow, sizeof(overflow)));
  assert(!state.active);
  assert(!bleCrtpupConsume(&state, NULL, 0));
  uint8_t short_packet[] = {0x81, 4, 5};
  assert(bleCrtpupConsume(&state, short_packet, sizeof(short_packet)));
  assert(state.data[0] == 4 && state.data[1] == 5);
  // A connection reset must discard an incomplete packet.
  assert(!bleCrtpupConsume(&state, start, sizeof(start)));
  bleCrtpupReset(&state);
  uint8_t orphan[] = {0x00, 3};
  assert(!bleCrtpupConsume(&state, orphan, sizeof(orphan)));
  // A fresh start must resynchronize after the discarded packet.
  assert(bleCrtpupConsume(&state, short_packet, sizeof(short_packet)));
  puts("BLE uplink: 128 exact roundtrips and invalid-input checks passed");
  return 0;
}
