####################################################################################################
#
# flipflip's ESP8266 Album Art Display
#
# Copyright (c) 2020 Philippe Kehl <flipflip at oinkzwurgl dot org>
#
####################################################################################################

SKETCH  := AlbumArtDisplay.ino
ARDUINO := arduino
PORT    := /dev/ttyUSB0
SRCS    := $(SKETCH) $(sort $(wildcard src/*.h) $(wildcard src/*.c) $(wildcard src/*.cpp))
DEPS    := Makefile tools/debug.pl tools/gen_config.h
PERL    := perl
TOUCH   := touch
RM      := rm
MV      := mv
SED     := sed
SHELL   := bash

.PHONY: defaulttarget
defaulttarget:
	@echo "Make what?! Try 'make help'..."

####################################################################################################
# canned receipe for verify, build, clean, etc.

target_names   :=
targets_verify :=
targets_upload :=
targets_clean  :=
targets_trace  :=

define makeRules

target_names += $(strip $(1))

targets_verify += $(strip $(1))-verify

.PHONY: $(strip $(1))-verify
$(strip $(1))-verify: build-$(strip $(1))/.verify

build-$(strip $(1))/.verify: $(SRCS)
	$(PERL) tools/gen_config_h.pl $(1)
	$(ARDUINO) --verify --preserve-temp-files --verbose --board $(2) $$< --pref build.path=$$(dir $$@) 2>&1 \
		| sed -r 's|/.*?/build-$(strip $(1))/sketch/||g' ; exit "$$$${PIPESTATUS[0]}"
	$(TOUCH) $$@

targets_upload += $(strip $(1))-upload

.PHONY: $(strip $(1))-upload
$(strip $(1))-upload: $(SRCS)
	$(PERL) tools/gen_config_h.pl $(1)
	$(ARDUINO) --upload --preserve-temp-files --verbose --board $(2) $$< --pref build.path=build-$(strip $(1)) --port $(PORT) 2>&1 \
		| sed -r 's|/.*?/build-$(strip $(1))/sketch/||g' ; exit "$$$${PIPESTATUS[0]}"

targets_clean += $(strip $(1))-clean

.PHONY: $(strip $(1))-clean
$(strip $(1))-clean:
	$(RM) -rf build-$(strip $(1))

targets_trace += $(strip $(1))-trace

.PHONY: $(strip $(1))-trace
$(strip $(1))-trace: build-$(strip $(1))/$(SKETCH).elf
	$(PERL) tools/EspStackTraceDecoder.pl build-$(strip $(1))/$(SKETCH).elf -

endef

####################################################################################################
# generate target receipes

# see ~/.arduino15/packages/esp32/hardware/esp32/1.0.4/boards.txt
BOARD_ESP32_MINI32 := esp32:esp32:esp32:PSRAM=disabled,PartitionScheme=noota_3g,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=921600,DebugLevel=verbose
$(eval $(call makeRules, esp32-mini32, $(BOARD_ESP32_MINI32)))

# manual recipes for making and flashing SPIFFS image
mkspiffs=$(lastword $(sort $(wildcard ~/.arduino15/packages/esp32/tools/mkspiffs/*/mkspiffs)))
esptoolpy=$(lastword $(sort $(wildcard ~/.arduino15/packages/esp32/tools/esptool_py/*/esptool.py)))
build-esp32-mini32/spiffs.bin: $(wildcard data/*) Makefile
	$(RM) -f $@
	$(mkspiffs) -c data -b 4096 -p 256 -s 3080192 $@
	ls -l $@
.PHONE: flash-spiffs
flash-spiffs: build-esp32-mini32/spiffs.bin
	$(esptoolpy) --chip esp32 --port $(PORT) --baud 921600 --before default_reset --after hard_reset write_flash \
		-z --flash_mode qio --flash_freq 80m --flash_size detect \
		0x110000 $<

####################################################################################################

.PHONY: help
help:
	@echo
	@echo "Usage:"
	@echo
	@echo "    make <target> [PORT=...] [ARDUINO=...]"
	@echo
	@echo "Where <target> can be:"
	@echo
	@echo "    <name>-verify    build (verify) sketch"
	@echo "    <name>-upload    build and upload sketch"
	@echo "    <name>-trace     decode stack trace"
	@echo "    <name>-clean     clean build directory"
	@echo "    monitor          show serial output"
	@echo "    clean            clean all build directories"
	@echo "    verify           build (verify) sketch for all <name>s"
	@echo "    flash-spiffs     make and flash filesystem (data/*)"
	@echo
	@echo "The following <name>s are available:"
	@echo
	@echo "    $(target_names)"
	@echo
	@echo "Optional parameters:"
	@echo
	@echo "    PORT      serial port (default: $(PORT))"
	@echo "    ARDUINO   path to arduino binary (default: $(ARDUINO))"
	@echo
	@echo "Example to build, upload and start serial monitor:"
	@echo
	@echo "   make $(firstword $(target_names))-upload monitor"
	@echo

.PHONY: debugmf
debugmf:
	@echo "target_names: $(target_names)"
	@echo "targets_verify: $(targets_verify)"
	@echo "targets_upload: $(targets_upload)"
	@echo "targets_trace: $(targets_trace)"
	@echo "targets_clean: $(targets_clean)"

.PHONY: verify
verify: $(targets_verify)

.PHONY: clean
clean: $(targets_clean)
	rm -f src/config.h

.PHONY: monitor
monitor:
	perl tools/debug.pl $(PORT):115200

####################################################################################################
# eof