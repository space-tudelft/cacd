
include ../../../../Config.mk

CWD = $(shell pwd)

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../../../.. -I$(QTLIB)/include -DQT_NO_DEBUG=1

OBJM = AppMainWindow_moc.cc ProcessManager_moc.cc

OBJS = AppMainWindow.o AppMainWindow_moc.o Main.o ProcessManager.o ProcessManager_moc.o

LIBS =	../../guibuilder/libGUIBuilder.a \
	../../parser/libParser.a \
	../../widgets/StatusIndicator/libStatusIndicator.a \
	../../widgets/SectionWidget/libSectionWidget.a \
	../../widgets/SpreadSheet/libSpreadSheet.a \
	../../datastruct/libDatastruct.a \
	../../helper/DaliStyles/libDaliStyles.a \
	../../gui/dialogs/ProcessDescDlg/libProcessDescDlg.a \
	../../gui/validators/libValidators.a \
	../../gui/dialogs/GenerateDlg/libGenerateDlg.a \
	../../gui/dialogs/ConditionListDlg/libConditionListDlg.a \
	../../widgets/TriStateButton/libTristateButton.a \
	../../helper/log/liblog.a \
	../../gui/dialogs/AboutDlg/libAboutDlg.a

TARGET = spock
TARGETDIR = $(ICDPATH)/share/lib/spock

install: libs $(TARGET)
	mv $(TARGET) $(BINPATH)
	if [ -d $(TARGETDIR) ]; then rm -fr $(TARGETDIR); fi
	mkdir $(TARGETDIR)
	cp -r section_icons $(TARGETDIR)
	cp    spock.uis     $(TARGETDIR)

libs:
	make -C ../../datastruct
	make -C ../../gui/dialogs/AboutDlg
	make -C ../../gui/dialogs/ConditionListDlg
	make -C ../../gui/dialogs/GenerateDlg
	make -C ../../gui/dialogs/ProcessDescDlg
	make -C ../../gui/validators
	make -C ../../guibuilder
	make -C ../../helper/DaliStyles
	make -C ../../helper/log
	make -C ../../parser
	make -C ../../widgets/SectionWidget
	make -C ../../widgets/SpreadSheet
	make -C ../../widgets/StatusIndicator
	make -C ../../widgets/TriStateButton

AppMainWindow_moc.cc : AppMainWindow.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@
ProcessManager_moc.cc: ProcessManager.h
	echo \#include \"$(CWD)/$<\" > $@
	$(QTLIB)/bin/moc -i $< >> $@

%.o: %.cc
	$(CCC) $(CFLAGS) -c $<
Main.o: Main.cc
	$(CCC) $(CFLAGS) -DNO_DEBUG=1 -c $<

$(TARGET): $(OBJS) $(LIBS)
	$(CCC) $(LDFLAGS) -o $@ $(OBJS) \
		$(LIBS) \
		-L$(QTLIB)/lib -lqt-mt \
		$(LIBPATH)/libddm.a

# -lXinerama -lXft -lfreetype -lz -lXrender
# -lXmu -lXt -lGLU -lGL -lX11 -ldl -lSM -lICE -lXext -lxcb -lXdmcp -lXau

clean:
	$(RM) $(TARGET) $(OBJS) $(OBJM)
	make -C ../../datastruct clean
	make -C ../../gui/dialogs/AboutDlg clean
	make -C ../../gui/dialogs/ConditionListDlg clean
	make -C ../../gui/dialogs/GenerateDlg clean
	make -C ../../gui/dialogs/ProcessDescDlg clean
	make -C ../../gui/validators clean
	make -C ../../guibuilder clean
	make -C ../../helper/DaliStyles clean
	make -C ../../helper/log clean
	make -C ../../parser clean
	make -C ../../widgets/SectionWidget clean
	make -C ../../widgets/SpreadSheet clean
	make -C ../../widgets/StatusIndicator clean
	make -C ../../widgets/TriStateButton clean

#EOF
