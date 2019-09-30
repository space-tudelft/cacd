
proc delete_design {type index} {
#-----------------------------------------------------------------------------#
# interface for a procedure to delete (part of) a design from the design_data.#
#-----------------------------------------------------------------------------#
   global Fnt Aarr Carr Earr Parr SimLibName DbName topmostWindow

   switch $type {
     A   { set aname [lindex $Aarr($index) 0]
           set e_idx [lindex $Aarr($index) 1]
           set ename [lindex $Earr($e_idx) 0]
           set name $ename\($aname\)
         }
     E   { set ename [lindex $Earr($index) 0]
           set first_char [string toupper [string index $ename 0]]
           set extr_name "$first_char[string range $ename 1 end]"
           set name $ename
         }
     C   { set name [lindex $Carr($index) 0] }
     P   { set name [lindex $Parr($index) 0] }
   }
   set label_str "delete $name from library: $SimLibName"

   toplevel    .del
   frame       .del.frl     -bd 2 -relief ridge -bg wheat
   label       .del.frl.lbl -text $label_str -font $Fnt -bg wheat
   if {$type == "E"} {
      checkbutton .del.frl.wl  -text $SimLibName -font $Fnt -bg wheat\
                               -anchor w\
                               -command {
                                  if {$wl} {
                                     if {[winfo exists .del.frl.cl]} {
                                        .del.frl.cl select
                                        .del.frl.cl configure -state disabled
                                     }
                                     if {[winfo exists .del.frl.ll]} {
                                        .del.frl.ll select
                                        .del.frl.ll configure -state disabled
                                     }
                                     if {[winfo exists .del.frl.el]} {
                                        .del.frl.el select
                                        .del.frl.el configure -state disabled
                                     }
                                  } else {
                                     if {[winfo exists .del.frl.cl]} {
                                        .del.frl.cl configure -state normal
                                     }
                                  }
                                }
      .del.frl.wl select
      frame       .del.frf     -bd 2 -relief ridge -bg wheat
      label       .del.frf.lbl -text "delete file:" -font $Fnt -bg wheat
   }
   frame       .del.frt       -bd 2 -relief ridge -bg wheat
   text        .del.frt.txt   -width 70 -height 10 -font $Fnt\
                            -yscrollcommand ".del.frt.sb set"\
                            -bg wheat -padx 10
   scrollbar   .del.frt.sb    -command ".del.frt.txt yview" -bg wheat
   frame       .del.frc     -bd 2 -relief ridge -bg gold
   button      .del.frc.ok  -text "Delete" -font $Fnt -bg gold\
                            -command "delete_spec_items $type $index $name"
   button      .del.frc.qt  -text "Cancel" -font $Fnt -bg gold\
                            -command {destroy .del}

   pack .del.frl     -side top    -fill x
   pack .del.frl.lbl -side top    -fill x
   if {$type == "E"} {
      pack .del.frl.wl  -side top    -fill x -padx 10 -anchor w
      pack .del.frf     -side top    -fill x -fill both -expand 1
      pack .del.frf.lbl -side top    -fill x
   }
   pack .del.frt     -side top    -fill both -expand 1
   pack .del.frt.txt -side left   -fill both -expand 1
   pack .del.frt.sb  -side right  -fill y
   pack .del.frc     -side bottom -fill x
   pack .del.frc.ok  -side left   -padx 20 -pady 3 -fill x -expand 1
   pack .del.frc.qt  -side right  -padx 20 -pady 3 -fill x -expand 1

   if {$type == "E"} {
      if {[nelsis_cell_exists $ename $DbName circuit]} {
         checkbutton .del.frl.cl -text "$DbName (circuit)"\
                                 -font $Fnt -bg wheat -anchor w\
                                 -command {
                                     if {$cl} {
                                        if {[winfo exists .del.frl.ll]} {
                                           .del.frl.ll select
                                           .del.frl.ll configure -state disabled
                                        }
                                        if {[winfo exists .del.frl.el]} {
                                           .del.frl.el select
                                           .del.frl.el configure -state disabled
                                        }
                                     } else {
                                        if {[winfo exists .del.frl.ll]} {
                                           .del.frl.ll configure -state normal
                                        }
                                     }
                                  }
         pack .del.frl.cl -side top -fill x -padx 10 -anchor w
      }
      if {[nelsis_cell_exists $ename $DbName layout]} {
         checkbutton .del.frl.ll  -text "$DbName (layout)"\
                                  -font $Fnt -bg wheat -anchor w\
                                  -command {
                                     if {$ll} {
                                        if {[winfo exists .del.frl.el]} {
                                           .del.frl.el select
                                           .del.frl.el configure -state disabled
                                        }
                                     } else {
                                        if {[winfo exists .del.frl.el]} {
                                           .del.frl.el configure -state normal
                                        }
                                     }
                                   }
         pack .del.frl.ll -side top -fill x -padx 10 -anchor w
      }
      if {[nelsis_cell_exists $extr_name $DbName circuit]} {
         checkbutton .del.frl.el  -text "$DbName (extracted circuit)"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frl.el -side top -fill x -padx 10 -anchor w
      }
      if {[file exists ./SLS/$ename.sls]} {
         checkbutton .del.frf.fs  -text "./SLS/$ename.sls"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frf.fs -side top -fill x -padx 10 -anchor w
      }
      if {[file exists ./ADB/$ename.ddc]} {
         checkbutton .del.frf.fdb -text "./ADB/$ename.ddc"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frf.fdb -side top -fill x -padx 10 -anchor w
      }
      if {[file exists ./.$ename.term]} {
         checkbutton .del.frf.ftrm -text "./.$ename.term"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frf.ftrm -side top -fill x -padx 10 -anchor w
      }
      if {[file exists ./circuits/$ename.cir]} {
         checkbutton .del.frf.fcir -text "./circuits/$ename.cir"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frf.fcir -side top -fill x -padx 10 -anchor w
      }
      if {[file exists ./components/$ename.cmp]} {
         checkbutton .del.frf.fcmp -text "./components/$ename.cmp"\
                                  -font $Fnt -bg wheat -anchor w
         pack .del.frf.fcmp -side top -fill x -padx 10 -anchor w
      }
   }
   wm attributes .del -topmost 1; set topmostWindow .del
   wm title .del "delete $name"
   grab set .del
   tkwait window .del
}

