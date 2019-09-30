
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJ1 = autocheck.o
OBJ2 = dimcheck.o

TARGET1 = autocheck
TARGET2 = dimcheck2

install: $(TARGET1) $(TARGET2)
	mv $(TARGET1) $(TARGET2) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET1): $(OBJ1)
	$(CC) $(LDFLAGS) -o $@ $^
$(TARGET2): $(OBJ2)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET1) $(OBJ1) $(TARGET2) $(OBJ2)

#END
