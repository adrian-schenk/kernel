include ../../Makefile.config

# Define variables
TARGET = ext4_complete.o
OBJ = ext4.o

.PHONY: all clean

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $(TARGET) $(OBJ)

%.o: %.c
	${CC} ${CFLAGS}  $< -o $@ -I../ -I../../kernel

%.o: %.S
	${CC} ${CFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
