*#<input>
* input source and output and GND groundings
vin   TA TC1 DC 0 AC 1
vload TB TC2 DC 0
vgnd  TC1  0 DC 0

* first line after .control can only contain title
*#<control>
.control
ny9t example
ac dec 100 100Meg 50G
* plot abs(re(i(vload))),abs(im(i(vload))) loglog
print abs(re(i(vload))),abs(im(i(vload))) 
.endc
