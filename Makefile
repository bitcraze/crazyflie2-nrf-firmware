#Put your personal build config in config.mk and DO NOT COMMIT IT!
-include config.mk

PLATFORM ?= cf2
-include platform/platform_$(PLATFORM).mk

BLE      ?= 1    # BLE mode activated or not. If disabled, CRTP mode is active

PROGRAM = $(PLATFORM)_nrf
PROJECT_NAME     := crazyflie2_nrf_firmware
TARGETS          := $(PROGRAM)
OUTPUT_DIRECTORY := _build
SDK_ROOT         := vendor/nrf5sdk
PROJ_DIR         := src

OPENOCD           ?= openocd
OPENOCD_DIR       ?=
OPENOCD_INTERFACE ?= $(OPENOCD_DIR)interface/stlink.cfg
OPENOCD_TARGET    ?= target/nrf51.cfg
OPENOCD_CMDS      ?=


ifeq ($(DEVBOARD),1)
BOARD ?= PCA10028
else
BOARD ?= CUSTOM
endif

$(OUTPUT_DIRECTORY)/$(PROGRAM).out: \
  LINKER_SCRIPT  := crazyflie2_nrf_firmware.ld

# Source files common to all targets
SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c
SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/button/app_button.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_error.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_error_weak.c
SRC_FILES += $(SDK_ROOT)/components/libraries/timer/app_timer.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_util_platform.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/crc16/crc16.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fds/fds.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fstorage/fstorage.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/util/nrf_assert.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/util/sdk_errors.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/sensorsim/sensorsim.c
SRC_FILES += $(SDK_ROOT)/components/libraries/mailbox/app_mailbox.c
SRC_FILES += $(SDK_ROOT)/components/boards/boards.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c
#SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
SRC_FILES += $(SDK_ROOT)/components/libraries/bsp/bsp.c
SRC_FILES += $(SDK_ROOT)/components/libraries/bsp/bsp_btn_ble.c
#SRC_FILES += $(SDK_ROOT)/components/libraries/bsp/bsp_nfc.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_advdata.c
SRC_FILES += $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_params.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_state.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_srv_common.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/id_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_data.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_database.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_id.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/pm_mutex.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/security_manager.c
SRC_FILES += $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf51.S
SRC_FILES += $(SDK_ROOT)/components/toolchain/system_nrf51.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c
SRC_FILES += $(PROJ_DIR)/ble/ble.c
SRC_FILES += $(PROJ_DIR)/ble/ble_crazyflies.c
SRC_FILES += $(PROJ_DIR)/ble/timeslot.c
SRC_FILES += $(PROJ_DIR)/ow.c
SRC_FILES += $(PROJ_DIR)/ow/owlnk.c
SRC_FILES += $(PROJ_DIR)/ow/ownet.c
SRC_FILES += $(PROJ_DIR)/ow/owtran.c
SRC_FILES += $(PROJ_DIR)/ow/crcutil.c
SRC_FILES += $(PROJ_DIR)/pm.c
SRC_FILES += $(PROJ_DIR)/syslink.c
SRC_FILES += $(PROJ_DIR)/esb.c
SRC_FILES += $(PROJ_DIR)/main.c
SRC_FILES += $(PROJ_DIR)/uart.c
SRC_FILES += $(PROJ_DIR)/syslink.c
SRC_FILES += $(PROJ_DIR)/pm.c
SRC_FILES += $(PROJ_DIR)/systick.c
SRC_FILES += $(PROJ_DIR)/button.c
SRC_FILES += $(PROJ_DIR)/swd.c
SRC_FILES += $(PROJ_DIR)/ds2431.c
SRC_FILES += $(PROJ_DIR)/ds28e05.c
SRC_FILES += $(PROJ_DIR)/esb.c
SRC_FILES += $(PROJ_DIR)/memory.c
SRC_FILES += $(PROJ_DIR)/platform.c
SRC_FILES += $(PROJ_DIR)/platform_$(PLATFORM).c
SRC_FILES += $(PROJ_DIR)/debug.c
SRC_FILES += $(PROJ_DIR)/systick.c
SRC_FILES += $(PROJ_DIR)/shutdown.c

