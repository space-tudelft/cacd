
include ../../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = AboutDlg_moc.cc

OBJS = AboutDlg.o AboutDlg_moc.o

TARGET = libAboutDlg.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

AboutDlg_moc.cc: AboutDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
