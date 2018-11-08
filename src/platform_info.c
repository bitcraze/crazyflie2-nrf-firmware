#include "platform_info.h"

#include <string.h>

static const char *defaultPlatform = "0;CF20;R=D";

static volatile unsigned char *platformStringLocation = 0x3FFE0;

void platformInfoGetPlatformString(char *platformString)
{
  if (platformStringLocation == 0xffu) {
    strncpy(platformString, defaultPlatform, 32);
    platformString[32] = 0;
  } else {
    strncpy(platformString, platformStringLocation, 32);
    platformString[32] = 0;
  }
}