#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BLE_CRTPDOWN_MAX_FRAGMENTS 2
#define BLE_CRTPDOWN_MAX_NOTIFICATION_SIZE 20

typedef struct {
  uint8_t data[BLE_CRTPDOWN_MAX_NOTIFICATION_SIZE];
  uint8_t length;
} BleCrtpdownFragment;

typedef struct {
  BleCrtpdownFragment fragments[BLE_CRTPDOWN_MAX_FRAGMENTS];
  uint8_t count;
} BleCrtpdownFragments;

bool bleCrtpdownBuildFragments(const uint8_t *data, uint16_t length, uint8_t pid,
                               BleCrtpdownFragments *result);

uint8_t bleCrtpdownNextPid(uint8_t pid);
