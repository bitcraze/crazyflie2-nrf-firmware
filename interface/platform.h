#pragma once

#include <stdbool.h>

#define PLATFORM_DEVICE_TYPE_STRING_MAX_LEN 33
#define PLATFORM_DEVICE_TYPE_MAX_LEN 31

/**
 * Fetch deviceType string from flash
 * 
 * This function defaults to "0;CF20;R=D" if no string is present in the
 * hardware
 * 
 * \param [out] deviceTypeString Buffer of at least 
 *                               PLATFORM_DEVICE_TYPE_STRING_MAX_LEN bytes
 *                               where the platform string will be stored
 */
void platformGetDeviceTypeString(char *deviceTypeString);

/**
 * Parse deviceType string to extract the deviceType
 * 
 * Ignores the key=value sections.
 * 
 * \param [in] deviceTypeString deviceTypeString extracted from the hardware
 * \param [out] deviceType Buffer of at least PLATFORM_DEVICE_TYPE_MAX_LEN
 *                         bytes where the device type will be stored
 * \return 0 in case of success, 1 in case of failure.
 */
int platformParseDeviceTypeString(const char* deviceTypeString, char* deviceType);

/**
 * Initialize the platform
 * 
 * Initialize the platform discovering capabilities and returns if it has been successful
 * 
 * \return 0 in case of success, 1 in case of failure.
 */
int platformInit();

// ************** Capabilities functions **************
// The following functions can be implemented by different platform to give
// access to the current device capabilities. Not all platform has to implement
// all functions
bool platformHasRfx2411n();