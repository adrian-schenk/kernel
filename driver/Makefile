include ../Makefile.config

TARGET=driver.o

$(TARGET): net/net.o registry.o
	$(LD) $(LDFLAGS) -r -o $@ $^

net/net.o:
	$(MAKE) -C net net.o