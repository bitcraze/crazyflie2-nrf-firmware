/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie 2.x NRF Firmware
 *
 * Copyright (C) 2025 Bitcraze AB
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 *
 *
 * crc32_calc.c - implements CRC32 checksum calculation
 *                functionality for verifying data integrity in
 *                memory and communication. Provides straightforward
 *                CRC32 computation using bitwise operations.
 *
 */

#include "crc32_calc.h"
#include <stddef.h>

#define POLYNOMIAL        0xEDB88320u
#define INITIAL_REMAINDER 0xFFFFFFFFu
#define FINAL_XOR_VALUE   0xFFFFFFFFu

static uint32_t crc32ByBit(const uint8_t* message, uint32_t length)
{
    uint32_t remainder = INITIAL_REMAINDER;

    for (uint32_t i = 0; i < length; i++)
    {
        remainder ^= (uint32_t)message[i];
        // Process 8 bits
        for (int b = 0; b < 8; b++)
        {
            if (remainder & 1)
            {
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder >>= 1;
            }
        }
    }

    remainder ^= FINAL_XOR_VALUE;
    return remainder;
}

/**
 * @brief Computes a 32-bit CRC using the polynomial 0xEDB88320.
 *        The remainder is XORed with 0xFFFFFFFF at the end.
 *
 * @param buffer Pointer to the data buffer
 * @param size   Number of bytes in the buffer
 * @return       32-bit CRC of the data
 */
uint32_t crc32CalculateBuffer(const void* buffer, size_t size)
{
    return crc32ByBit((const uint8_t*)buffer, (uint32_t)size);
}
