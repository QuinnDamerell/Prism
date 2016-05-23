###########################################################################
# Prism

TARGET := prism

CPP_FILES += \
	Prism/src/FadeCandyDevice.cpp \
	Prism/src/Prism.cpp \
	Prism/src/PrismCommandHost.cpp \
	Prism/src/UsbDeviceManager.cpp \
	Prism/src/Gems/ColorPeaks.cpp \
	Prism/src/Gems/RandomColorGem.cpp \
	Prism/src/Gems/SolidColorGem.cpp \
	Prism/src/Gems/RunningPixel.cpp

INCLUDES += -IPrism/interface
INCLUDES += -IPrism/inc/Gems
INCLUDES += -IPrism/inc

CLEAN_FILES += Prism/src/*.d Prism/src/*.o Prism/src/Gems/*.d Prism/src/Gems/*.o
CPPFLAGS += -Wno-strict-aliasing -fexceptions

###########################################################################
# LightFx

TARGET := LightFx

CPP_FILES += \
	LightFX/LightFX/src/Timer.cpp \
	LightFX/LightFX/src/Panel.cpp \
	LightFX/LightFX/src/OutputBitmap.cpp \
	LightFX/LightFX/src/ConstantRateDriver.cpp \
	LightFX/LightFX/src/Bitmap.cpp \
	LightFX/LightFX/src/TimelineObject.cpp \
	LightFX/LightFX/src/Drawable/Drawable.cpp \
	LightFX/LightFX/src/Drawable/SolidDrawable.cpp \
	LightFX/LightFX/src/Fadables/Fader.cpp \
	LightFX/LightFX/src/Colorables/Colorer.cpp \
	LightFX/LightFX/src/Colorables/RainbowColorer.cpp \

INCLUDES += -ILightFX/LightFX/inc
INCLUDES += -ILightFX/LightFX/inc/Colorables
INCLUDES += -ILightFX/LightFX/inc/Drawables
INCLUDES += -ILightFX/LightFX/inc/Fadeables
INCLUDES += -ILightFX/LightFX/interfaces
INCLUDES += -ILightFX/LightFX/interfaces/Colorables
INCLUDES += -ILightFX/LightFX/interfaces/Drawables
INCLUDES += -ILightFX/LightFX/interfaces/Fadables

CLEAN_FILES += LightFX/LightFX/src/*.d LightFX/LightFX/src/*.o LightFX/LightFX/src/Drawable/*.d LightFX/LightFX/src/Drawable/*.o
CLEAN_FILES += LightFX/LightFX/src/Fadables/*.d LightFX/LightFX/src/Fadables/*.o LightFX/LightFX/src/Colorables/*.d LightFX/LightFX/src/Colorables/*.o
CPPFLAGS += -Wno-strict-aliasing -fexceptions

###########################################################################
# PrismPi

TARGET := PrismPi.out

CPP_FILES += \
	PrismPi/main.cpp \

CLEAN_FILES += PrismPi/*.d PrismPi/*.o
CPPFLAGS += -Wno-strict-aliasing -fexceptions

###########################################################################
# System Support

SYS := $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
UNAME := Linux
endif
ifneq (, $(findstring mingw, $(SYS)))
UNAME := MINGW32
endif
ifneq (, $(findstring darwin, $(SYS)))
UNAME := Darwin
endif

MINGW := $(findstring MINGW32, $(UNAME))
LIBS += -lstdc++ -lm
VERSION := 0.1
CXXFLAGS += -DFCSERVER_VERSION=$(VERSION)

ifeq ($(UNAME), Darwin)
	# Mac OS X (32-bit build)
	LDFLAGS += -m32
	CPPFLAGS += -m32 -DHAVE_POLL_H

	ifeq ("$(shell which llvm-gcc)", "")
		# We want to support all the way back to OS 10.6 (Snow Leopard), which used gcc
		# instead of llvm. Omit some flags that this old gcc doesn't handle.
	else
		# Assume it's a new enough Mac OS version
		CPPFLAGS += -Wno-tautological-constant-out-of-range-compare
		CXXFLAGS += -std=gnu++0x
	endif
else
	# Everyone except ancient versions of gcc on Mac OS likes this flag...
	CXXFLAGS += -std=gnu++0x
endif

ifneq ("$(MINGW)", "")
	# Windows
	TARGET := $(TARGET).exe
	CPPFLAGS += -D_WIN32_WINNT=0x0501

	# Static build makes it portable but big, UPX packer decreases size a lot.
	LDFLAGS += -static
	PACK_CMD := upx\upx391w.exe $(TARGET)
endif

ifneq ("$(DEBUG)", "")
	# Debug build
	TARGET := debug-$(TARGET)
	CPPFLAGS += -g -DDEBUG -DENABLE_LOGGING
	PACK_CMD :=
else
	# Optimized build	
	STRIP_CMD := strip $(TARGET)
	CPPFLAGS += -Os -DNDEBUG
	LDFLAGS += -Os
endif

###########################################################################
# Built-in libusbx

C_FILES += \
	libusb/libusb/core.c \
	libusb/libusb/descriptor.c \
	libusb/libusb/hotplug.c \
	libusb/libusb/io.c \
	libusb/libusb/strerror.c \
	libusb/libusb/sync.c

ifeq ($(UNAME), Darwin)
	# Mac OS X

	C_FILES += \
		libusb/libusb/os/darwin_usb.c \
		libusb/libusb/os/poll_posix.c \
		libusb/libusb/os/threads_posix.c

	LIBS += -framework CoreFoundation -framework IOKit -lobjc
	CPPFLAGS += -DOS_DARWIN -DTHREADS_POSIX -DPOLL_NFDS_TYPE=nfds_t \
		-DLIBUSB_CALL= -DDEFAULT_VISIBILITY= -DHAVE_GETTIMEOFDAY
endif

ifeq ($(UNAME), Linux)
	# Linux

	C_FILES += \
		libusb/libusb/os/linux_usbfs.c \
		libusb/libusb/os/linux_netlink.c \
		libusb/libusb/os/poll_posix.c \
		libusb/libusb/os/threads_posix.c

	INCLUDES += -IPrismPi
	LIBS += -lpthread -lrt
	CPPFLAGS += -DOS_LINUX -DTHREADS_POSIX -DPOLL_NFDS_TYPE=nfds_t \
		-DLIBUSB_CALL= -DDEFAULT_VISIBILITY= -DHAVE_GETTIMEOFDAY -DHAVE_POLL_H \
		-DHAVE_ASM_TYPES_H -DHAVE_SYS_SOCKET_H -DHAVE_LINUX_NETLINK_H -DHAVE_LINUX_FILTER_H
endif

ifneq ("$(MINGW)", "")
	# Windows

	C_FILES += \
		libusb/libusb/os/windows_usb.c \
		libusb/libusb/os/poll_windows.c \
		libusb/libusb/os/threads_windows.c

	LIBS += -lws2_32
	CPPFLAGS += -DOS_WINDOWS -DPOLL_NFDS_TYPE=int -DDEFAULT_VISIBILITY= -DHAVE_GETTIMEOFDAY
endif

INCLUDES += -Ilibusb/libusb
CLEAN_FILES += \
	libusb/libusb/*.d libusb/libusb/*.o \
	libusb/libusb/os/*.d libusb/libusb/os/*.o

###########################################################################
# Build Rules

# Compiler options for C and C++
CPPFLAGS += -MMD $(INCLUDES)

# Compiler options for C++ only
CXXFLAGS += -felide-constructors -fno-exceptions

OBJS := $(CPP_FILES:.cpp=.o) $(C_FILES:.c=.o)

print-%: ; @echo $* = $($*)
all: print-SYS $(TARGET)

# FIXME: A race condition between objects regeneration and their source mtime in make ? 
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(STRIP_CMD)
	$(PACK_CMD)
	rm -f src/version.o

-include $(OBJS:.o=.d)

clean:
	rm -f $(CLEAN_FILES) $(TARGET)

.PHONY: clean all
