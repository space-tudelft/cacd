
proc write_cir_file {} {
   global cir_name CELL_ARR CONN_ARR PORT_ARR
   global BBOX_ID N_CELL N_CONN N_PORT N_GND

   set f_cir [open $cir_name.cir "w"]
   set bbox [.cv coords $BBOX_ID]
   puts $f_cir "B [lindex $bbox 0] [lindex $bbox 1] [lindex $bbox 2]\
                [lindex $bbox 3] $cir_name"
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      set pname [lindex $PORT_ARR($i) 1]
      set bbox [.cv coords P_$pname]
      set xc [expr ([lindex $bbox 0]+[lindex $bbox 2])/2.]
      set yc [expr ([lindex $bbox 1]+[lindex $bbox 3])/2.]
      puts $f_cir "T $PORT_ARR($i) $xc $yc"
   }
   for {set i [expr 0]} {$i <= $N_CELL} {incr i} {
      set cname $CELL_ARR($i)
      if {$cname == ""} {
         puts $f_cir "I"
      }\
      else {
         set bbox [.cv bbox $cname]
         set xc [expr ([lindex $bbox 0]+[lindex $bbox 2])/2.]
         set yc [expr ([lindex $bbox 1]+[lindex $bbox 3])/2.]
         puts $f_cir "I $CELL_ARR($i) $xc $yc"
      }
   }
   for {set i [expr 0]} {$i <= $N_CONN} {incr i} {
      set cname $CONN_ARR($i)
      puts $f_cir "C $CONN_ARR($i)"
   }
   close $f_cir
}
