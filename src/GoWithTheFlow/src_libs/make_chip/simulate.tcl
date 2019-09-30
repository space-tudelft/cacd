
proc do_compile {file_name} {
   global my_wd

   exec vcom -explicit $file_name
}

proc do_simulate {} {
   global my_wd cir_name PORT_ARR N_PORT fsim

   set fsim [open "|vsim -c $cir_name" "w+"]

   fconfigure $fsim -buffering line
   puts $fsim "echo DONE"
   while {1} {
      gets $fsim txt
      if {[lindex $txt 1] == "DONE"} {
         break
      }
   }
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      if {[lindex $PORT_ARR($i) 0] == "in"} {
         puts $fsim "force [lindex $PORT_ARR($i) 1] 0 0"
         puts $fsim "echo DONE"
         while {1} {
            gets $fsim txt
            if {[lindex $txt 1] == "DONE"} {
               break
            }
         }
        .cv itemconfigure $cir_name.[lindex $PORT_ARR($i) 1] -fill blue
       .cv bind $cir_name.[lindex $PORT_ARR($i) 1] <ButtonPress-1> "set_input [lindex $PORT_ARR($i) 1]"
      }
   }
   puts $fsim "run 1 us"
   puts $fsim "echo DONE"
   while {1} {
      gets $fsim txt
      if {[lindex $txt 1] == "DONE"} {
         break
      }
   }
   update_instances
   update_outputs
}

proc update_outputs {} {
   global cir_name PORT_ARR N_PORT fsim

   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      if {[lindex $PORT_ARR($i) 0] == "out"} {
         puts $fsim "examine /[lindex $PORT_ARR($i) 1]"
         puts $fsim "echo DONE"
         while {1} {
            gets $fsim txt
            if {[lindex $txt 1] == "DONE"} {
               break
            }\
            elseif {[lindex $txt 0] == "#"} {
               set cur_val [lindex $txt 1]
               if {$cur_val == 1} {
                  .cv itemconfigure $cir_name.[lindex $PORT_ARR($i) 1] -fill red
               }\
               elseif {$cur_val == 0} {
                  .cv itemconfigure $cir_name.[lindex $PORT_ARR($i) 1] -fill blue
               }\
               else {
                  .cv itemconfigure $cir_name.[lindex $PORT_ARR($i) 1] -fill wheat3
               }
            }
         }
      }
      update
   }
}

proc update_instances {} {
   global N_CELL CELL_ARR fsim

   for {set i [expr 0]} {$i <= $N_CELL} {incr i} {
      if {$CELL_ARR($i) != ""} {
         puts $fsim "examine -name $CELL_ARR($i)/*"
         puts $fsim "echo DONE"
         gets $fsim txt
         while {1} {
            gets $fsim txt
            if {[lindex $txt 1] == "DONE"} {
               break
            }
            foreach ii $txt {
               if { $ii == "#"} {
                  continue
               }
               set tname [lindex $ii 0]
               set tmpterm [string toupper [file tail $tname]]
               set cur_val [lindex $ii 1]
               if {$cur_val == 1} {
                  .cv itemconfigure $CELL_ARR($i).$tmpterm -fill red
               }\
               elseif {$cur_val == 0} {
                  .cv itemconfigure $CELL_ARR($i).$tmpterm -fill blue
               }\
               else {
                  .cv itemconfigure $CELL_ARR($i).$tmpterm -fill wheat3
               }
            }
         }
      }
   }
}

proc set_input {name} {
   global fsim cir_name SIM_TYPE


   if {[.cv itemcget $cir_name.$name -fill] == "red"} {
      .cv itemconfigure $cir_name.$name -fill blue
      update
      if {$SIM_TYPE == "INTERN"} {
         update_signal $name 0
      }\
      else {
         puts $fsim "force /$name 0 0"
      }
   }\
   else {
      .cv itemconfigure $cir_name.$name -fill red
      update
      if {$SIM_TYPE == "INTERN"} {
         update_signal $name 1
      }\
      else {
         puts $fsim "force /$name 1 0"
      }
   }
   if {$SIM_TYPE == "VHDL"} {
      puts $fsim "echo DONE"
      while {1} {
         gets $fsim txt
         if {[lindex $txt 1] == "DONE"} {
            break
         }
      }
      puts $fsim "run 1 us"
      puts $fsim "echo DONE"
      while {1} {
         gets $fsim txt
         if {[lindex $txt 1] == "DONE"} {
            break
         }
      }
      update_instances
      update_outputs
   }
}
