
proc do_add_no210 {x0 y0} {
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
                        -fill wheat -tag no210_$N_CELL
# terminals
   .cv create rectangle [expr $xl+$GW] [expr $y0-0.5*$GW]\
                        $xl [expr $y0-1.5*$GW]\
                        -fill wheat3 -tag "no210_$N_CELL no210_$N_CELL.A"
   .cv create rectangle [expr $xl+$GW] [expr $y0+0.5*$GW]\
                        $xl [expr $y0+1.5*$GW]\
                        -fill wheat3 -tag "no210_$N_CELL no210_${N_CELL}.B"
   .cv create rectangle [expr $xr-$GW] [expr $y0+$GW/2.]\
                        $xr [expr $y0-$GW/2.]\
                        -fill wheat3 -tag "no210_$N_CELL no210_$N_CELL.Y"
   .cv create text      [expr $xl+$GW/2.] [expr $y0+2*$GW]\
                        -text "B" -font $PORT_FNT\
                        -tag no210_$N_CELL
   .cv create text      [expr $xl+$GW/2.] [expr $y0-2*$GW]\
                        -text "A" -font $PORT_FNT\
                        -tag no210_$N_CELL
   .cv create text      [expr $xr-$GW/2.] [expr $y0-$GW]\
                        -text "Y" -font $PORT_FNT\
                        -tag no210_$N_CELL
# symbol
   .cv create rectangle [expr $x0-$GW] [expr $y0-$GW]\
                        [expr $x0+$GW] [expr $y0+$GW]\
                        -fill wheat -outline wheat3  -tag no210_$N_CELL
   .cv create line      [expr $xl+$GW] [expr $y0-$GW]\
                        [expr $xl+2*$GW] [expr $y0-$GW]\
                        [expr $xl+2*$GW] [expr $y0-0.5*$GW]\
                        [expr $x0-$GW] [expr $y0-0.5*$GW]\
                        -fill wheat3  -tag no210_$N_CELL
   .cv create line      [expr $xl+$GW] [expr $y0+$GW]\
                        [expr $xl+2*$GW] [expr $y0+$GW]\
                        [expr $xl+2*$GW] [expr $y0+0.5*$GW]\
                        [expr $x0-$GW] [expr $y0+0.5*$GW]\
                        -fill wheat3  -tag no210_$N_CELL
   .cv create line      [expr $xr-$GW] $y0\
                        [expr $x0+$GW] $y0\
                        -fill wheat3  -tag no210_$N_CELL
   .cv create line      [expr $x0+$GW] [expr $y0-$GW]\
                        [expr $x0+2*$GW] $y0\
                        -fill wheat3  -tag no210_$N_CELL
   .cv create text      $x0 $y0 -text ">=1" -font $CELL_FNT\
                        -fill wheat3  -tag no210_$N_CELL

   set CELL_ARR($N_CELL) no210_$N_CELL
}
