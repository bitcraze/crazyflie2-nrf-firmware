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
 * crc32_calc.h - defines the interface for CRC32 checksum
 *                calculation. Contains function declarations for
 *                computing CRC32 checksums on data buffers.
 *
 */

#ifndef CRC32_CALC_H
#define CRC32_CALC_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Computes a 32-bit CRC using the polynomial 0xEDB88320.
 *        The remainder is XORed with 0xFFFFFFFF at the end.
 *
 * @param buffer Pointer to the data buffer
 * @param size   Number of bytes in the buffer
 * @return       32-bit CRC of the data
 */
uint32_t crc32CalculateBuffer(const void* buffer, size_t size);

#endif /* CRC32_CALC_H */
