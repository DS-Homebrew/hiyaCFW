#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export HBMENU_MAJOR	:= 1
export HBMENU_MINOR	:= 1
export HBMENU_PATCH	:= 0


VERSION	:=	$(HBMENU_MAJOR).$(HBMENU_MINOR).$(HBMENU_PATCH)
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files embedded using bin2o
# GRAPHICS is a list of directories containing image files to be converted with grit
#---------------------------------------------------------------------------------
TARGET		:=	hiyaCFW
BUILD		:=	build
SOURCES		:=	source
INCLUDES	:=	include
DATA		:=	data
GRAPHICS	:=  gfx

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

CFLAGS	:=	-g -Wall -O2\
 		-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
		-ffast-math \
		$(ARCH)
		
CFLAGS	+=	$(INCLUDE) -DARM9
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=ds_arm9.specs -g -Wl,--gc-sections $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project (order is important)
#---------------------------------------------------------------------------------
LIBS	:= 	-lfat -lnds9


#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
PNGFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	load.bin

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(PNGFILES:.png=.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

icons := $(wildcard *.bmp)

ifneq (,$(findstring $(TARGET).bmp,$(icons)))
	export GAME_ICON := $(CURDIR)/$(TARGET).bmp
else
	ifneq (,$(findstring icon.bmp,$(icons)))
		export GAME_ICON := $(CURDIR)/icon.bmp
	endif
endif

export GAME_TITLE := $(TARGET)

.PHONY: bootloader BootStrap clean arm7/$(TARGET).elf arm9/$(TARGET).elf

all:	bootloader $(TARGET).nds

$(TARGET).nds:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -r9 00080002 \
			-b $(CURDIR)/icon.bmp "hiyaCFW;CFW for Nintendo DSi;made by Apache Thunder" \
			-g HIYA 01 "HIYACFW" -z 80040000 -u 00030004
	python2 fix_ndsheader.py $(TARGET).nds
	cp $(TARGET).nds hiya.dsi

$(TARGET).arm7: arm7/$(TARGET).elf
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

$(TARGET).arm9: arm9/$(TARGET).elf
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	@$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	@$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).nds.orig.nds hiya.dsi $(TARGET).arm9 $(TARGET).arm7.elf $(TARGET).arm9.elf data bootcode.dsi
	@$(MAKE) -C bootloader clean
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
		
data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader "EXTRA_CFLAGS= -DNO_DLDI"

#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
