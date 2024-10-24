#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifneq (,$(shell which python3))
PYTHON	:= python3
else ifneq (,$(shell which python2))
PYTHON	:= python2
else ifneq (,$(shell which python))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

export TARGET	:=	$(shell basename $(CURDIR))
export TOPDIR	:=	$(CURDIR)

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES	:=

# These set the information text in the nds file
GAME_ICON	:= icon.bmp
GAME_TITLE	:= hiyaCFW
GAME_SUBTITLE1	:= CFW for Nintendo DSi
GAME_SUBTITLE2	:= made by Apache Thunder

include $(DEVKITARM)/ds_rules

.PHONY: bootloader checkarm7 checkarm9 clean libslim

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: libslim bootloader checkarm7 checkarm9 $(TARGET).nds

#---------------------------------------------------------------------------------
bootloader:
	$(MAKE) -C bootloader "EXTRA_CFLAGS= -DNO_DLDI"

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf -r9 00080002 \
			-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
			-g HIYA 01 "HIYACFW" -z 80040000 -u 00030004 $(_ADDFILES)
	$(PYTHON) fix_ndsheader.py $(TARGET).nds
	cp $(TARGET).nds hiya.dsi

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C libs/libslim clean
	@rm -f $(TARGET).nds $(TARGET).nds.orig.nds hiya.dsi

libslim:
	$(MAKE) -C libs/libslim/libslim
