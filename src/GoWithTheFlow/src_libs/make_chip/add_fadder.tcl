
proc do_add_fadder {x0 y0} {
   global GW CELL_FNT PORT_FNT
   global N_CELL CELL_ARR

   .cv delete outline
   incr N_CELL

   set xl  [expr $x0-7*$GW/2.]
   set xr  [expr $x0+7*$GW/2.]
   set yt  [expr $y0-7*$GW/2.]
   set yb  [expr $y0+7*$GW/2.]

# outline
   .cv create rectangle $xl $yt $xr $yb\
                        -fill wheat -tag fadder_$N_CELL
# terminals
   .cv create rectangle [expr $xl+2*$GW] $yt\
                        [expr $xl+3*$GW] [expr $yt+$GW]\
                        -fill wheat3 -tag "fadder_$N_CELL fadder_$N_CELL.A"
   .cv create rectangle [expr $xr-3*$GW] $yt\
                        [expr $xr-2*$GW] [expr $yt+$GW]\
                        -fill wheat3 -tag "fadder_$N_CELL fadder_$N_CELL.B"
   .cv create rectangle [expr $xl+$GW] [expr $y0+$GW/2.]\
                        $xl [expr $y0-$GW/2.]\
                        -fill wheat3 -tag "fadder_$N_CELL fadder_$N_CELL.CO"
   .cv create rectangle [expr $xr-$GW] [expr $y0+$GW/2.]\
                        $xr [expr $y0-$GW/2.]\
                        -fill wheat3 -tag "fadder_$N_CELL fadder_$N_CELL.CI"
   .cv create rectangle [expr $x0-$GW/2.] $yb\
                        [expr $x0+$GW/2.] [expr $yb-$GW]\
                        -fill wheat3 -tag "fadder_$N_CELL fadder_$N_CELL.S"
   .cv create text      [expr $xl+$GW] [expr $yt+0.5*$GW]\
                        -text "A" -font $PORT_FNT\
                        -tag fadder_$N_CELL
   .cv create text      [expr $xr-$GW] [expr $yt+0.5*$GW]\
                        -text "B" -font $PORT_FNT\
                        -tag fadder_$N_CELL
   .cv create text      [expr $xl+$GW] [expr $y0+$GW]\
                        -text "CO" -font $PORT_FNT\
                        -tag fadder_$N_CELL
   .cv create text      [expr $xr-$GW] [expr $y0+$GW]\
                        -text "CI" -font $PORT_FNT\
                        -tag fadder_$N_CELL
   .cv create text      [expr $x0+$GW] [expr $yb-$GW]\
                        -text "S" -font $PORT_FNT\
                        -tag fadder_$N_CELL
#symbol
   .cv create rectangle [expr $x0-$GW] [expr $y0-$GW]\
                        [expr $x0+$GW] [expr $y0+$GW]\
                        -fill wheat -outline wheat3  -tag fadder_$N_CELL
   .cv create line      [expr $x0-$GW] [expr $yt+$GW]\
                        [expr $x0-$GW] [expr $y0-2*$GW]\
                        [expr $x0-0.5*$GW] [expr $y0-2*$GW]\
                        [expr $x0-0.5*$GW] [expr $y0-$GW]\
                        -fill wheat3  -tag fadder_$N_CELL
   .cv create line      [expr $x0+$GW] [expr $yt+$GW]\
                        [expr $x0+$GW] [expr $y0-2*$GW]\
                        [expr $x0+0.5*$GW] [expr $y0-2*$GW]\
                        [expr $x0+0.5*$GW] [expr $y0-$GW]\
                        -fill wheat3  -tag fadder_$N_CELL
   .cv create line      $x0 [expr $y0+$GW]\
                        $x0 [expr $yb-$GW]\
                        -fill wheat3  -tag fadder_$N_CELL
   .cv create line      [expr $xl+$GW] $y0\
                        [expr $x0-$GW] $y0\
                        -fill wheat3  -tag fadder_$N_CELL
   .cv create line      [expr $xr-$GW] $y0\
                        [expr $x0+$GW] $y0\
                        -fill wheat3  -tag fadder_$N_CELL
   .cv create text      $x0 $y0 -text "+" -font CELL_FNT -fill wheat3\
                        -tag  fadder_$N_CELL

   set CELL_ARR($N_CELL) fadder_$N_CELL
}
