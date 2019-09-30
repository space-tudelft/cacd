/* This commandfile first stores at the following addresses 
   the following numbers:
       0   1      
       1   2
       2   4
       3   8
       4  16
       5  20
    And then multiplies the contents of some addresses
*/
       
      
set phi1 = (h l)*~
set phi2 = (l h)*~
set sel[0] = l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2 l*2
set sel[1] = l*2 l*2 l*2 l*2 h*2 h*2 l*2 l*2 l*2 l*2 l*2 l*2 h*2 h*2
set sel[2] = l*2 l*2 h*2 h*2 l*2 l*2 l*2 l*2 l*2 h*2 h*2 h*2 l*2 l*2
set sel[3] = l*2 h*2 l*2 h*2 l*2 h*2 l*2 h*2 h*2 l*2 l*2 h*2 l*2 h*2 
set in[0] = l*2 l*2 l*2 l*2 l*2 l*2
set in[1] = l*2 l*2 l*2 l*2 l*2 l*2
set in[2] = l*2 l*2 l*2 l*2 l*2 l*2
set in[3] = l*2 l*2 l*2 l*2 h*2 h*2
set in[4] = l*2 l*2 l*2 h*2 l*2 l*2
set in[5] = l*2 l*2 h*2 l*2 l*2 h*2
set in[6] = l*2 h*2 l*2 l*2 l*2 l*2
set in[7] = h*2 l*2 l*2 l*2 l*2 l*2
set readwrite = l*12 h*16
set sx = l*13 (h*2 l*2)*4
set sy = l*13 (l*2 h*2)*4

option simperiod = 28
option sigunit = 10n
option outacc = 10p
option level = 3 

define x[0..7] : xdec \
       - - - - - - - - : $dec

define y[0..7] : ydec \
       - - - - - - - - : $dec

define out[0..15] : outdec \
       - - - - - - - - - - - - - - - - : $dec

print phi1 phi2 sel[0..3] in[0..7] readwrite k[0..7] sx sy 
print x[0..7] y[0..7] out[0..15]
print xdec ydec outdec
plot phi1 phi2 sel[0..3] in[0..7] readwrite k[0..7]
plot sx sy x[0..7] y[0..7] out[0..15]
