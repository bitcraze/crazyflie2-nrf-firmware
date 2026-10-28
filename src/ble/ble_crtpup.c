/* SPDX-License-Identifier: LGPL-3.0-or-later */
/**
 * @file
 * Implementation of the CRTPUP assembly contract documented in ble_crtpup.h.
 */
#include "ble_crtpup.h"

#include <string.h>

void bleCrtpupReset(BleCrtpupState *state)
{
  if (state)
  {
    memset(state, 0, sizeof(*state));
  }
}

bool bleCrtpupConsume(BleCrtpupState *state, const uint8_t *fragment, uint16_t size)
{
  if (!state)
  {
    return false;
  }
  if (!fragment || size < 2 || size > 20)
  {
    bleCrtpupReset(state);
    return false;
  }
  const uint8_t control = fragment[0];
  const uint8_t pid = (control >> 5) & 3;
  if (control & 0x80)
  {
    bleCrtpupReset(state);
    state->active = true;
    state->pid = pid;
    state->length = (control & 31) + 1;
  }
  else if (!state->active || pid != state->pid || (control & 31))
  {
    bleCrtpupReset(state);
    return false;
  }
  if (state->index + size - 1 > state->length)
  {
    bleCrtpupReset(state);
    return false;
  }
  memcpy(state->data + state->index, fragment + 1, size - 1);
  state->index += size - 1;
  if (state->index == state->length)
  {
    state->active = false;
    return true;
  }
  return false;
}
