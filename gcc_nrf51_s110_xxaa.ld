/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x16000, LENGTH = 0x2A000 /* 88 kB is taken by S110, 168 kB for app. */
  RAM (rwx) : ORIGIN = 0x20002000, LENGTH = 0x2000 /* 8 kB is taken by S110,8 kB for app. */
}


INCLUDE "gcc_nrf51_common.ld"
