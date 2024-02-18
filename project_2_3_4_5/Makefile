SOURCE	:= Main.c Parser.c Registers.c Core.c
CC	:= gcc
TARGET	:= RVSim

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) -lm -w

clean:
	rm -f $(TARGET)
