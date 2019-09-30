
proc connect {from_tag to_tag} {
   global N_CONN CONN_ARR
   incr N_CONN
   set spot1 [.cv coords $from_tag]
   set x1 [expr ([lindex $spot1 0] + [lindex $spot1 2])/2.]
   set y1 [expr ([lindex $spot1 1] + [lindex $spot1 3])/2.]
   set spot2 [.cv coords $to_tag ]
   set x2 [expr ([lindex $spot2 0] + [lindex $spot2 2])/2.]
   set y2 [expr ([lindex $spot2 1] + [lindex $spot2 3])/2.]
   .cv create line $x1 $y1 $x2 $y2 -tag conn_$N_CONN
   set CONN_ARR($N_CONN) $from_tag>$to_tag
}

proc do_connect {x0 y0} {
   global sel_tag  N_CONN CONN_ARR

   if {$sel_tag == ""} {
      return
   }
   .cv delete outline
   set to_tag [lindex [.cv gettags [.cv find closest $x0 $y0]] 1]
   if {[string first "." $to_tag] < 0} {
      return
   }
   if {$to_tag == $sel_tag } {
      return
   }
   connect $sel_tag $to_tag
}

proc update_conn {cn} {
   global CONN_ARR
   set name $CONN_ARR($cn)
   set tgs [split $name ">"]
   set tg1 [lindex $tgs 0]
   set tg2 [lindex $tgs 1]
   set spot1 [.cv coords $tg1]
   set x1 [expr ([lindex $spot1 0] + [lindex $spot1 2])/2.]
   set y1 [expr ([lindex $spot1 1] + [lindex $spot1 3])/2.]
   set spot2 [.cv coords $tg2 ]
   set x2 [expr ([lindex $spot2 0] + [lindex $spot2 2])/2.]
   set y2 [expr ([lindex $spot2 1] + [lindex $spot2 3])/2.]
   .cv coords conn_$cn $x1 $y1 $x2 $y2
}
