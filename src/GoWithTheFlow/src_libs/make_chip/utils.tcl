
proc snap_x {x} {
   global GW

   set origin [.cv coords origin]
   set x_orig  [lindex $origin 0]
   if {$x < $x_orig} {
      set i0 [expr int(($x-$x_orig)/$GW-0.5)]
   }\
   else {
      set i0 [expr int(($x-$x_orig)/$GW+0.5)]
   }
   set x0 [expr $x_orig+$i0*$GW]
   return $x0
}

proc snap_y {y} {
   global GW

   set origin [.cv coords origin]
   set y_orig  [lindex $origin 1]
   if {$y < $y_orig} {
      set j0 [expr int(($y-$y_orig)/$GW-0.5)]
   }\
   else {
      set j0 [expr int(($y-$y_orig)/$GW+0.5)]
   }
   set y0 [expr $y_orig+$j0*$GW]
   return $y0
}

proc press_commands {x0 y0} {
   global mode cmd x_orig y_orig x_prev y_prev

   if {$mode == "EDIT"} {
      set x_orig $x0
      set y_orig $y0
      set x_prev $x0
      set y_prev $y0
      switch $cmd {
         "ADD_NA210"  {set_outline $x0 $y0 na210_dim}
         "ADD_NO210"  {set_outline $x0 $y0 no210_dim}
         "ADD_EX210"  {set_outline $x0 $y0 ex210_dim}
         "ADD_IV110"  {set_outline $x0 $y0 iv110_dim}
         "ADD_GND"    {set_outline $x0 $y0 term_dim}
         "ADD_AND2"   {set_outline $x0 $y0 and2_dim}
         "ADD_FADDER" {set_outline $x0 $y0 fadder_dim}
         "CONNECT"    {set_tag $x0 $y0}
         "MOVE"       {set_outline $x0 $y0 no_dim}
         "DELETE"     {delete_item $x0 $y0}
      }
   }
}

proc release_commands {x0 y0} {
   global mode cmd x_orig y_orig

   if {$mode == "EDIT"} {
      switch $cmd {
         "ADD_NA210"  {do_add_na210    [snap_x $x0] [snap_y $y0]}
         "ADD_NO210"  {do_add_no210    [snap_x $x0] [snap_y $y0]}
         "ADD_EX210"  {do_add_ex210    [snap_x $x0] [snap_y $y0]}
         "ADD_IV110"  {do_add_iv110    [snap_x $x0] [snap_y $y0]}
         "ADD_GND"    {do_add_gnd_term [snap_x $x0] [snap_y $y0]}
         "ADD_AND2"   {do_add_and2     [snap_x $x0] [snap_y $y0]}
         "ADD_FADDER" {do_add_fadder   [snap_x $x0] [snap_y $y0]}
         "CONNECT"    {do_connect $x0 $y0}
         "MOVE"       {do_move $x0 $y0}
      }
   }
}

proc drag_commands {x0 y0} {
   global mode cmd

   if {$mode == "EDIT"} {
      switch $cmd {
         "ADD_NA210"  {do_drag_rect $x0 $y0}
         "ADD_NO210"  {do_drag_rect $x0 $y0}
         "ADD_EX210"  {do_drag_rect $x0 $y0}
         "ADD_IV110"  {do_drag_rect $x0 $y0}
         "ADD_GND"    {do_drag_rect $x0 $y0}
         "ADD_AND2"   {do_drag_rect $x0 $y0}
         "ADD_FADDER" {do_drag_rect $x0 $y0}
         "CONNECT"    {do_drag_line $x0 $y0}
         "MOVE"       {do_drag_rect $x0 $y0}
      }
   }
}

proc set_outline {x0 y0 dim_idx} {
   global GW CELL_W CELL_H PORT_W PORT_H cmd sel_tag

   switch $dim_idx {
   "no_dim"   { set sel_tag [lindex [.cv gettags [.cv find closest $x0 $y0]] 0]
                if {[string first "conn_" [lindex $sel_tag 0]] < 0} {
                set bbox [.cv bbox $sel_tag]
                .cv create rectangle [lindex $bbox 0] [lindex $bbox 1]\
                                     [lindex $bbox 2] [lindex $bbox 3]\
                                     -outline blue -tag outline
                }
              }
   "na210_dim"  { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-7*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+7*$GW/2.]\
                                       -outline blue -tag outline
                }
   "no210_dim"  { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-7*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+7*$GW/2.]\
                                       -outline blue -tag outline
                }
   "ex210_dim"  { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-7*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+7*$GW/2.]\
                                       -outline blue -tag outline
                }
   "and2_dim"   { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-7*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+7*$GW/2.]\
                                       -outline blue -tag outline
                }
   "fadder_dim" { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-7*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+7*$GW/2.]\
                                       -outline blue -tag outline
                }
   "iv110_dim"  { .cv create rectangle [expr $x0-7*$GW/2.]\
                                       [expr $y0-5*$GW/2.]\
                                       [expr $x0+7*$GW/2.]\
                                       [expr $y0+5*$GW/2.]\
                                       -outline blue -tag outline
                }
   "term_dim" { .cv create rectangle [expr $x0-$PORT_W] [expr $y0-$PORT_H]\
                           [expr $x0+$PORT_W] [expr $y0+$PORT_H]\
                           -outline blue -tag outline
              }
   }
}

