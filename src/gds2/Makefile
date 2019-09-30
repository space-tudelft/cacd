
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = incl.h defs.h

OBJ1 = cgi.o swire.o
OBJ2 = cga.o
OBJ3 = cig.o
OBJ4 = clambda.o
OBJ5 = cga2gds.o
OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)

TARGET1 = cgi
TARGET2 = cga
TARGET3 = cig
TARGET4 = clambda
TARGET5 = cga2gds
TARGETS = $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

install: $(TARGETS)
	mv $(TARGETS) $(BINPATH)

cgi.c: incl.h defs.h
swire.c: defs.h
cig.c: defs.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET1): $(OBJ1)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET2): $(OBJ2)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET3): $(OBJ3)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET4): $(OBJ4)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm
$(TARGET5): $(OBJ5)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGETS) $(OBJS)

#END
