
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	main.o main_bool.o ini_heap.o sel_edge.o Iedge.o \
	reheap.o free_sr.o update_sr.o chk_hrchy.o mk_frmstr.o \
	extr_prof.o add_grpnbr.o fr_frmstr.o

TARGET = nbool

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
