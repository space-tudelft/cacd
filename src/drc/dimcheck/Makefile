
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS =	main.o main_check.o ini.o free_sr.o dig_circle.o \
	extr_prof.o Iedge.o update_sr.o checks.o fltr_err.o

TARGET = dimcheck

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/libddm.a -lm

clean:
	$(RM) $(TARGET) $(OBJS)

#EOF
