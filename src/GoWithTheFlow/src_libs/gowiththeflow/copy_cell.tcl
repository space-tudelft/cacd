
proc copy_cell {} {
#-----------------------------------------------------------------------------#
# procedure to make a window to copy cells from a different database.         #
#-----------------------------------------------------------------------------#
   global Fnt topmostWindow MyWd

   toplevel  .cpy
   frame     .cpy.frm       -bd 2 -relief ridge -bg wheat
   label     .cpy.frm.lbl   -text "project directory to copy from:" -font $Fnt -bg wheat
   entry     .cpy.frm.en    -width 50 -font $Fnt -bg wheat
   button    .cpy.frm.br    -text "Browse" -font $Fnt -bg wheat\
                            -command {set_copy_dir}
   button    .cpy.frm.ul    -text "Unlock Copy Directory" -font $Fnt -bg wheat\
                            -command {unlock_copy_dir}
   button    .cpy.frm.ok    -text "OK" -font $Fnt -bg wheat\
                            -command {accept_copy_dir}
   frame     .cpy.frt       -bd 2 -relief ridge -bg wheat
   text      .cpy.frt.txt   -width 50 -height 10 -font $Fnt\
                            -yscrollcommand ".cpy.frt.sb set"\
                            -bg wheat -padx 10
   scrollbar .cpy.frt.sb    -command ".cpy.frt.txt yview" -bg wheat
   frame     .cpy.frc       -bd 2 -relief ridge -bg gold
   button    .cpy.frc.ok    -text "Copy" -font $Fnt -bg gold -state disabled\
                            -command {copy_spec_items}
   checkbutton .cpy.frc.st  -text "Stop" -font $Fnt -bg gold -variable StopRequested\
                            -indicatoron False -pady 5\
                            -selectcolor gold
   button      .cpy.frc.qt  -text "Cancel" -font $Fnt -bg gold -command {destroy .cpy}

   pack .cpy.frm     -side top    -fill x
   pack .cpy.frm.lbl -side top    -fill x
   pack .cpy.frm.en  -side top    -fill x -padx 5
   pack .cpy.frm.br  -side left   -padx 50 -pady 3
   pack .cpy.frm.ul  -side left   -padx 50 -pady 3
   pack .cpy.frm.ok  -side right  -padx 20 -pady 3
   pack .cpy.frt     -side top    -fill both -expand 1
   pack .cpy.frt.txt -side left   -fill both -expand 1
   pack .cpy.frt.sb  -side right  -fill y
   pack .cpy.frc     -side bottom -fill x
   pack .cpy.frc.ok  -side left   -padx 20 -pady 3 -fill x -expand 1
   pack .cpy.frc.st  -side left   -padx 20 -pady 3 -fill x -expand 1
   pack .cpy.frc.qt  -side right  -padx 20 -pady 3 -fill x -expand 1

   cd $MyWd/..
   .cpy.frm.en insert end [pwd]
   cd $MyWd

   wm attributes .cpy -topmost 1; set topmostWindow .cpy
   wm title .cpy "import cell"
   grab set .cpy
   tkwait window .cpy
}

proc goto_copy_dbdir {copy_dir} {
#-----------------------------------------------------------------------------#
# procedure to set the CWD to a database from which cells must be copied      #
#-----------------------------------------------------------------------------#
   global env

   set lst ""
   set lst [lappend lst CWD]
   set lst [lappend lst $copy_dir]
   array set env $lst
   cd $env(CWD)
}

proc get_subcells {cell} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the components present in an entity.            #
#-----------------------------------------------------------------------------#
   upvar #0 Wlib wlib
   upvar subcells subcells
   upvar packages packages

   set entity 0
   set fp [open [.cpy.frm.en get]/$wlib/_info "r"]

   while {![eof $fp]} {
      set txt [gets $fp]

      switch [string index $txt 0] {
	Z { set i [string first " " $txt]
	    set n [string range $txt 1 $i-1]
	    set txt [string range $txt $i+1 end]
	    switch -glob -- $txt {
		e*  { if {$txt != "e$cell"} continue }
		DE* { if {[lindex $txt 1] != $wlib} continue }
		DP* { if {[lindex $txt 1] != $wlib} continue }
		default { continue }
	    }
	    set Z($n) $txt
	  }
	R { set n [string range $txt 1 end]
	    if {![info exist Z($n)]} continue
	    set txt $Z($n)
	  }
      }

      switch [string index $txt 0] {
	E { if {$txt == "E$cell"} { set entity 1 } { set entity 0 } }
	P { set entity 0 }
	C { set entity 0 }
	e { if {$txt == "e$cell"} { set entity 1 } }
	D { if {!$entity} continue
	    set c [string index $txt 1]
	    if {($c == "E" || $c == "P") && [lindex $txt 1] == $wlib} {
		set comp [lindex $txt 3]
		if {$comp == $cell} continue
		if {$c == "E" && [lsearch $subcells $comp] < 0} { lappend subcells $comp }
		if {$c == "P" && [lsearch $packages $comp] < 0} { lappend packages $comp }
	    }
	  }
      }
   }
   close $fp
}

