include ../Makefile.config

# Define variables
TARGET = net.o
OBJ = ethernet/ethernet.o arp/arp.o icmp/icmp.o

.PHONY: all clean

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $(TARGET) $(OBJ)

%.o: %.c
	${CC} ${CFLAGS}  $< -o $@ -I../dev -I../fs -I./

%.o: %.S
	${CC} ${CFLAGS}  $< -o $@ -I../dev -I../fs -I./
# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)