proc delete_spec_items {type index name} {
#-----------------------------------------------------------------------------#
# this proc determines the delete function to carry out according to the type #
#-----------------------------------------------------------------------------#
   global posXY

   .del.frc.ok configure -state disabled

   set posXY 0
   switch $type {
     A { set res [delete_architecture  $index]}
     C { set res [delete_configuration $index]}
     E { set res [delete_entity  $index]; set name "entity $name"}
     P { set res [delete_package $index]; set name "package $name"}
   }
   unset posXY

   if {$res == 0} {
      upd_status_line "delete of $name successfully DONE"
    # destroy .del
      destroy .del.frl.wl
      add_mess .del.frt.txt "== deletion successfully DONE =="
   } else {
      add_mess .del.frt.txt "== deletion stopped UNCOMPLETED =="
   }
   read_infofile
}

proc delete_db_cell {cell view} {
   global DbName

   goto_dbdir
   upd_status_line "deleting $view $cell ....."
   catch {exec rmdb -f -c $cell $view} err_str
   if {$err_str != ""} {
      upd_status_line "$err_str"
   } else {
      upd_status_line "-- $view $cell removed from database $DbName --"
   }
   restore_cwd
   read_infofile
}

proc delete_entity {index} {
#-----------------------------------------------------------------------------#
# procedure to delete an entity of a design from the design_data.             #
# also the circuit and layout parts of the entity in the nelsis_database can  #
# be deleted with this function.                                              #
#-----------------------------------------------------------------------------#
   global DbName Earr SimLibName

   upvar #0 wl wl
   upvar #0 el el
   upvar #0 ll ll
   upvar #0 cl cl
   upvar #0 fs fs
   upvar #0 fdb fdb
   upvar #0 fcir fcir
   upvar #0 fcmp fcmp
   upvar #0 ftrm ftrm

   set ename [lindex $Earr($index) 0]
   if {[file isdir $DbName]} {
      goto_dbdir
      if {[winfo exists .del.frl.el] && $el} {
         set first_char [string toupper [string index $ename 0]]
         set extr_name "$first_char[string range $ename 1 end]"
         add_mess .del.frt.txt "deleting circuit $extr_name ....."
         catch {exec rmdb -f -c $extr_name circuit} err_str
         if {$err_str != ""} {
            add_mess .del.frt.txt "$err_str"
            add_mess .del.frt.txt "*** $extr_name not removed ***"
            restore_cwd
            return 1
         } else {
            destroy .del.frl.el
            add_mess .del.frt.txt "-- circuit $extr_name removed from database $DbName --"
         }
      }
      if {[winfo exists .del.frl.ll] && $ll} {
         add_mess .del.frt.txt "deleting layout $ename ....."
         catch {exec rmdb -f -c $ename layout} err_str
         if {$err_str != ""} {
            add_mess .del.frt.txt "$err_str"
            add_mess .del.frt.txt "*** $ename not removed ***"
            restore_cwd
            return 1
         } else {
            destroy .del.frl.ll
            add_mess .del.frt.txt "-- layout $ename removed from database $DbName --"
         }
      }
      if {[winfo exists .del.frl.cl] && $cl} {
         add_mess .del.frt.txt "deleting circuit $ename ....."
         catch {exec rmdb -f -c $ename circuit} err_str
         if {$err_str != ""} {
            add_mess .del.frt.txt "$err_str"
            add_mess .del.frt.txt "*** $ename not removed ***"
            restore_cwd
            return 1
         } else {
            destroy .del.frl.cl
            add_mess .del.frt.txt "-- circuit $ename removed from database $DbName --"
         }
      }
      restore_cwd
   }
   if {[winfo exists .del.frf.fs] && $fs} {
      file delete ./SLS/$ename.sls
      destroy .del.frf.fs
      add_mess .del.frt.txt "-- file ./SLS/$ename.sls removed --"
   }
   if {[winfo exists .del.frf.fdb] && $fdb} {
      file delete ./ADB/$ename.ddc
      destroy .del.frf.fdb
      add_mess .del.frt.txt "-- file ./ADB/$ename.ddc removed --"
   }
   if {[winfo exists .del.frf.ftrm] && $ftrm} {
      file delete ./.$ename.term
      destroy .del.frf.ftrm
      add_mess .del.frt.txt "-- file ./.$ename.term removed --"
   }
   if {[winfo exists .del.frf.fcir] && $fcir} {
      file delete ./circuits/$ename.cir
      destroy .del.frf.fcir
      add_mess .del.frt.txt "-- file ./circuits/$ename.cir removed --"
   }
   if {[winfo exists .del.frf.fcmp] && $fcmp} {
      destroy .del.frf.fcmp
      file delete ./components/$ename.cmp
      add_mess .del.frt.txt "-- file ./components/$ename.cmp removed --"
   }
   if {[winfo exists .del.frl.wl] && $wl} {
      add_mess .del.frt.txt "   deleting entity $ename ..."
      set cfg_parents [find_entity_uses $ename]
      if {$cfg_parents != ""} {
         add_mess .del.frt.txt\
                 " *** you cannot delete entity $ename ***\n\
                   *** it is in use in the configuration(s) ***"
         foreach cfg $cfg_parents {
            add_mess .del.frt.txt "*** $cfg"
         }
         return 1
      }\
      else {
         for {set i 2} {$i < [llength $Earr($index)]} {incr i} {
            delete_architecture [lindex $Earr($index) $i]
         }
         exec vdel -lib $SimLibName $ename
         add_mess .del.frt.txt "-- entity $ename deleted --"
         set answ [df_choise_dialog\
              " should the vhdl_file:\n      [lindex $Earr($index) 1]\n\
                from which the deleted entity was built,\n\
                also be deleted ?"]
         if {$answ == "yes"} { file delete [lindex $Earr($index) 1]
	    add_mess .del.frt.txt "-- file [lindex $Earr($index) 1] deleted --"
	 }
      }
   }
   return 0
}

