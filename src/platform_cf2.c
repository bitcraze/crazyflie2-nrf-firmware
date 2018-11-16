#include "platform.h"

#include <string.h>

bool has_rfx2411n = false;

bool platformHasRfx2411n()
{
  return has_rfx2411n;
}

int platformInit()
{
  static char deviceTypeString[PLATFORM_DEVICE_TYPE_STRING_MAX_LEN];
  static char deviceType[PLATFORM_DEVICE_TYPE_MAX_LEN];

  platformGetDeviceTypeString(deviceTypeString);
  if (platformParseDeviceTypeString(deviceTypeString, deviceType) != 0) {
    return 1;
  }

  if (!strcmp(deviceType, "CF20")) {
    has_rfx2411n = false;
  } else if (!strcmp(deviceType, "CF21")) {
    has_rfx2411n = true;
  } else {
    return 1;
  }

  return 0;
}