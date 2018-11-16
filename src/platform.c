#include "platform.h"

#include <string.h>

static const char *defaultDeviceType = "0;CF20;R=D";

static volatile unsigned char *deviceTypeStringLocation = 0x3FFE0;

void platformGetDeviceTypeString(char *deviceTypeString)
{
  if (deviceTypeStringLocation[0] == 0xffu) {
    strncpy(deviceTypeString, defaultDeviceType, 32);
    deviceTypeString[32] = 0;
  } else {
    strncpy(deviceTypeString, deviceTypeStringLocation, 32);
    deviceTypeString[32] = 0;
  }
}

int platformParseDeviceTypeString(const char* deviceTypeString, char* deviceType) {
  char *state;
  char *tok;
  
  // first token is the version, must be "0"
  tok = strtok_r(deviceTypeString, ";", &state);
  if (tok == NULL || strcmp(tok, "0")) {
    return 1;
  }

  // Second token is the platform name
  tok = strtok_r(NULL, ";", &state);
  if (tok == NULL) {
    return 1;
  }
  strncpy(deviceType, tok, PLATFORM_DEVICE_TYPE_MAX_LEN);
  deviceType[PLATFORM_DEVICE_TYPE_MAX_LEN-1] = '\0';

  // Next tokens are KEY=VALUE pairs, ignored for now

  return 0;
}