proc delete_configuration {idx} {
#-----------------------------------------------------------------------------#
# procedure to delete a configuration of a design from the design_data.       #
#-----------------------------------------------------------------------------#
   global Carr SimLibName

   set cfg_name [lindex $Carr($idx) 0]
   add_mess .del.frt.txt "   deleting config $cfg_name ..."
   set cfg_parents [find_config_uses $cfg_name]
   if {$cfg_parents != ""} {
      add_mess .del.frt.txt\
              " *** you cannot delete configuration $cfg_name\n\
                *** it is in use in the configuration(s)"
      foreach cfg $cfg_parents {
         add_mess .del.frt.txt "       $cfg"
      }
      return 1
   }\
   else {
      exec vdel -lib $SimLibName $cfg_name
      add_mess .del.frt.txt "-- configuration $cfg_name deleted --"
      set answ [df_choise_dialog\
              " should the vhdl_file:\n      [lindex $Carr($idx) 3]\n\
                from which the deleted configuration was built,\n\
                also be deleted ?"]
      if {$answ == "yes"} { file delete [lindex $Carr($idx) 3]
	 add_mess .del.frt.txt "-- file [lindex $Carr($idx) 3] deleted --"
      }
   }
   return 0
}

proc delete_architecture {idx} {
#-----------------------------------------------------------------------------#
# procedure to delete a configuration of a design from the design_data.       #
#-----------------------------------------------------------------------------#
   global Aarr Earr SimLibName

   set aname [lindex $Aarr($idx) 0]
   set e_idx [lindex $Aarr($idx) 1]
   set ename [lindex $Earr($e_idx) 0]
   add_mess .del.frt.txt "   deleting architecture $aname of $ename ..."
   set cfg_parents [find_arch_uses $ename $aname]
   if {$cfg_parents != ""} {
      add_mess .del.frt.txt\
              " *** you cannot delete architecture $aname\n\
                *** it is in use in the configuration(s)"
      foreach cfg $cfg_parents {
         add_mess .del.frt.txt "       $cfg"
      }
      return 1
   }\
   else {
      foreach ac [find_arch_configs $ename $aname] {
         delete_configuration $ac
      }
      exec vdel -lib $SimLibName $ename $aname
      add_mess .del.frt.txt "-- architecture $aname of $ename deleted --"
      set answ [df_choise_dialog\
              " should the vhdl_file:\n      [lindex $Aarr($idx) 2]\n\
                from which the deleted architecture was built,\n\
                also be deleted ?"]
      if {$answ == "yes"} { file delete [lindex $Aarr($idx) 2]
	 add_mess .del.frt.txt "-- file [lindex $Aarr($idx) 2] deleted --"
      }
   }
   return 0
}

