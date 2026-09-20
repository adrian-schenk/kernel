include ../Makefile.config

# Define variables
TARGET = net.o
SRCS = $(patsubst ./%,%,$(shell find . -type f \( -name "*.c" -o -name "*.S" \)))
OBJ = $(SRCS:.c=.o)
OBJ := $(OBJ:.S=.o)
DEPS = $(OBJ:.o=.d)

.PHONY: all clean

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $(TARGET) $(OBJ)

%.o: %.c
	${CC} ${CFLAGS} -MMD -MP $< -o $@

%.o: %.S
	${CC} ${CFLAGS} -MMD -MP $< -o $@

-include $(DEPS)

# Clean up generated files
clean:
	rm -f $(OBJ) $(DEPS) $(TARGET)
