#pragma once

#include <stdbool.h>

#include "esb.h"
#include "syslink.h"

bool syslinkRadioRawFanOut(const struct syslinkPacket *source,
                           EsbPacket *esbPacket, EsbPacket *blePacket);