proc delete_package {idx} {
#-----------------------------------------------------------------------------#
# procedure to delete a package of a design from the design_data.             #
#-----------------------------------------------------------------------------#
   global Parr SimLibName

   set pname [lindex $Parr($idx) 0]
   set pack_uses [find_pack_uses $idx]
   set nr 0
   foreach pack $pack_uses {
      if {$pack != "body"} { incr nr }
   }
   if {$nr > 0} {
      add_mess .del.frt.txt\
              " *** you cannot delete package $pname\n\
                *** it is in use in the design_parts:"
      foreach pack $pack_uses {
	 if {$pack != "body"} {
	    add_mess .del.frt.txt "       $pack"
	 }
      }
      return 1
   }

   lappend files [lindex $Parr($idx) 1]
   if {$pack_uses != ""} {
      set msg ""
      if {[llength $Parr($idx)] == 3} {
	 set bname [lindex $Parr($idx) 2]
	 if {[string index $bname 0] == "B"} {
	    set idx [string range $bname 1 end]
	    lappend files [lindex $Parr($idx) 1]
	 } else { set msg "B not found" }
      } else { set msg "llength != 3" }
      if {$msg != ""} {
	 add_mess .del.frt.txt\
		 " *** you cannot delete package $pname\n\
		   *** because of body problem: $msg"
	 return 1
      }
   }
   if {1} {
      add_mess .del.frt.txt "   deleting package $pname ..."
      exec vdel -lib $SimLibName $pname
      add_mess .del.frt.txt "-- package $pname deleted --"
      set b ""
      foreach fn $files {
	 set answ [df_choise_dialog\
		 " should the vhdl_file:\n      $fn\n\
		   from which the package$b was built,\n\
		   also be deleted ?"]
	 if {$answ == "yes"} { file delete $fn
	    add_mess .del.frt.txt "-- file $fn deleted --"
	 }
	 set b "_body"
      }
   }
   return 0
}