proc set_tag {x0 y0} {
   global sel_tag  x_prev y_prev

   set sel_tag [lindex [.cv gettags [.cv find closest $x0 $y0]] 1]
   if {[string first "." $sel_tag] < 0} {
      set sel_tag ""
   }
   if {$sel_tag != ""} {
      set x_prev $x0
      set y_prev $y0
      .cv create line $x0 $y0 $x0 $y0 -fill blue -tag outline
   }
}

proc do_move {x0 y0} {
   global x_orig y_orig sel_tag N_CONN CONN_ARR GW

   if {[string first "conn_" [lindex $sel_tag 0]] >= 0} {
      return
   }

   .cv delete outline
   set x_move [expr $x0-$x_orig]
   set y_move [expr $y0-$y_orig]
   if {$sel_tag == "border"} {
      .cv move all $x_move $y_move
   }\
   else {
      set x_move [expr [snap_x $x0] - [snap_x $x_orig]]
      set y_move [expr [snap_y $y0] - [snap_y $y_orig]]
      .cv move $sel_tag $x_move $y_move
      for {set i [expr 0]} {$i <= $N_CONN } {incr i} {
         if {[string first $sel_tag $CONN_ARR($i)] >= 0} {
            update_conn $i
         }
      }
   }
}

proc do_drag_rect  {x0 y0} {
   global x_prev y_prev

   set dx [expr $x0-$x_prev]
   set dy [expr $y0-$y_prev]
   .cv move outline $dx $dy
   set x_prev $x0
   set y_prev $y0
}

proc do_drag_line {x0 y0} {
   global x_prev y_prev x_orig y_orig

   .cv coords outline $x_orig $y_orig $x0 $y0
   set x_prev $x0
   set y_prev $y0
}


proc delete_item {x0 y0} {
   global CELL_ARR CONN_ARR N_CELL N_CONN

   set del_tag [lindex [.cv gettags [.cv find closest $x0 $y0]] 0]
   if {$del_tag != "border"} {
      .cv delete $del_tag
      set idx [lindex [split $del_tag  "_"] 1]
      if {[string first "conn_" $del_tag] >= 0 } {
         set CONN_ARR($idx) ""
      }\
      else {
         set CELL_ARR($idx) ""
         for {set i [expr 0]} {$i <= $N_CONN} {incr i} {
            if {[string first $del_tag $CONN_ARR($i)] >= 0} {
               set CONN_ARR($i) ""
               .cv delete conn_$i
            }
         }
      }
   }
}

proc show_cells {} {
   global CELLS_MADE

   pack .fr1.rb5   -side left -padx 2 -pady 2
   pack .fr1.rb6   -side left -padx 2 -pady 2
   pack .fr1.rb7   -side left -padx 2 -pady 2
   pack .fr1.rb8   -side left -padx 2 -pady 2
   pack .fr1.rb9   -side left -padx 2 -pady 2
   if  {[lsearch $CELLS_MADE "and2"] >= 0} {
      pack .fr1.rb10  -side left -padx 2 -pady 2
   }
   if  {[lsearch $CELLS_MADE "fadder"] >= 0} {
      pack .fr1.rb11  -side left -padx 2 -pady 2
   }
}

proc hide_cells {} {
   pack forget .fr1.rb5   -side left -padx 2 -pady 2
   pack forget .fr1.rb6   -side left -padx 2 -pady 2
   pack forget .fr1.rb7   -side left -padx 2 -pady 2
   pack forget .fr1.rb8   -side left -padx 2 -pady 2
   pack forget .fr1.rb9   -side left -padx 2 -pady 2
   pack forget .fr1.rb10  -side left -padx 2 -pady 2
   pack forget .fr1.rb11  -side left -padx 2 -pady 2
}


proc add_user_name {circuit} {
   global env

   set fpi [open $circuit.eps "r"]
   set fpo [open $circuit.ps "w"]
   while {![eof $fpi]} {
      set txt [gets $fpi]
      if {$txt != "EndLayout"} {
         puts $fpo $txt
      }\
      else {
         puts $fpo "0 0 0 \($env(USER)\) plotText"
         puts $fpo "EndLayout"
      }
   }
   close $fpi
   close $fpo
}
