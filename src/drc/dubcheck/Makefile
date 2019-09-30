
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	main.o main_check.o ini.o free_sr.o dig_circle.o \
	get_vln.o extr_prof.o Iedge.o update_sr.o checks.o \
	extr_ovlp.o print_err.o fr_errstr.o print_sr.o fltr_err.o

TARGET = dubcheck

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
