DFB_EXAMPLE=$(shell pwd)

#===============================================================================
# export variable
#===============================================================================
ifeq ($(CFG_HI_EXPORT_FLAG),)
SDK_DIR := $(CURDIR)/../../../../..
include $(SDK_DIR)/base.mak
endif

include $(SAMPLE_DIR)/base.mak

#===============================================================================
# local variable
#===============================================================================
CFLAGS := $(subst -Werror,,$(CFG_HI_SAMPLE_CFLAGS))
ifeq (${CFG_HI_DIRECTFB_SUPPORT},y)
CFLAGS += -DHI_ADVCA_FUNCTION_RELEASE
endif

CFLAGS +=   -I$(SDK_INC_DIR)                    \
            -I$(SAMPLE_DIR)/common              \
	    -I${DFB_EXAMPLE}                    \
	    -I${DFB_EXAMPLE}/../../include      \
            -I$(SDK_DIR)/source/component/directfb/directfb/include/directfb \


CFLAGS += -DFONT=\"./../share/directfb-examples/fonts/decker.ttf\"
CFLAGS += -DDATADIR=\"./../share/directfb-examples\"
CFLAGS += -DVERSION=\"1.6.0\"

LIBPATH = -L$(SDK_SHARE_LIB_DIR)          \
          -L$(SDK_DIR)/source/component/directfb/directfb/lib


LIB = -Wl,--start-group

LIB += -lhi_common -lfreetype -ljpeg -lpng  -ldirectfb -ldirect -lfusion -lstdc++ -lz -lpthread -ldl -lm -lrt

ifneq ($(findstring $(CFG_HI_CHIP_TYPE), hi3716mv310 hi3716mv320 hi3716mv330 hi3110ev500),)
LIB +=-lhi_mpi
else
LIB +=-lhi_msp
endif

LIB += -Wl,--end-group


COMMON_SRC := $(patsubst %.o, %.c, $(COMMON_FILES))
COMMON_SRC += ${DFB_EXAMPLE}/../../include/sample_displayInit.c

SRC := $(wildcard *.c)

#=======================================================================

APP= $(SRC:%.c=%)

#===============================================================================
# rules
#===============================================================================
.PHONY: all clean install uninstall

all: $(APP)

$(APP):  %: %.c
	$(AT)$(CC) -o $@  $< $(COMMON_SRC) $(LIBPATH) $(LIB) $(CFLAGS)

clean:
	$(AT)rm -f $(APP)
	$(AT)rm -f *.o
install:all
	$(AT)cp -rf $(APP) ${DFB_EXAMPLE}/../../../sample/bin
uninstall:clean
	
