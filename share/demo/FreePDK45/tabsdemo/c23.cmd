option sigunit = 1e-03
option simperiod = 100

set VSS = l
set VDD = h
set CK  = l (h l)*~
set RCI = l h
set R = h*2 l
option level = 2

define Q5 Q4 Q3 Q2 Q1 : State - - - - - : $dec

print R, CK, RCI, Q1, Q2, Q3, Q4, Q5, RCO, State

