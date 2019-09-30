
include ../../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = ConditionListDlg_moc.cc

OBJS = ConditionListDlg.o ConditionListDlg_moc.o

TARGET = libConditionListDlg.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

ConditionListDlg_moc.cc: ConditionListDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
