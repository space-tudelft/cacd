
include ../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../..

INCL =	incl.h

OBJS =	chg_group.o comp_event.o convert.o debug.o errexit.o find.o free_group.o ins_event.o \
	ins_nwf.o ins_olf.o main.o merge.o no_groups.o read_boxes.o ScanProf.o set_group.o

TARGET = makevln

install : $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c $(INCL)
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
