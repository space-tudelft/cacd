
include ../../Config.mk

CWD = $(shell pwd)

TARGET = tabs

install: $(TARGET)
	@rm -f $(BINPATH)/$(TARGET)
	cp $(TARGET) $(BINPATH)/$(TARGET)
	chmod +x $(BINPATH)/$(TARGET)

$(TARGET):
	echo "#!/bin/sh" > $(TARGET)
	echo "exec tclsh $(CWD)/tabs.tcl \$$*" >> $(TARGET)
clean:

#EOF
