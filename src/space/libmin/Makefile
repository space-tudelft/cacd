
include ../../Config.mk

CFLAGS =-Wall -Wno-unused $(OCFLAGS) -I../../..

TARGET= libmin.a

INCL =	minbmat.h mincompl.h mincover.h mindefs.h minparse.h \
	minrobdd.h minsolve.h minsym.h mintable.h mintaut.h \
	STDIntSet.h STDtypes.h

OBJS =	minbmat.o mincompl.o mincover.o mindefs.o minparse.o \
	minrobdd.o minsolve.o minsym.o mintable.o mintaut.o \
	STDIntSet.o

install: $(TARGET)
	mv $(TARGET) $(LIBPATH)

%.o: %.cc $(INCL)
	$(CCC) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	ar crs $(TARGET) $(OBJS)

clean:; $(RM) $(OBJS) $(TARGET)

#EOF
