# Path to Circle relative to your project root
CIRCLEHOME = tools/circle

TARGET = kernel8
OBJS = src/main.o

# EXACT LINK ORDER: FatFS, SDCard, Core FS, VC4, LinuxEmu, Circle
LIBS = $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
       $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
       $(CIRCLEHOME)/lib/fs/libfs.a \
       $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
       $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
       $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
       $(CIRCLEHOME)/lib/libcircle.a

# Include directories and Target Defines (Pi 3 / 64-bit)
DEFINE += -I $(CIRCLEHOME)/addon/vc4 \
          -I $(CIRCLEHOME)/addon/vc4/interface/khronos/include \
          -I $(CIRCLEHOME)/addon/SDCard \
          -I $(CIRCLEHOME)/addon/fatfs \
          -DAARCH=64 \
          -DSDHOST_DEF_EMMC

# Add POSIX threads addon from Circle
ADDON_DIR += tools/circle/addon/pthreads

ARM_ALLOW_MULTI_CORE = 1

# Ensure defines are set for threading and stdlib
DEFINES += -DSTDLIB_SUPPORT=1 -DWAIT_USE_STI

# Compiler flags targeting Raspberry Pi 3 (Cortex-A53)w
CXXFLAGS += -mcpu=cortex-a72 -DUSE_PL011_SERIAL
CFLAGS   += -mcpu=cortex-a72

# Add the pre-compiled multicore object directly to the linker invocation
OBJS += tools/circle/lib/multicore.o \
        tools/circle/lib/spinlock.o

include $(CIRCLEHOME)/Rules.mk

src/%.o: src/%.cpp
	$(PREFIX)g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean: clean_src
clean_src:
	rm -f src/*.o $(TARGET).elf $(TARGET).img $(TARGET).map