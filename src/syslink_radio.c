#include "syslink_radio.h"

#include <string.h>

static void copyPacket(const struct syslinkPacket *source, EsbPacket *destination)
{
  if (destination) {
    destination->size = source->length;
    memcpy(destination->data, source->data, source->length);
  }
}

bool syslinkRadioRawFanOut(const struct syslinkPacket *source,
                           EsbPacket *esbPacket, EsbPacket *blePacket)
{
  if (!source || source->length > sizeof(((EsbPacket *)0)->data)) {
    return false;
  }

  copyPacket(source, esbPacket);
  copyPacket(source, blePacket);
  return true;
}
