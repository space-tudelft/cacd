
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

TARGET= libfunc.a

OBJS =	BSFREE.o BSRESET.o BSROTATE.o BSSET.o BSTOI.o BSUNDEF.o BTAND.o BTEXNOR.o BTEXOR.o BTINVERT.o \
	BTNAND.o BTNOR.o BTOR.o BWAND.o BWEXNOR.o BWEXOR.o BWINVERT.o BWNAND.o BWNOR.o BWOR.o \
	ITOBS.o ITOTC.o TCTOI.o curs_error.o func_error.o single_c_s.o single_s.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.c
	@if [ -f /usr/include/curses.h ]; then \
		echo $(CC) $(CFLAGS) -c $<; \
		$(CC) $(CFLAGS) -c $<; \
	else \
		echo $(CC) $(CFLAGS) -DNO_CURSES -c $<; \
		$(CC) $(CFLAGS) -DNO_CURSES -c $<; \
	fi

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

clean:
	$(RM) $(OBJS) $(TARGET)

#EOF