proc cell_in_working_lib {cell} {
#-----------------------------------------------------------------------------#
# procedure to see if an entity is already present in the working library.    #
#-----------------------------------------------------------------------------#
   if {[get_earr_idx $cell] >= 0} { return 1 }
   return 0
}

proc package_in_working_lib {package} {
#-----------------------------------------------------------------------------#
# procedure to see if a package is already present in the working library.    #
#-----------------------------------------------------------------------------#
   if {[get_parr_idx $package] >= 0} { return 1 }
   return 0
}

proc nelsis_cell_exists {cell nlib type} {
#-----------------------------------------------------------------------------#
# procedure to see if a cell exists in the given part of a nelsis_database.   #
#-----------------------------------------------------------------------------#
   if {$nlib == ""} { return 0 }
   if {[catch {open $nlib/$type/celllist} fp]} { return 0 }
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == $cell} { close $fp; return 1 }
   }
   close $fp
   return 0
}

proc set_mess {str colour} {
#-----------------------------------------------------------------------------#
# procedure to set a message to the message_window.                           #
#-----------------------------------------------------------------------------#
   .cpy.frt.txt delete 1.0 end
   .cpy.frt.txt insert end $str
   .cpy.frt.txt tag add mess_tag 1.0 end
   .cpy.frt.txt tag configure mess_tag -foreground $colour
}

proc add_mess {txt_wdw str} {
#-----------------------------------------------------------------------------#
# procedure to add a message to the message_window.                           #
#-----------------------------------------------------------------------------#
   $txt_wdw insert end "$str\n"
   $txt_wdw see end
   update
}

proc set_copy_dir {} {
#-----------------------------------------------------------------------------#
# procedure to set the directory to copy from.                                #
#-----------------------------------------------------------------------------#
   global MyWd
   set_mess "" black
   set dir [tk_chooseDirectory -initialdir $MyWd/.. -mustexist true -parent .cpy]
   if {$dir != ""} {
      .cpy.frm.en delete 0 end
      .cpy.frm.en insert end $dir
   }
}