# Include folders common to all targets
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s130/headers
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mailbox
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_gls
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fstorage
INC_FOLDERS += $(SDK_ROOT)/components/boards
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/common
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_advertising
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/adc
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s130/headers/nrf51
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_bas_c
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_hrs_c
INC_FOLDERS += $(SDK_ROOT)/components/libraries/queue
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_dtm
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/cmsis/include
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_rscs_c
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/uart
INC_FOLDERS += $(SDK_ROOT)/components/ble/common
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_lls
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/wdt
INC_FOLDERS += $(SDK_ROOT)/components/libraries/bsp
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_bas
INC_FOLDERS += $(SDK_ROOT)/components/libraries/experimental_section_vars
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_ans_c
INC_FOLDERS += $(SDK_ROOT)/components/libraries/slip
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mem_manager
INC_FOLDERS += $(SDK_ROOT)/external/segger_rtt
INC_FOLDERS += $(SDK_ROOT)/components/libraries/csense_drv
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/hal
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_nus_c
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rtc
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_ias
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd/class/hid/mouse
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/ppi
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_dfu
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/twis_slave
INC_FOLDERS += $(SDK_ROOT)/components
INC_FOLDERS += $(SDK_ROOT)/components/libraries/scheduler
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_lbs
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_hts
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/delay
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crc16
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/timer
INC_FOLDERS += $(SDK_ROOT)/components/libraries/util
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd/class/cdc
INC_FOLDERS += $(SDK_ROOT)/components/libraries/csense
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rng
INC_FOLDERS += $(SDK_ROOT)/components/libraries/low_power_pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/hardfault
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_cscs
INC_FOLDERS += $(SDK_ROOT)/components/libraries/uart
INC_FOLDERS += $(SDK_ROOT)/components/libraries/hci
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd/class/hid/kbd
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/spi_slave
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/lpcomp
INC_FOLDERS += $(SDK_ROOT)/components/libraries/timer
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/power
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd/config
INC_FOLDERS += $(SDK_ROOT)/components/toolchain
INC_FOLDERS += $(SDK_ROOT)/components/libraries/led_softblink
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/qdec
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_cts_c
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/spi_master
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_nus
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_hids
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pdm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crc32
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd/class/audio
INC_FOLDERS += $(SDK_ROOT)/components/libraries/sensorsim
INC_FOLDERS += $(SDK_ROOT)/components/ble/peer_manager
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/swi
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_tps
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_dis
INC_FOLDERS += $(SDK_ROOT)/components/device
INC_FOLDERS += $(SDK_ROOT)/components/ble/nrf_ble_qwr
INC_FOLDERS += $(SDK_ROOT)/components/libraries/button
INC_FOLDERS += $(SDK_ROOT)/components/libraries/usbd
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/saadc
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_lbs_c
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_racp
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/gcc
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fds
INC_FOLDERS += $(SDK_ROOT)/components/libraries/twi
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/clock
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_rscs
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/usbd
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/common/softdevice_handler
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_hrs
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log/src
INC_FOLDERS += $(PROJ_DIR)/../config
INC_FOLDERS += $(PROJ_DIR)/../interface
INC_FOLDERS += $(PROJ_DIR)/../_build

# Libraries common to all targets
LIB_FILES += \

# C flags common to all targets
CFLAGS += -DBOARD_${BOARD}
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF51
CFLAGS += -DS130
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF51422
CFLAGS += -DNRF_SD_BLE_API_VERSION=2
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs
CFLAGS +=  -Wall -Werror -Os -g3 -fsingle-precision-constant -ffast-math -std=gnu11
CFLAGS += -mfloat-abi=soft
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums 
# Disable checks of newer compilers that the SDK does not pass
CFLAGS += -Wno-error=array-bounds

# Enable app config
CFLAGS += -DUSE_APP_CONFIG

# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DBOARD_${BOARD}
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF51
ASMFLAGS += -DS130
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF51422
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=2
ASMFLAGS += -D__STACK_SIZE=512
ASMFLAGS += -D__HEAP_SIZE=512

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys


.PHONY: $(TARGETS) default all clean help flash_jlink flash_softdevice_jlink version.h

# Default target - first one defined
default: $(PROGRAM)
ifeq ($(strip $(BLE)),1)
	@echo "BLE  Activated"
else
	@echo "BLE  Disabled"
endif
	@echo "Built for platform $(PLATFORM)"

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	cf2_nrf

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# make main.c depend on version.h so it gets built.
$(OUTPUT_DIRECTORY)/$(TARGETS)_main.c.o: version.h

version.h:
	@echo Generating version.h
	python3 tools/build/generateVersionHeader.py --crazyflie-base $(abspath .) --output $(OUTPUT_DIRECTORY)/$@

# Flash the program
flash_jlink: $(OUTPUT_DIRECTORY)/$(PROGRAM).hex
	@echo Flashing: $<
	nrfjprog --program $< -f nrf51 --sectorerase --verify
	nrfjprog --reset -f nrf51

# Flash softdevice
flash_softdevice_jlink:
	@echo Flashing: s130_nrf51_2.0.1_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex -f nrf51 --sectorerase --verify
	nrfjprog --reset -f nrf51

erase_jlink:
	nrfjprog --eraseall -f nrf51

## Flash and debug targets

flash: $(OUTPUT_DIRECTORY)/$(PROGRAM).hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET)  -c init -c targets -c "reset halt" \
                 -c "flash write_image erase $(OUTPUT_DIRECTORY)/$(PROGRAM).hex" -c "verify_image $(OUTPUT_DIRECTORY)/$(PROGRAM).hex" \
                 -c "reset run" -c shutdown

flash_s130: $(NRF_S110)/s130_nrf51_2.0.1_softdevice.hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "nrf51 mass_erase" \
                 -c "flash write_image erase $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex" \
                 -c "reset run" -c shutdown

flash_mbs: bootloaders/nrf_mbs_v1.0.hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "flash write_image erase $^" -c "verify_image $^" -c "reset halt" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001014 0x3F000" \
	               -c "reset run" -c shutdown

flash_cload: bootloaders/cload_nrf_v1.0.hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "flash write_image erase $^" -c "verify_image $^" -c "reset halt" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001014 0x3F000" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001080 0x3A000" -c "reset run" -c shutdown

flash_mbs_21: bootloaders/nrf_mbs_cf21.hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "flash write_image erase $^" -c "verify_image $^" -c "reset halt" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001014 0x3F000" \
	               -c "reset run" -c shutdown

flash_cload_21: bootloaders/cload_nrf_cf21.hex
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "flash write_image erase $^" -c "verify_image $^" -c "reset halt" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001014 0x3F000" \
	               -c "mww 0x4001e504 0x01" -c "mww 0x10001080 0x3A000" -c "reset run" -c shutdown

mass_erase:
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
                 -c "nrf51 mass_erase" -c shutdown

reset:
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets \
	               -c reset -c shutdown

openocd: $(OUTPUT_DIRECTORY)/$(PROGRAM).out
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets


semihosting: $(OUTPUT_DIRECTORY)/$(PROGRAM).out
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) $(OPENOCD_CMDS) -f $(OPENOCD_TARGET) -c init -c targets -c reset -c "arm semihosting enable" -c reset

gdb: $(OUTPUT_DIRECTORY)/$(PROGRAM).out
	$(GDB) -ex "target remote localhost:3333" -ex "monitor reset halt" $^

cload: $(OUTPUT_DIRECTORY)/$(PROGRAM).bin
	$(CLOAD_SCRIPT) flash $(OUTPUT_DIRECTORY)/$(PROGRAM).bin nrf51-fw

factory_reset:
	make mass_erase
	make flash_s130
	make flash_mbs
	make flash_cload
	make flash

factory_reset_21:
	make mass_erase
	make flash_s130
	make flash_mbs_21
	make flash_cload_21
	make flash
