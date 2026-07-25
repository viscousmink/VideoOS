# Path to Circle relative to your project root
CIRCLEHOME = tools/circle

# Target name (Rules.mk will build kernel8.elf & kernel8.img)
TARGET = kernel8

# Objects required for your application
OBJS = src/main.o

# Core Circle library required for boot, memory management, and drivers
LIBS = $(CIRCLEHOME)/lib/libcircle.a

# Compiler flags for Raspberry Pi 4 (Cortex-A72)
CXXFLAGS += -mcpu=cortex-a72 -DUSE_PL011_SERIAL
CFLAGS   += -mcpu=cortex-a72

# Include Circle's master build rules
include $(CIRCLEHOME)/Rules.mk

# Custom compilation rule for sources inside src/
src/%.o: src/%.cpp
	$(PREFIX)g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# Extend Circle's clean target to remove objects in src/
clean: clean_src

clean_src:
	rm -f src/*.o $(TARGET).elf $(TARGET).img $(TARGET).map