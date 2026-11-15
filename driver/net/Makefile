include ../../Makefile.config

TARGET=net.o
OBJS=e1000.o

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -r -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -MMD -MP $< -o $@

%.o: %.S
	${CC} ${CFLAGS} -MMD -MP $< -o $@
