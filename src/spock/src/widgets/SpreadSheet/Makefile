
include ../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM =	Cell_moc.cc ComboCell_moc.cc ConditionListCell_moc.cc DaliCell_moc.cc \
	DaliColorSelectDlg_moc.cc EditCell_moc.cc SpreadSheet_moc.cc XSpaceCell_moc.cc

OBJS =	Cell.o ComboCell.o ConditionListCell.o DaliCell.o DaliColorSelectDlg.o EditCell.o SpreadSheet.o XSpaceCell.o \
	Cell_moc.o ComboCell_moc.o ConditionListCell_moc.o DaliCell_moc.o \
	DaliColorSelectDlg_moc.o EditCell_moc.o SpreadSheet_moc.o XSpaceCell_moc.o

TARGET = libSpreadSheet.a

$(TARGET): $(OBJS)
	ar crs $@ $(OBJS)
	ranlib $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<

Cell_moc.cc: Cell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
ComboCell_moc.cc: ComboCell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
ConditionListCell_moc.cc: ConditionListCell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
DaliCell_moc.cc: DaliCell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
DaliColorSelectDlg_moc.cc: DaliColorSelectDlg.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
EditCell_moc.cc: EditCell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
SpreadSheet_moc.cc: SpreadSheet.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
XSpaceCell_moc.cc: XSpaceCell.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)

#EOF
