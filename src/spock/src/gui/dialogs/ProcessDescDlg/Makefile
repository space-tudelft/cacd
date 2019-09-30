
include ../../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = ProcessDescDlg_moc.cc

OBJS = ProcessDescDlg.o ProcessDescDlg_moc.o

TARGET = libProcessDescDlg.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

ProcessDescDlg_moc.cc: ProcessDescDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
