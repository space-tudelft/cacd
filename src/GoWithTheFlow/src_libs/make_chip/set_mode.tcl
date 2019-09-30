
proc set_edit_mode {} {
   global fsim N_CELL CELL_ARR SUBTERM_ARR PORT_ARR N_PORT cir_name SOG_DB

   if {$fsim != ""} {
      puts $fsim "quit"
      set fsim ""
   }
   for {set i [expr 0]} {$i <= $N_CELL} {incr i} {
      set inst $CELL_ARR($i)
      if {$inst != ""} {
         set cmp [lindex [split $inst _] 0]
         if {$cmp != "vss"} {
            foreach ii $SUBTERM_ARR($cmp) {
               .cv itemconfigure $inst.$ii -fill wheat3
            }
         }
      }
   }
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      if {$PORT_ARR($i) != ""} {
         .cv itemconfigure P_[lindex $PORT_ARR($i) 1] -fill wheat3
         .cv itemconfigure T_[lindex $PORT_ARR($i) 1] -fill black
      }
   }
   for {set i [expr 1]} {$i < [expr 11]} {incr i} {
      .fr1.rb$i configure -state normal
   }
   .fr2.file configure -state normal
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      .cv bind $cir_name.[lindex $PORT_ARR($i) 1] <ButtonPress-1> ""
   }
   if {[file exists $SOG_DB/layout/$cir_name] == 1} {
      .fr2.file.cmds entryconfigure PrintLayout -state normal
   }\
   else {
      .fr2.file.cmds entryconfigure PrintLayout -state disabled
   }
}

proc set_simulation_mode {} {
   global cir_name N_PORT PORT_ARR SIM_TYPE

   for {set i [expr 1]} {$i < [expr 11]} {incr i} {
      .fr1.rb$i configure -state disabled
   }
   .fr2.file configure -state disabled
   update
   write_cir_file
   cir2vhd $cir_name
   if {$SIM_TYPE == "VHDL"} {
      do_compile $cir_name.vhd
      do_simulate
   }\
   else {
      newsim $cir_name.vhd
      for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
         if {[lindex $PORT_ARR($i) 0] == "in"} {
           .cv itemconfigure $cir_name.[lindex $PORT_ARR($i) 1] -fill blue
           .cv bind $cir_name.[lindex $PORT_ARR($i) 1] <ButtonPress-1>\
                                         "set_input [lindex $PORT_ARR($i) 1]"
            update_signal [lindex $PORT_ARR($i) 1] 0
         }
      }
      update_signal vss 0
   }
}

proc set_layout_mode {} {
   global cir_name my_wd SOG_DB fsim N_PORT PORT_ARR N_CELL CELL_ARR
   global SUBTERM_ARR env

   if {$fsim != ""} {
      puts $fsim "quit"
      set fsim ""
   }
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      .cv bind $cir_name.[lindex $PORT_ARR($i) 1] <ButtonPress-1> ""
   }
   for {set i [expr 1]} {$i < [expr 11]} {incr i} {
      .fr1.rb$i configure -state disabled
   }
   .fr2.file configure -state disabled
   for {set i [expr 0]} {$i <= $N_CELL} {incr i} {
      set inst $CELL_ARR($i)
      if {$inst != ""} {
         set cmp [lindex [split $inst _] 0]
         if {$cmp != "vss"} {
            foreach ii $SUBTERM_ARR($cmp) {
               .cv itemconfigure $inst.$ii -fill wheat3
            }
         }
      }
   }
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      if {$PORT_ARR($i) != ""} {
         .cv itemconfigure P_[lindex $PORT_ARR($i) 1] -fill wheat3
         .cv itemconfigure T_[lindex $PORT_ARR($i) 1] -fill black
      }
   }
   update
   write_cir_file
   cir2sls $cir_name
   cd [pwd]/$SOG_DB
   set env(CWD) [pwd]
   set res [catch {exec csls ../$cir_name.sls} mess]
   puts "$mess"
   exec seadali &
   cd $my_wd
   set env(CWD) [pwd]
}
