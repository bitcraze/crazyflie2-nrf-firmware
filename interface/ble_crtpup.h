/* SPDX-License-Identifier: LGPL-3.0-or-later */
/**
 * @file
 * Bounded assembly of CRTP packets from BLE CRTPUP fragments.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Assembly state owned by one BLE connection. */
typedef struct
{
  uint8_t data[32];  ///< Assembled bytes; valid as a packet only after successful completion.
  uint8_t length;    ///< Declared total packet length, including the CRTP header.
  uint8_t index;     ///< Number of packet bytes received so far.
  uint8_t pid;       ///< Two-bit identifier of the packet being assembled.
  bool active;      ///< True while waiting for the rest of an incomplete packet.
} BleCrtpupState;

/**
 * Clear an incomplete packet and its assembled data.
 *
 * Call this at connection boundaries and before using a new state object.
 *
 * @param[out] state State to clear; NULL is accepted and has no effect.
 */
void bleCrtpupReset(BleCrtpupState *state);

/**
 * Consume one fragment using the documented length-minus-one/PID wire format.
 *
 * A start fragment replaces any incomplete packet. Invalid input clears the
 * state. Completion leaves data and length available until the next call or
 * reset; active is then false. A false return never exposes a complete packet.
 *
 * @param[in,out] state Connection-local state initialized with bleCrtpupReset.
 * @param[in] fragment Control byte followed by 1-19 packet bytes.
 * @param[in] size Fragment size, including the control byte (2-20 bytes).
 * @return True only when state->data contains exactly state->length packet
 *         bytes. False for an incomplete packet, invalid input or NULL state.
 */
bool bleCrtpupConsume(BleCrtpupState *state, const uint8_t *fragment, uint16_t size);
