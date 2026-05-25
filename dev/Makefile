include ../Makefile.config

# Define variables
TARGET = devs.o
OBJ = keyboard.o

.PHONY: all clean

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $(TARGET) $(OBJ)

%.o: %.c
	${CC} ${CFLAGS}  $< -o $@

%.o: %.S
	${CC} ${CFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