proc get_vhdl_working_libs {} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the working libraries in the chosen directory   #
# to copy from and show it as a choice_menu.                                  #
#-----------------------------------------------------------------------------#
   global Fnt
   upvar #0 Wlib wlib

   set wdirs ""
   set copy_dir [.cpy.frm.en get]
   foreach dir [glob -nocomplain -type d $copy_dir/*] {
      if {[file exists $dir/_info]} { lappend wdirs [file tail $dir] }
   }
   if {$wdirs == ""} { return 0 }
   catch {destroy .cpy.frwl}
   catch {destroy .cpy.fre}
   catch {destroy .cpy.frf}
   set wlib [lindex $wdirs 0]
   frame .cpy.frwl     -bd 2 -relief ridge -bg wheat
   label .cpy.frwl.lbl -text "working library:" -font $Fnt -bg wheat
   eval tk_optionMenu .cpy.frwl.wl Wlib $wdirs
   pack .cpy.frwl -side top -fill x -after .cpy.frm
   pack .cpy.frwl.lbl -side left -padx 20
   pack .cpy.frwl.wl -side right -padx 20 -pady 3
   .cpy.frwl.wl configure -bg wheat -font $Fnt
   .cpy.frwl.wl.menu configure -bg wheat -font $Fnt
   for {set i 0} {$i <= [.cpy.frwl.wl.menu index end]} {incr i} {
      .cpy.frwl.wl.menu entryconfigure $i -command {accept_wlib}
   }
   accept_wlib
   return 1
}

proc accept_nlib {} {
#-----------------------------------------------------------------------------#
# procedure that is carried out when a nelsis_library to copy from is chosen. #
#-----------------------------------------------------------------------------#
   global Fnt
   upvar #0 Nlib nlib
   upvar #0 EntCopy ent2copy

   catch {destroy .cpy.frnl.cb_c}
   catch {destroy .cpy.frnl.cb_l}
   if {[nelsis_cell_exists $ent2copy [.cpy.frm.en get]/$nlib circuit]} {
      checkbutton .cpy.frnl.cb_c -text circuit -bg wheat -font $Fnt
      pack .cpy.frnl.cb_c -side left
      .cpy.frnl.cb_c select
   }
   if {[nelsis_cell_exists $ent2copy [.cpy.frm.en get]/$nlib layout]} {
      checkbutton .cpy.frnl.cb_l -text layout -bg wheat -font $Fnt
      pack .cpy.frnl.cb_l -side left
      .cpy.frnl.cb_l select
   }
}

proc get_nelsis_libs {} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the nelsis libraries in the chosen directory    #
# to copy from and show it as a choice_menu.                                  #
#-----------------------------------------------------------------------------#
   global Fnt
   upvar #0 Nlib nlib
   upvar #0 EntCopy ent2copy

   set ndirs ""
   set copy_dir [.cpy.frm.en get]
   foreach dir [glob -nocomplain -type d $copy_dir/*] {
      if {[file exists $dir/.dmrc]} {
         if {[nelsis_cell_exists $ent2copy $dir circuit] ||
             [nelsis_cell_exists $ent2copy $dir layout]} {
            lappend ndirs [file tail $dir]
         }
      }
   }
   catch {destroy .cpy.frnl}
   if {[llength $ndirs] == 0} return
   set nlib [lindex $ndirs 0]
   frame .cpy.frnl     -bd 2 -relief ridge -bg wheat
   frame .cpy.frnl.fr  -bg wheat
   label .cpy.frnl.fr.lbl -text "nelsis library:" -font $Fnt -bg wheat
   eval tk_optionMenu .cpy.frnl.fr.nl Nlib $ndirs
   pack .cpy.frnl -side top -fill x -after .cpy.frf
   pack .cpy.frnl.fr -side top -fill x
   pack .cpy.frnl.fr.lbl -side left -padx 20
   pack .cpy.frnl.fr.nl -side right -padx 20 -pady 3
   .cpy.frnl.fr.nl configure -bg wheat -font $Fnt
   .cpy.frnl.fr.nl.menu configure -bg wheat -font $Fnt
   for {set i 0} {$i <= [.cpy.frnl.fr.nl.menu index end]} {incr i} {
      .cpy.frnl.fr.nl.menu entryconfigure $i -command {accept_nlib}
   }
   accept_nlib
}

proc accept_entity {} {
#-----------------------------------------------------------------------------#
# procedure that is carried out when the entity to copy is chosen.            #
#-----------------------------------------------------------------------------#
   global Fnt
   upvar #0 EntCopy ent2copy
   upvar #0 Wlib wlib

   set from [.cpy.frm.en get]
   set found 0
   set vhdl_file_list ""

   set fp [open "$from/$wlib/_info" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      switch [string index $txt 0] {
	E  { set found 0; if {$txt == "E$ent2copy"} { incr found; .cpy.fre.lbl configure -text "entity cell:" } }
	P  { set found 0; if {$txt == "P$ent2copy"} { incr found; .cpy.fre.lbl configure -text "package cell:" } }
	C  { set found 0 }
	e  { if {$txt == "e$ent2copy"} { incr found } }
	F  { if {!$found} continue
	     set fname [string range $txt 1 end]
	     if {[lsearch $vhdl_file_list $fname] < 0} { lappend vhdl_file_list $fname }
	   }
	Z  { set i [string first " " $txt]
	     set s [string range $txt $i+1 end]
	     switch [string index $s 0] {
		e { if {$s == "e$ent2copy"} { incr found; set Z([string range $txt 1 $i-1]) e }
		    continue
		  }
		F { if {!$found} continue
		    set fname [string range $s 1 end]
		  }
		! { if {!$found} continue
		    if {[lindex $s 0] != "!s107"} continue
		    set fname [string trimright [lindex $s 1] |]
		  }
		default { continue }
	     }
	     if {[lsearch $vhdl_file_list $fname] < 0} { lappend vhdl_file_list $fname }
	   }
	R  { if {[info exist Z([string range $txt 1 end])]} { incr found } }
      }
   }
   close $fp

   catch {destroy .cpy.frf}
   frame .cpy.frf     -bd 2 -relief ridge -bg wheat
   frame .cpy.frf.frv -bg wheat
   label .cpy.frf.frv.lbl -text "vhdl_files" -font $Fnt -bg wheat
   frame .cpy.frf.fro -bg wheat
   label .cpy.frf.fro.lbl -text "other_files" -font $Fnt -bg wheat
   pack .cpy.frf -side top -fill x -after .cpy.fre
   pack .cpy.frf.frv -side left -fill both -expand 1
   pack .cpy.frf.fro -side right -fill both -expand 1
   pack .cpy.frf.frv.lbl -side top
   pack .cpy.frf.fro.lbl -side top
   for {set i 0} {$i < [llength $vhdl_file_list]} {incr i} {
      checkbutton .cpy.frf.frv.cb$i -text [lindex $vhdl_file_list $i] -bg wheat -font $Fnt
      pack .cpy.frf.frv.cb$i -side top -anchor w
      .cpy.frf.frv.cb$i select
   }
   if {[file exists $from/SLS/$ent2copy.sls]} {
      checkbutton .cpy.frf.fro.cb_sls -text SLS/$ent2copy.sls -bg wheat -font $Fnt
      pack .cpy.frf.fro.cb_sls -side top -anchor w
      .cpy.frf.fro.cb_sls select
   }
   if {[file exists $from/ADB/$ent2copy.ddc]} {
      checkbutton .cpy.frf.fro.cb_db -text ADB/$ent2copy.ddc -bg wheat -font $Fnt
      pack .cpy.frf.fro.cb_db -side top -anchor w
      .cpy.frf.fro.cb_db select
   }
   if {[file exists $from/circuits/$ent2copy.cir]} {
      checkbutton .cpy.frf.fro.cb_cir -text circuits/$ent2copy.cir -bg wheat -font $Fnt
      pack .cpy.frf.fro.cb_cir -side top -anchor w
      .cpy.frf.fro.cb_cir select
   }
   if {[file exists $from/components/$ent2copy.cmp]} {
      checkbutton .cpy.frf.fro.cb_cmp -text components/$ent2copy.cmp -bg wheat -font $Fnt
      pack .cpy.frf.fro.cb_cmp -side top -anchor w
      .cpy.frf.fro.cb_cmp select
   }
   if {[file exists $from/.$ent2copy.term]} {
      checkbutton .cpy.frf.fro.cb_term -text .$ent2copy.term -bg wheat -font $Fnt
      pack .cpy.frf.fro.cb_term -side top -anchor w
      .cpy.frf.fro.cb_term select
   }
   set other_file_list [glob -nocomplain $from/$ent2copy.*]
   for {set i 0} {$i < [llength $other_file_list]} {incr i} {
      checkbutton .cpy.frf.fro.cbo$i -text [file tail [lindex $other_file_list $i]] -bg wheat -font $Fnt
      pack .cpy.frf.fro.cbo$i -side top -anchor w
   }
   get_nelsis_libs
}

proc accept_wlib {} {
#-----------------------------------------------------------------------------#
# procedure that is carried out when a working_library to copy from is chosen.#
#-----------------------------------------------------------------------------#
   global Fnt MaxNbrMenuItemsInColumn
   upvar #0 Wlib wlib
   upvar #0 EntCopy ent2copy

   set ent_list ""
   set pck_list ""

   set fp [open [.cpy.frm.en get]/$wlib/_info "r"]
   while {![eof $fp]} {
      set txt [gets $fp]
      switch [string index $txt 0] {
	E { lappend ent_list [string range $txt 1 end] }
	P { lappend pck_list [string range $txt 1 end] }
      }
   }
   close $fp

   set ent_list [lsort $ent_list]
   set pck_list [lsort $pck_list]

   catch {destroy .cpy.fre}
   catch {destroy .cpy.frf}
   set ent2copy [lindex $ent_list 0]
   if {$ent_list != ""} {
      frame .cpy.fre     -bd 2 -relief ridge -bg wheat
      label .cpy.fre.lbl -text "entity cell:" -font $Fnt -bg wheat
      eval tk_optionMenu .cpy.fre.wl EntCopy $ent_list $pck_list
      pack .cpy.fre -side top -fill x -after .cpy.frwl
      pack .cpy.fre.lbl -side left -padx 20
      pack .cpy.fre.wl -side right -padx 20 -pady 3
      .cpy.fre.wl configure -bg wheat -font $Fnt
      .cpy.fre.wl.menu configure -bg wheat -font $Fnt
      for {set i 0} {$i <= [.cpy.fre.wl.menu index end]} {incr i} {
         .cpy.fre.wl.menu entryconfigure $i -command {accept_entity}
         if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
            .cpy.fre.wl.menu entryconfigure $i -columnbreak 1
         }
      }
      accept_entity
   }\
   else {
      catch {destroy .cpy.frnl}
   }
}

proc accept_copy_dir {} {
#-----------------------------------------------------------------------------#
# procedure that is carried out when a directory to copy from is chosen.      #
#-----------------------------------------------------------------------------#
   global MyWd

   set copy_dir [.cpy.frm.en get]
   if {$copy_dir == ""} {
      set_mess "no directory chosen: do this first" red
      return
   }
   if {![file isdir $copy_dir]} {
      set_mess "directory:\n $copy_dir\n does not exist" red
      return
   }

   cd $copy_dir
   set copy_dir [pwd]
   cd $MyWd

   set msg ""
   if {![file exists $copy_dir/VHDL]} {
      append msg "warning: there is no sub_directory VHDL\n"
   }
   if {![file exists $copy_dir/ADB]} {
      append msg "warning: there is no sub_directory ADB\n"
   }
   if {![file exists $copy_dir/SLS]} {
      append msg "warning: there is no sub_directory SLS\n"
   }
   if {![file exists $copy_dir/components]} {
      append msg "warning: there is no sub_directory components\n"
   }
   if {![file exists $copy_dir/circuits]} {
      append msg "warning: there is no sub_directory circuits\n"
   }

   set ok [get_vhdl_working_libs]
   if {!$ok} {
      append msg "warning: no working library found!\n"
      set_mess $msg red
      return
   }
   if {[string equal $copy_dir $MyWd]} {
      append msg "you cannot copy from directory:\n $copy_dir\n\
                this is the current project directory\n"
      set ok 0
   }
   set_mess $msg black

   if {$ok} {
      if {$msg != ""} { .cpy.frt.txt insert end "\n" }
      .cpy.frm.en configure -state disabled
      .cpy.frm.br configure -state disabled
      .cpy.frc.ok configure -state normal
   }
}

proc copy_spec_items {} {
#-----------------------------------------------------------------------------#
# procedure that prepares the actual copying of the data.                     #
# the actual copying takes place in the procedures: copy_spec_files,          #
# copy_nelsis_circuit_cell and copy_nelsis_layout_cell.                       #
#-----------------------------------------------------------------------------#
   global DbName
   upvar #0 EntCopy ent2copy
   upvar #0 cb_c cb_c
   upvar #0 cb_l cb_l
   upvar #0 StopRequested stop_requested

   .cpy.frc.ok configure -state disabled

   set subcells ""
   set packages ""
   get_subcells $ent2copy

   set first_copy 0
   foreach i $subcells {
      if {![cell_in_working_lib $i]} {
         add_mess .cpy.frt.txt "***** used subcell $i of $ent2copy not yet present *****"
         add_mess .cpy.frt.txt "***** first copy cell $i *****"
         incr first_copy
      }
   }
   foreach i $packages {
      if {![package_in_working_lib $i]} {
         add_mess .cpy.frt.txt "***** used package $i of $ent2copy not yet present *****"
         add_mess .cpy.frt.txt "***** first copy package $i *****"
         incr first_copy
      }
   }
   if {$first_copy} {
      add_mess .cpy.frt.txt "----- nothing copied -----"
      add_mess .cpy.frt.txt "===== copying cell $ent2copy DONE ====="
      .cpy.frc.ok configure -state normal
      return
   }
   .cpy.frc.qt configure -state disabled
   copy_spec_files

   incr first_copy
   if {([winfo exists .cpy.frnl.cb_c] && $cb_c == "1") ||
       ([winfo exists .cpy.frnl.cb_l] && $cb_l == "1")} {
      if {$DbName == ""} {
	 add_mess .cpy.frt.txt "***** SORRY: variable DbName not set *****"
         add_mess .cpy.frt.txt "----- nelsis database cell $ent2copy not copied -----"
      } elseif {![file exists $DbName/.dmrc]} {
	 add_mess .cpy.frt.txt "***** ERROR: no nelsis database '$DbName' *****"
         add_mess .cpy.frt.txt "----- nelsis database cell $ent2copy not copied -----"
      } else {
	 set first_copy 0
      }
   }

   if {!$first_copy && [winfo exists .cpy.frnl.cb_c] && $cb_c == "1"} {
      foreach i $subcells {
         if {![nelsis_cell_exists $i $DbName circuit]} {
            add_mess .cpy.frt.txt "***** used subcell $i of $ent2copy not yet present in the circuit part of database $DbName *****"
            add_mess .cpy.frt.txt "***** first copy cell $i of nelsis database *****"
            incr first_copy
         }
      }
      if {$first_copy} {
         add_mess .cpy.frt.txt "----- nelsis database cell $ent2copy not copied -----"
      } else {
	 if {$stop_requested} { copy_suspend }
	 copy_nelsis_circuit_cell
      }
   }
   if {!$first_copy && [winfo exists .cpy.frnl.cb_l] && $cb_l == "1"} {
      foreach i $subcells {
         if {![nelsis_cell_exists $i $DbName layout]} {
            add_mess .cpy.frt.txt "***** used subcell $i of $ent2copy not yet present in the layout part of database $DbName *****"
            add_mess .cpy.frt.txt "***** first copy cell $i of nelsis database *****"
            incr first_copy
         }
      }
      if {$first_copy} {
         add_mess .cpy.frt.txt "----- layout part of nelsis database cell $ent2copy not copied -----"
      } else {
	 if {$stop_requested} { copy_suspend }
	 copy_nelsis_layout_cell
      }
   }
   add_mess .cpy.frt.txt "===== copying cell $ent2copy DONE ====="
   .cpy.frc.ok configure -state normal
   .cpy.frc.qt configure -state normal

   if {!$first_copy} read_infofile
}

proc copy_nelsis_circuit_cell {} {
#-----------------------------------------------------------------------------#
# procedure that copies the circuit part of a cell in the nelsis_database.    #
# it does so using the routines xsls end csls from the nelsis_system.         #
#-----------------------------------------------------------------------------#
   global DbName MyWd
   upvar #0 EntCopy ent2copy
   upvar #0 Nlib nlib

#first make a tempory file with the include_statements
   set fp [open $MyWd/$DbName/tmp.sls w]
   puts $fp "#include \"sls_prototypes/digilib8_93.ext\""
   puts $fp "#include \"sls_prototypes/analib8_00.ext\""
   puts $fp "#include \"sls_prototypes/bonding11_93.ext\""
   close $fp
# then add the extracted sls_description to this file
   goto_copy_dbdir [.cpy.frm.en get]/$nlib
   catch {exec xsls $ent2copy >> $MyWd/$DbName/tmp.sls} err_str
   if {$err_str != ""} {
      add_mess .cpy.frt.txt "***** ERRORS making $DbName/tmp.sls *****"
      add_mess .cpy.frt.txt "$err_str"
      restore_cwd
      return
   }
   goto_dbdir
   catch {exec csls  -s tmp.sls} err_str
   if {$err_str != ""} {
      add_mess .cpy.frt.txt "***** ERRORS parsing file $DbName/tmp.sls *****"
      add_mess .cpy.frt.txt "$err_str"
      restore_cwd
      return
   }
   set fn [.cpy.frm.en get]/$nlib/circuit/$ent2copy/src_arch
   if {[file readable $fn]} {
      file copy -force $fn circuit/$ent2copy/src_arch
   }
   restore_cwd
   file delete $MyWd/$DbName/tmp.sls
   add_mess .cpy.frt.txt "circuit database of cell $ent2copy copied"
}

proc copy_nelsis_layout_cell {} {
#-----------------------------------------------------------------------------#
# procedure that copies the layout part of a cell in the nelsis_database.     #
# it does so using the routines xldm end cldm from the nelsis_system.         #
#-----------------------------------------------------------------------------#
   global DbName MyWd
   upvar #0 EntCopy ent2copy
   upvar #0 Nlib nlib

   file delete $MyWd/$DbName/tmp.ldm
   goto_copy_dbdir [.cpy.frm.en get]/$nlib
   catch {exec xldm -r -f $MyWd/$DbName/tmp.ldm $ent2copy} err_str
   if {[string match *aborted* $err_str]} {
      add_mess .cpy.frt.txt "***** ERRORS making ldm_file $DbName/tmp.ldm *****"
      add_mess .cpy.frt.txt "$err_str"
      restore_cwd
      return
   }
   goto_dbdir
   catch {exec cldm -f tmp.ldm} err_str
   if {([string match *\[0-9\]:* $err_str]) ||
       ([string match *aborted* $err_str])} {
      add_mess .cpy.frt.txt "***** ERRORS parsing file $DbName/tmp.ldm *****"
      add_mess .cpy.frt.txt "$err_str"
      restore_cwd
      return
   }
   restore_cwd
   file delete $MyWd/$DbName/tmp.ldm
   add_mess .cpy.frt.txt "layout database of cell $ent2copy copied"
}

proc copy_spec_files {} {
#-----------------------------------------------------------------------------#
# procedure that copies indicated files to the right directories.             #
# In the case of vhdl_files these files are also compiled .                   #
#-----------------------------------------------------------------------------#
   global SimLibName MyWd
   upvar #0 StopRequested stop_requested

   set copy_dir [.cpy.frm.en get]
   set children [winfo children .cpy.frf.frv]
   eval lappend children [winfo children .cpy.frf.fro]
   foreach chld $children {
      if {[winfo class $chld] == "Checkbutton"} {
         set var [lindex [split $chld "."] 4]
         upvar #0 $var $var
         if {[expr $$var] == "1"} {
            set fname [$chld cget -text]
	    if {![file exists $copy_dir/$fname]} {
               add_mess .cpy.frt.txt "file $fname does not exist"
	       continue
	    }
            set dir [file dir $fname]
            if {![file exists $dir]} {
               add_mess .cpy.frt.txt "file $fname; dir '$dir' does not exist"
               continue
            }
            if {[file ext $fname] == ".vhd" && $dir == "VHDL"} {
	       if {[set wdw [get_edit_wdw $fname]] != ""} {
		  set msg "We must quit the found edit window with file:\n '$fname'"
		  if {[df_choise_dialog "$msg\nmust the content be saved as backup?"] == "yes"} {
		     set fp [open $MyWd/$fname "w"]
		     puts -nonewline $fp [$wdw.txt get 1.0 end]
		     close $fp
		  }
		  destroy $wdw
	       }
	    }
	    if {[file exists $MyWd/$fname]} {
	       file rename -force $MyWd/$fname $MyWd/$fname\_
	    }
            file copy -force $copy_dir/$fname $MyWd/$fname
            add_mess .cpy.frt.txt "file $fname copied"
            if {[file ext $fname] == ".vhd" && $dir == "VHDL"} {
               if {[do_vcom $SimLibName $fname] > 0} {
                  add_mess .cpy.frt.txt "file $fname compiled succesfully"
                  read_infofile
               } else {
                  add_mess .cpy.frt.txt "*** file $fname compiles with errors ***"
                  add_mess .cpy.frt.txt "*** correct them and close the edit_window to continue ***"
		  if {[set wdw [get_edit_wdw $fname]] != ""} {
		     wm attributes .cpy -topmost 0
		     wm attributes $wdw -topmost 1; set topmostWindow $wdw
		     grab set $wdw
		     tkwait window $wdw
		     wm attributes .cpy -topmost 1; set topmostWindow .cpy
		     grab set .cpy
		  }
               }
            }
            if {$stop_requested} { copy_suspend }
         }
      }
   }
}

proc unlock_copy_dir {} {
#-----------------------------------------------------------------------------#
# procedure that makes it possible to chose a new directory to copy from.     #
#-----------------------------------------------------------------------------#
   catch {destroy .cpy.frwl}
   catch {destroy .cpy.fre}
   catch {destroy .cpy.frf}
   catch {destroy .cpy.frnl}

   .cpy.frm.en configure -state normal
   .cpy.frm.br configure -state normal
   .cpy.frc.ok configure -state disabled
}

proc copy_suspend {} {
#-----------------------------------------------------------------------------#
# procedure that halts the copying process afer a copy is finished untill     #
# it is allowed to continue again with the next copy.                         #
#-----------------------------------------------------------------------------#
   .cpy.frc.st configure -text "Continue"
   .cpy.frc.qt configure -state normal
   tkwait variable StopRequested
   .cpy.frc.st configure -text "Stop"
   .cpy.frc.qt configure -state disabled
}
