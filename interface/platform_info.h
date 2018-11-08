#pragma once

/**
 * Fetch platform string from flash
 * 
 * This function defaults to CF20 if no string is present in the hardware
 * 
 * \param [out] platformString Buffer of at least 33 bytes where the
 *                             platform string will be stored
 */
void platformInfoGetPlatformString(char *platformString);