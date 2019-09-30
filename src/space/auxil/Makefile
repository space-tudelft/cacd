
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

TARGET= auxil.a

HDRS = auxil.h assert.h bool.h color.h debug.h malloc.h monitor.h \
	proto.h portable.h plot.h

OBJS = alarm.o assert.o binom.o clock.o color.o debug.o die.o extrasay.o \
	factln.o fopen.o gammaln.o gauss.o getopt.o levin.o \
	monit.o monitor.o mprintf.o multinom.o new.o param.o plot.o \
	say.o signal.o strfind.o strsave.o tempdir.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

testalarm: alarm.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^

testassert: assert.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^

testclock: clock.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^

testdebug: debug.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^ $(LIBPATH)/$(TARGET)

testnew: new.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^ $(LIBPATH)/$(TARGET)

testsay: say.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^

testsignal: signal.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^ $(LIBPATH)/$(TARGET)

testtemp: tempdir.c
	$(CC) $(CFLAGS) -DDRIVER -o $@ $^

msum: msum.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBPATH)/$(TARGET)

clean:; $(RM) $(OBJS) $(TARGET) msum test*

#EOF
