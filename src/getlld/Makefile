
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL = extern.h incl.h

OBJS = main.o outp_cif.o outp_ldm.o outp_cmk.o output.o

TARGET = getlld
LINKS  = xcif xcmk xldm

install : $(TARGET)
	for name in $(LINKS); do \
	  ln -s $(TARGET) $$name; \
	  chmod 755 $$name; \
	done
	mv $(TARGET) $(LINKS) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(LINKS) $(OBJS)

#EOF
