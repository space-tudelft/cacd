
include ../../Config.mk

CFLAGS=-Wall -Wno-unused -fPIC -I../../.. -DCONFIG_SPACE3D

TARGET = green.a

HDRS = export.h extern.h gputil.h green.h images.h

OBJS = analytic.o galerkin.o gputil.o green.o integrat.o mkgreen.o \
	colloc.o init.o intadptv.o mpgreen.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.c
	$(CC)  -c $< $(CFLAGS) -O
%.o: %.C
	$(CCC) -c $< $(CFLAGS) -O
%.o: %.cc
	$(CCC) -c $< $(CFLAGS) -O
colloc.o: colloc.cc
	$(CCC) -c $< $(CFLAGS)

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

colloc: colloc.cc
	$(CCC) -DDRIVER $(CFLAGS) $^ -o $@ \
		$(LIBPATH)/auxil.a

galerkin: galerkin.c
	$(CCC) -DDRIVER $(CFLAGS) $^ -o $@ \
		$(LIBPATH)/green.a \
		$(LIBPATH)/auxil.a -lm

mkgreen: mkgreen.c
	$(CC) -DDRIVER $(CFLAGS) $^ -o $@ \
		$(LIBPATH)/auxil.a -lm

clean:
	$(RM) $(OBJS) $(TARGET) colloc galerkin mkgreen

#EOF
