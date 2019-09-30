
proc op_mk_list_file {label_str} {
   global entity

   if {$label_str == ""} {
      echo "First make a choice !"
      return
   }
   set label [lindex [split $label_str ":()" ] 0]
   set ent_name [lindex [split $label_str ":()" ] 1]
   if {[winfo exists .main_pane.list] == 0} {
      set nw_list "True"
      view -icon list
   } else {
      set nw_list "False"
   }
   add list -ports /$entity/$label/*
   set types {
      {{LST Files} {.lst}}
   }
   set fname [tk_getSaveFile -filetypes $types -defaultextension ".lst"\
                             -initialfile "$ent_name.lst"]
   if {[file extension $fname] != ".lst"} {
      echo "wrong file_extension: must be lst"
   } else {
      write tssi $fname
   }
   if {$nw_list == "True"} {
      noview list
   }
   .op_sd deactivate
}

proc op_show_files {} {
   .op_sd clear items
   .op_sd clear selection
   set tmp_list [find instances *]

   foreach i $tmp_list {
      set label [lindex [split $i "/ " ] 2]
      set comp [lindex [split $i "()" ] 1]
      .op_sd insert items end $label:$comp
   }
   .op_sd activate
   raise .op_sd 
}

proc do_op_init {} {
   global OPPROGPATH DoFilePath auto_path op_init_done

   if {![info exists DoFilePath]} {
      if {[info exists ::env(OPPROGPATH)]} {
         set OPPROGPATH $::env(OPPROGPATH)
      } else {
         puts stderr "env OPPROGPATH not set"
	 exit 1
      }

      set DoFilePath $OPPROGPATH/do_files
      lappend auto_path $OPPROGPATH/src_libs/shared/comm
      lappend auto_path $OPPROGPATH/src_libs/shared/snit

      package require comm
      package require snit

      set fp_vsim_id [open "vsim_id" w]
      puts $fp_vsim_id "[comm::comm self]"
      close $fp_vsim_id
   }

   if {![winfo exists .mBar.tools.oP_Macros]} {
      add_separator "" tools
      add_submenu "" tools "OP_Macros"
      set do_file_list [glob $DoFilePath/*.do]
      foreach f $do_file_list {
         set f [file tail $f]
         if {$f != "init_simulator.do"} {
            add_menuitem "" tools.oP_Macros $f "do [file join $DoFilePath $f]"
         }
      }
   }
   if {![winfo exists .op_sd]} {
      iwidgets::selectiondialog .op_sd -selectionon false\
                                       -itemslabel "make listfile of"
      [.op_sd component selectionbox] configure -height 75
      .op_sd hide Apply
      .op_sd buttonconfigure OK -command {op_mk_list_file [.op_sd get]}
   }
   if {[winfo exists .main_pane.wave] == 0 || [winfo exists .main_pane.wave.mBar] == 0} {
      view -x 10 -y 340 -width 1000 -height 300 -undock wave
   }
   if {[.main_pane.wave.mBar.tools entrycget end -label] != "Make_list_file"} {
      add_separator .main_pane.wave Tools
      add_menuitem  .main_pane.wave Tools Make_list_file {op_show_files}
   }
   add wave /*
}

change_menu_cmd "" view Wave {do_op_init}
do_op_init
