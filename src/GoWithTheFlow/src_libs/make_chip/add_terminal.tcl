
proc add_term {x0 y0 type name} {
   global PORT_W PORT_H PORT_FNT
   global PORT_ARR N_PORT cir_name

   set xl  [expr $x0-$PORT_W/2.]
   set xr  [expr $x0+$PORT_W/2.]
   set yt  [expr $y0-$PORT_H/2.]
   set yb  [expr $y0+$PORT_H/2.]
   .cv create rectangle $xl $yt $xr $yb -fill wheat3\
                                        -tag "border $cir_name.$name P_$name"
   set bc [.cv bbox border]
   if {[expr abs($x0-[lindex $bc 0])] < 50} {
      .cv create text [expr $x0-$PORT_W] $y0 -anchor e -font $PORT_FNT\
                      -text $name -tag "border $cir_name.$name T_$name"
   }\
   elseif {[expr abs($x0-[lindex $bc 2])] < 50} {
      .cv create text [expr $x0 +$PORT_W] $y0 -anchor w -font $PORT_FNT\
                      -text $name -tag "border $cir_name.$name T_$name"
   }\
   elseif {[expr abs($y0-[lindex $bc 1])] < 50} {
      .cv create text $x0 [expr $y0-$PORT_H] -anchor s -font $PORT_FNT\
                      -text $name -tag "border $cir_name.$name T_$name"
   }\
   else {
      .cv create text $x0 [expr $y0+$PORT_H] -anchor n -font $PORT_FNT\
                      -text $name -tag "border $cir_name.$name T_$name"
   }
   incr N_PORT
   set PORT_ARR($N_PORT) "$type $name"
}
