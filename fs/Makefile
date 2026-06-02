include ../Makefile.config

# Define variables
TARGET = fs.o
OBJ = ext4/ext4.o

.PHONY: all clean

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $(TARGET) $(OBJ)

ext4/ext4.o: ext4/ext4.c ext4/ext4.h ext4/Makefile ../Makefile.config
	$(MAKE) -C ext4 ext4.o

%.o: %.c
	${CC} ${CFLAGS}  $< -o $@

%.o: %.S
	${CC} ${CFLAGS}  $< -o $@
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
