CC=g++
CFLAGS=-c -std=c++11 -Wall -O3
LDFLAGS=-lSDL2
SOURCES= \
		main.cpp \
		Hardware/CPU.cpp Hardware/Memory.cpp Hardware/Timer.cpp Hardware/GPU.cpp \
		Hardware/Instruction.cpp Hardware/ExtInstruction.cpp \
		Hardware/Config.cpp Hardware/Joypad.cpp \
		Util/ROMReader.cpp Util/GUI.cpp


OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=gbemu
EXECUTABLE_BOARD=gbemu_b



all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@ $(LDFLAGS)
	@echo "Build successfully created!"

board: $(SOURCES) $(EXECUTABLE_BOARD)

$(EXECUTABLE_BOARD): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@
	@echo "Build for board successfully created!"

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	-rm -rf Hardware/*.o
	-rm -rf *.o
	-rm -rf $(EXECUTABLE)
