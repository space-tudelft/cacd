
include ../../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = GenerateDlg_moc.cc GenerateDlgInternal_moc.cc NewProcListEntryDlg_moc.cc

OBJS = GenerateDlg.o GenerateDlg_moc.o GenerateDlgInternal.o GenerateDlgInternal_moc.o NewProcListEntryDlg.o NewProcListEntryDlg_moc.o

TARGET = libGenerateDlg.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

GenerateDlg_moc.cc: GenerateDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
GenerateDlgInternal_moc.cc: GenerateDlgInternal.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
NewProcListEntryDlg_moc.cc: NewProcListEntryDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
