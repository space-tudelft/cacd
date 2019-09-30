
proc do_add_gnd_term {x0 y0} {
   global PORT_W PORT_H PORT_FNT
   global cir_name N_CELL CELL_ARR

   .cv delete outline

   incr N_CELL
   set xl  [expr $x0-$PORT_W]
   set xr  [expr $x0+$PORT_W]
   set yt  [expr $y0-$PORT_H]
   set yb  [expr $y0+$PORT_H]
   set xc [expr ($xl+$xr)/2.]
   set yc [expr ($yt+$yb)/2.]
   .cv create rectangle $xl $yt $xr $yb -fill wheat3\
                        -tag "vss_$N_CELL vss_$N_CELL.vss"
   .cv create text $xc $yc -font $PORT_FNT\
                   -text '0' -tag "vss_$N_CELL vss_$N_CELL.vss"
   set CELL_ARR($N_CELL) "vss_$N_CELL"
}
