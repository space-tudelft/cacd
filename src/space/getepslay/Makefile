
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

OBJS = input.o main.o scan.o plot_eps.o

TARGET= getepslay

install: $(TARGET)
	mv $(TARGET) $(BINPATH)

$(OBJS): %.o: ../mplot/%.c
	$(CC) -DEPSPLOT $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH)/auxil.a $(LIBPATH)/libddm.a -lm

clean:; $(RM) *.o $(TARGET)

#EOF
