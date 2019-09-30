
include ../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = StatusIndicator_moc.cc

OBJS = StatusIndicator.o StatusIndicator_moc.o

TARGET = libStatusIndicator.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

StatusIndicator_moc.cc: StatusIndicator.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
