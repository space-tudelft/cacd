
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

SRC1 = icddoc.c
SRC2 = icdman.c

TARGET1 = icddoc
TARGET2 = icdman
TARGETS = $(TARGET1) $(TARGET2)

install: $(TARGETS)
	mv $(TARGETS) $(BINPATH)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBPATH)/libddm.a
$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBPATH)/libddm.a
clean:
	$(RM) $(TARGETS)

#EOF
