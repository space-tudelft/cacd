
proc get_edit_wdw {fname} {
#------------------------------------------------
# procedure to find an edit_window for file fname
#------------------------------------------------
   foreach i [winfo children .] {
      if {[string range $i 0 8] == ".edt_wdw_"} {
         if {[wm title $i] == $fname} { return $i }
      }
   }
   return ""
}

proc edit_wdw {file_name is_new ename} {
#----------------------------------------------------------
# procedure to start an edit_window with the specified file
#----------------------------------------------------------
   global Fnt EW_Earr EditFnt Textindent MyWd
   global entityChanged Vhdl_editor GWTF_VHDL_EDITOR

   if {$is_new == 0} {
      set extern_editor ""
      if {$GWTF_VHDL_EDITOR != ""} {
	 set extern_editor $GWTF_VHDL_EDITOR
      } elseif {$Vhdl_editor != "built-in"} {
	 set extern_editor $Vhdl_editor
      }
      if {$extern_editor != ""} {
	 if {$extern_editor == "vim"} {
	    exec xterm -e $extern_editor $MyWd/$file_name &
	 } else {
	    exec $extern_editor $file_name &
	 }
	 return
      }
   }

   set wdw [get_edit_wdw $file_name]
   if {$wdw == ""} {
      set w_idx 0
      while {[winfo exists .edt_wdw_$w_idx]} {
         incr w_idx
      }
      set edt_wdw "edt_wdw_$w_idx"
      set EW_Earr($edt_wdw) $ename
      toplevel   .$edt_wdw
      frame      .$edt_wdw.menubar -bd 2 -relief raised -bg gold
      menubutton .$edt_wdw.menubar.file -text "File"\
                                        -menu .$edt_wdw.menubar.file.cmds\
                                        -bg gold -font $Fnt
      menubutton .$edt_wdw.menubar.edit -text "Edit"\
                                        -menu .$edt_wdw.menubar.edit.cmds\
                                        -bg gold -font $Fnt
      menubutton .$edt_wdw.menubar.txt  -text "Text"\
                                        -menu .$edt_wdw.menubar.txt.txt\
                                        -bg gold -font $Fnt
      label      .$edt_wdw.menubar.pos  -borderwidth 2 -relief sunk\
                                        -font $Fnt -anchor w\
                                        -width 13 -text "cursor: 1.0"\
                                        -padx 10 -pady 5\
                                        -bg wheat3
      button     .$edt_wdw.menubar.cmp  -text "Compile" -bg gold -font $Fnt\
                                        -command "do_compile $edt_wdw"

      ctext      .$edt_wdw.txt          -width 80 -height 25 \
                                        -yscrollcommand ".$edt_wdw.sb set"\
                                        -bg wheat -font $EditFnt -undo 1

      highlight_vhdl .$edt_wdw.txt

      scrollbar  .$edt_wdw.sb           -command ".$edt_wdw.txt yview"\
                                        -bg wheat
      frame      .$edt_wdw.err          -bg wheat -bd 2 -relief ridge
      text       .$edt_wdw.err.txt      -bg wheat3 -height 5\
                                        -font $EditFnt -fg red\
                                        -yscrollcommand ".$edt_wdw.err.sb set"
      scrollbar  .$edt_wdw.err.sb       -command ".$edt_wdw.err.txt yview"\
                                        -bg wheat

      menu       .$edt_wdw.menubar.txt.txt -bg wheat3 -font $Fnt
                 .$edt_wdw.menubar.txt.txt add cascade -label "Font"\
                                           -menu .$edt_wdw.menubar.txt.txt.fnts
                 .$edt_wdw.menubar.txt.txt add cascade -label "Tab"\
                                           -menu .$edt_wdw.menubar.txt.txt.indents

      menu       .$edt_wdw.menubar.txt.txt.fnts -bg wheat3 -font $Fnt
                 .$edt_wdw.menubar.txt.txt.fnts add command -label "Small"\
                          -command "set_edit_font $edt_wdw Small"
                 .$edt_wdw.menubar.txt.txt.fnts add command -label "Normal"\
                          -command "set_edit_font $edt_wdw Normal"
                 .$edt_wdw.menubar.txt.txt.fnts add command -label "Large"\
                          -command "set_edit_font $edt_wdw Large"
                 .$edt_wdw.menubar.txt.txt.fnts add command -label "XLarge"\
                          -command "set_edit_font $edt_wdw XLarge"

      menu       .$edt_wdw.menubar.txt.txt.indents -bg wheat3 -font $Fnt
                 .$edt_wdw.menubar.txt.txt.indents add command -label "2"\
                          -command "set_edit_indent $edt_wdw 2"
                 .$edt_wdw.menubar.txt.txt.indents add command -label "3"\
                          -command "set_edit_indent $edt_wdw 3"
                 .$edt_wdw.menubar.txt.txt.indents add command -label "4"\
                          -command "set_edit_indent $edt_wdw 4"
                 .$edt_wdw.menubar.txt.txt.indents add command -label "6"\
                          -command "set_edit_indent $edt_wdw 6"
                 .$edt_wdw.menubar.txt.txt.indents add command -label "8"\
                          -command "set_edit_indent $edt_wdw 8"
                 .$edt_wdw.menubar.txt.txt.indents add command -label "12"\
                          -command "set_edit_indent $edt_wdw 12"

      menu       .$edt_wdw.menubar.file.cmds -bg wheat3 -font $Fnt
                 .$edt_wdw.menubar.file.cmds add command -label "Read"\
                          -command "find_vhdl_file read $edt_wdw"
                 .$edt_wdw.menubar.file.cmds add command -label "Save"\
                          -command "write_vhdlfile $edt_wdw 0"
                 .$edt_wdw.menubar.file.cmds add command -label "Save as ..."\
                          -command "write_vhdlfile $edt_wdw 1"
                 .$edt_wdw.menubar.file.cmds add command -label "Quit"\
                          -command "quit_editor $edt_wdw"

      menu       .$edt_wdw.menubar.edit.cmds -bg wheat3 -font $Fnt
                 .$edt_wdw.menubar.edit.cmds add command -label "Copy"\
                          -command "tk_textCopy .$edt_wdw.txt"
                 .$edt_wdw.menubar.edit.cmds add command -label "Cut"\
                          -command "tk_textCut .$edt_wdw.txt"
                 .$edt_wdw.menubar.edit.cmds add command -label "Paste"\
                          -command "tk_textPaste .$edt_wdw.txt"
                 .$edt_wdw.menubar.edit.cmds add command -label "Undo"\
                          -command ".$edt_wdw.txt edit undo"
                 .$edt_wdw.menubar.edit.cmds add command -label "Redo"\
                          -command ".$edt_wdw.txt edit redo"
                 .$edt_wdw.menubar.edit.cmds add command -label "Search/Replace"\
                          -command "do_search $edt_wdw"
                 .$edt_wdw.menubar.edit.cmds add command -label "Add component"\
                          -command "add_component $edt_wdw"

      pack .$edt_wdw.menubar      -side top -fill x
      pack .$edt_wdw.menubar.file -side left
      pack .$edt_wdw.menubar.edit -side left
      pack .$edt_wdw.menubar.txt  -side left
      pack .$edt_wdw.menubar.pos  -side right
      pack .$edt_wdw.menubar.cmp  -side right
      pack .$edt_wdw.err          -side bottom -fill both
      pack .$edt_wdw.err.txt      -side left -fill x -expand 1
      pack .$edt_wdw.err.sb       -side right -fill y
      pack .$edt_wdw.txt          -side left -fill both -expand 1
      pack .$edt_wdw.sb           -side right -fill y

      set_edit_indent $edt_wdw $Textindent

      set entityChanged($edt_wdw) 0

      bind .$edt_wdw.txt <ButtonRelease-1> "show_cursor_pos $edt_wdw"
      wm protocol .$edt_wdw WM_DELETE_WINDOW "quit_editor $edt_wdw"

      wm title .$edt_wdw $file_name
      if {[file ext $file_name] != ".vhd"} { .$edt_wdw.menubar.cmp configure -state disabled }
      if {$is_new == 0 || $is_new == 2} {
         read_file $file_name $edt_wdw
      }
      focus .$edt_wdw.txt
      return .$edt_wdw
   }\
   else {
      wm deiconify $wdw
      raise $wdw
      focus $wdw.txt
      return $wdw
   }
}

proc read_new_vhdlfile {fname edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to read a vhdl_file into the edit_window                          #
# it looks if there is already a window open with the chosen vhdl_file; if so #
# it brings to the foreground this window. Otherwise it reads the file into   #
# the window after a conformation that the previous contents are lost         #
# the actual reading of the file is done in the procedure 'read_file' which   #
# is called.                                                                  #
#-----------------------------------------------------------------------------#
   set wdw [get_edit_wdw $fname]
   if {$wdw == ""} {
      set name VHDL/[file tail [wm title .$edt_wdw]]
      if {[see_change $name $edt_wdw] == 1} {
         set answ [df_choise_dialog\
		"There are changes in the existing text\nmust it be saved?"]
         if {$answ == "yes"} { write_vhdlfile $edt_wdw 0 }
      }
      wm title .$edt_wdw $fname
      .$edt_wdw.menubar.cmp configure -state normal
      read_file $fname $edt_wdw
   }\
   else {
      wm deiconify $wdw
      raise $wdw
      focus $wdw.txt
   }
}

proc set_edit_font {edt_wdw dimension} {
#-----------------------------------------------------------------------------#
# procedure change the font of the text part of the edit_window               #
#-----------------------------------------------------------------------------#
   global EditFnt Textindent

   switch $dimension {
   "Small"  {set EditFnt "-*-Courier-Bold-R-Normal--*-120-*"}
   "Normal" {set EditFnt "-*-Courier-Bold-R-Normal--*-140-*"}
   "Large"  {set EditFnt "-*-Courier-Bold-R-Normal--*-180-*"}
   "XLarge" {set EditFnt "-*-Courier-Bold-R-Normal--*-240-*"}
   }
   .$edt_wdw.txt configure -font $EditFnt

   set_edit_indent $edt_wdw $Textindent
}

proc set_edit_indent {edt_wdw indent} {
   global Textindent

   set Textindent $indent

   switch $Textindent {
   "2"  {set s "00"}
   "3"  {set s "000"}
   "4"  {set s "0000"}
   "6"  {set s "000000"}
   "8"  {set s "00000000"}
   "12" {set s "000000000000"}
   }

   set pwidth "[font measure [.$edt_wdw.txt cget -font] -displayof .$edt_wdw.txt $s]"
   set scaling [tk scaling -displayof .$edt_wdw.txt]
   set tabsize [expr ($pwidth) / (72.0 * $scaling)]
   .$edt_wdw.txt configure -tabs ${tabsize}i
}

proc write_vhdlfile {edt_wdw saveas} {
#-----------------------------------------------------------------------------#
# procedure to write a .vhd file using the name file_name:                    #
# the origional .vhd file is written to name.vhd_                             #
#-----------------------------------------------------------------------------#
   global EW_Earr MyWd entityChanged

   set fn [file tail [wm title .$edt_wdw]]
   if {$saveas} {
      set types {
           {{VHDL Files} {.vhd}}
           {{All files}  *     }
      }
      set name [tk_getSaveFile -initialdir VHDL -initialfile $fn -filetypes $types -parent .$edt_wdw]
      if {$name == ""} return
      set tail VHDL/[file tail $name]
      if {$name == "$MyWd/$tail"} { set name $tail } else { set saveas 0 }
      if {$name != "VHDL/$fn"} { set EW_Earr($edt_wdw) "" }
      if {!$saveas} { upd_status_line "saving file as $name" }
   } else {
      set name VHDL/$fn
   }

   if {[file exist $name]} { file copy -force $name $name\_ }
   set fp [open $name "w"]
   puts -nonewline $fp [.$edt_wdw.txt get 1.0 end]
   close $fp

   if {$EW_Earr($edt_wdw) != ""} {
      if {$EW_Earr($edt_wdw) == [file rootname $fn]} {
         set entityChanged($edt_wdw) 1
      }
      outdate_cir $EW_Earr($edt_wdw)
   }
   if {$saveas} {
      wm title .$edt_wdw $name
      if {[file ext $name] != ".vhd"} {
	.$edt_wdw.menubar.cmp configure -state disabled
      } else {
	.$edt_wdw.menubar.cmp configure -state normal
      }
   }
}

proc read_file {file_name edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to read a vhdl_file into the edit_window                          #
#-----------------------------------------------------------------------------#
   .$edt_wdw.txt delete 1.0 end
   set n 0
   if {[catch {set fp [open $file_name "r"]}]} {
      if {[catch {set fp [open $file_name\_ "r"]}]} { incr n }
      if {$n} { set m "Can't" } { set m "Have" }
      .$edt_wdw.err.txt insert end "WARNING:\n\
		Can't read file $file_name\n\
		$m read the backup file.\n"
   }
   if {!$n} {
      while {1} {
	 set txt [gets $fp]
	 if {[eof $fp] && $txt == ""} break
	 if {$n} {
	   .$edt_wdw.txt fastinsert end "\n$txt"
	 } else {
	   .$edt_wdw.txt fastinsert end "$txt"
	   incr n
	 }
      }
      close $fp
   }
   .$edt_wdw.txt highlight 1.0 end
   .$edt_wdw.txt edit reset
}

proc quit_editor {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to close the edit_window                                          #
#-----------------------------------------------------------------------------#
   global entityChanged

   set tail_name [file tail [wm title .$edt_wdw]]
   set fname VHDL/$tail_name
   if {[see_change $fname $edt_wdw] == 1} {
      if {![file exist $fname]} {
	 set msg "The file does not yet exist"
      } else {
	 set msg "There are changes in the file"
      }
      set answ [df_choise_dialog "$msg\n '$fname'\nmust it be saved?"]
      if {$answ == "yes"} { write_vhdlfile $edt_wdw 0 }
   }

   set ent_name [file rootname $tail_name]
   set file_name [wm title .$edt_wdw]

   destroy .$edt_wdw

   if {$entityChanged($edt_wdw) && [file exist components/$ent_name.cmp]} {
      set answ [df_choise_dialog "Update component of '$ent_name'?"]
      if {$answ == "yes"} { mk_component $ent_name $file_name }
   }
}

proc see_change {file_name edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to see if the file has changed                                    #
#-----------------------------------------------------------------------------#

   if {![file exist $file_name]} { return 1 }
   set fp [open $file_name]
   set i 0
   foreach line [split [.$edt_wdw.txt get 1.0 end] \n] {
      set chk_txt [gets $fp]
      if {$chk_txt != $line} { set i 1; break }
   }
   if {![eof $fp]} { set i 1 }
   close $fp
   return $i
}

proc do_compile {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to compile the file for simulation                                #
#-----------------------------------------------------------------------------#
   global SimLibName

   set fname VHDL/[file tail [wm title .$edt_wdw]]
   if {[see_change $fname $edt_wdw] == 1} {
      if {![file exist $fname]} {
	 set msg "File $fname does not yet exist"
      } else {
	 set msg "There are changes in the existing text"
      }
      if {[df_choise_dialog "$msg:
Click 'Yes' to save the file first.
No compilation will be done otherwise"] == "no"} return
      write_vhdlfile $edt_wdw 0
   }
   upd_status_line "compiling file $fname ..."
   if {[do_vcom $SimLibName $fname] < 0} return
   read_infofile
   check_compiled_file $fname $edt_wdw
}

proc check_compiled_file {fname edt_wdw} {
   global Carr

   set lst ""
   if {[set e [get_e_of_file $fname]] != ""} {
      #puts "File is used by entity $e"
      set a [lindex [get_a_of_file $fname] 0]
      if {$a != ""} { set c [get_c_of_file $fname] } { set c "" }
      set lst [find_archs $e $a]
      for {set i 0} {$i < [array size Carr]} {incr i} {
	 if {[lindex $Carr($i) 1] == $e} {
	    if {[lindex $Carr($i) 0] == $c} continue
	    if {[lindex $Carr($i) 2] == $a && [check_config $fname [lindex $Carr($i) 3]]} {
		lappend lst "[lindex $Carr($i) 0] (need be changed)"
	    } { lappend lst [lindex $Carr($i) 0] }
	 }
      }
   }\
   elseif {[set a [get_a_of_file $fname]] != ""} {
      set e [lindex $a 1]
      set a [lindex $a 0]
      set c [get_c_of_file $fname]
      #puts "File is used by architecture $a of $e"
      for {set i 0} {$i < [array size Carr]} {incr i} {
	 if {[lindex $Carr($i) 1] == $e && [lindex $Carr($i) 2] == $a} {
	    if {[lindex $Carr($i) 0] == $c} continue
	    if {[check_config $fname [lindex $Carr($i) 3]]} {
		lappend lst "[lindex $Carr($i) 0] (need be changed)"
	    } { lappend lst [lindex $Carr($i) 0] }
	 }
      }
   }\
   elseif {[set c [get_c_of_file $fname]] != ""} {
      #puts "File is used by configuration $c"
      set lst [find_config_uses $c]
   }\
   elseif {[set p [get_p_of_file $fname]] != ""} {
      #puts "File is used by package $p"
      set lst [find_pack_uses [get_parr_idx $p]]
   }\
   else {
      .$edt_wdw.err.txt insert end "\nERROR: compiled_file $fname not found!"
   }
   if {$lst != ""}  { .$edt_wdw.err.txt insert end "\nNOTE: Following items need to be recompiled:"
     foreach i $lst { .$edt_wdw.err.txt insert end "\n      $i" }
   }
}

proc check_config {afname cfname} {
   upvar local_list local_list

   if {![info exist local_list]} {
      set local_list ""
      foreach child [find_children_in_vhdl $afname] {
	 if {[get_earr_idx $child] < 0} continue
	 # child is local entity
	 lappend local_list $child
      }
   }

   set config 0
   set ulist ""
   set fp [open $cfname]
   while {![eof $fp]} {
      if {[set tt [gets $fp]] == ""} continue
      set tt [string tolower $tt]
      regsub -all {[""{}]} $tt " " tt
      if {$config} {
	 if {[string match "work.*" [lindex $tt end]]} {
	    lappend ulist [string tolower [lindex $tt end-3]]
	 }
      } elseif {[lindex $tt 0] == "configuration"} { incr config }
   }
   close $fp

   # compare both lists
   foreach child $local_list {
      if {[lsearch $ulist $child] < 0} { return 1 }
   }
   foreach child $ulist {
      if {[lsearch $local_list $child] < 0} { return 1 }
   }
   return 0
}

proc show_cursor_pos {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to show the cursor_position                                       #
#-----------------------------------------------------------------------------#
   .$edt_wdw.menubar.pos configure -text "cursor:[.$edt_wdw.txt index insert]"
}

proc add_component {edt_wdw} {
#-----------------------------------------------------------------------------#
# window to chose a component from                                            #
#-----------------------------------------------------------------------------#
   global MyWd Fnt topmostWindow

   if {[.$edt_wdw.txt search -forwards -nocase architecture 1.0] == ""} {
      df_mess_dialog "ERROR:\n\
	A component can only be added to an architecture and\n\
	there is no architecture found in this vhdl_description.\n\
	No component added"
      return
   }

   toplevel  .comps
   frame     .comps.mnfr
   listbox   .comps.mnfr.lb -bg wheat -height 10 -width 30 -font $Fnt\
                         -yscrollcommand ".comps.mnfr.sb set"
   scrollbar .comps.mnfr.sb -command ".comps.mnfr.lb yview" -bg wheat
   frame     .comps.fr   -bg gold
   button    .comps.fr.ok -bg gold -text "Add" -font $Fnt -width 6\
                       -command "do_add_component $edt_wdw"
   button    .comps.fr.cl -bg gold -text "Cancel" -font $Fnt\
                       -command "destroy .comps" -width 6

   pack .comps.mnfr    -side top -fill both -expand 1
   pack .comps.mnfr.lb -side left -fill both -expand 1
   pack .comps.mnfr.sb -side right -fill y
   pack .comps.fr      -side bottom -fill x
   pack .comps.fr.ok   -side left -fill x -padx 10 -pady 3
   pack .comps.fr.cl   -side right -fill x -padx 10 -pady 3

   set files [glob -nocomplain -dir $MyWd/components -tail *.cmp]
   foreach f [lsort $files] {
      .comps.mnfr.lb insert end [file rootname $f]
   }
   .comps.mnfr.lb selection set 0
   wm attributes .comps -topmost 1; set topmostWindow .comps
   wm geometry .comps +[winfo pointerx .cv]+[winfo pointery .cv]
   wm title .comps "add components"
   grab set .comps
   tkwait window .comps
}

proc do_add_component {edt_wdw} {
#-----------------------------------------------------------------------------#
# add a component to an architecture of a vhdl_description in the editor.     #
#-----------------------------------------------------------------------------#
   global MyWd

   set cursel [.comps.mnfr.lb curselection]
   set cmp_file [.comps.mnfr.lb get [lindex $cursel 0]].cmp
   set f_cmp [open $MyWd/components/$cmp_file r]
   set p_list ""
   set t_list ""
   set max_plen 1
   set max_tlen 1
   while {![eof $f_cmp]} {
      set txt [gets $f_cmp]
      if {[lindex $txt 0] == "port"} {
         if {[string index [lindex $txt 1] 0] == "*"} {
            lappend t_list "inout"
         } else {
            lappend t_list [lindex $txt 1]
         }
         set len [string length [lindex $t_list end]]
	 if {$len > $max_tlen} { set max_tlen $len }
         set pname [split [lindex $txt 2] "(:)"]
         lappend p_list $pname
         set len [string length [lindex $pname 0]]
	 if {$len > $max_plen} { set max_plen $len }
      }
   }
   set comp_name [file rootname [file tail $cmp_file]]
   if {[.$edt_wdw.txt search -nocase CellsLib_DECL_PACK 1.0] == "" &&
       [is_lib_component $comp_name]} {
      .$edt_wdw.txt insert 1.0 "use CellsLib.CellsLib_DECL_PACK.all;\n"
      .$edt_wdw.txt insert 1.0 "library CellsLib;\n"
   }
   close $f_cmp
   set idx "[.$edt_wdw.txt search -forwards -nocase architecture 1.0] + 1 lines"
   .$edt_wdw.txt mark set insert $idx
   .$edt_wdw.txt insert $idx "   component $comp_name\n"
   for {set i 0} {$i < [llength $p_list]} {incr i} {
      set idx [.$edt_wdw.txt index insert]
      if {$i == 0} {
         .$edt_wdw.txt insert $idx "      port ("
      } else {
         .$edt_wdw.txt insert $idx "            "
      }
      set idx [.$edt_wdw.txt index insert]
      .$edt_wdw.txt insert $idx [lindex [lindex $p_list $i] 0]
      set idx [.$edt_wdw.txt index insert]
      set p_sp [expr $max_plen - [string length [lindex [lindex $p_list $i] 0]]]
      .$edt_wdw.txt insert $idx " [add_space $p_sp]: [lindex $t_list $i]"
      set idx [.$edt_wdw.txt index insert]
      set t_sp [expr $max_tlen - [string length [lindex $t_list $i]]]
      if {[llength [lindex $p_list $i]] < 2} {
         .$edt_wdw.txt insert $idx "[add_space $t_sp] std_logic"
      } else {
         set hi [lindex [lindex $p_list $i] 1]
         set lw [lindex [lindex $p_list $i] 2]
	 if {$lw > $hi} {
	 .$edt_wdw.txt insert $idx "[add_space $t_sp] std_logic_vector($hi to $lw)"
	 } else {
	 .$edt_wdw.txt insert $idx "[add_space $t_sp] std_logic_vector($hi downto $lw)"
	 }
      }
      set idx [.$edt_wdw.txt index insert]
      if {$i == [llength $p_list]-1} {
         .$edt_wdw.txt insert $idx ");\n"
      } else {
         .$edt_wdw.txt insert $idx ";\n"
      }
   }
   set idx [.$edt_wdw.txt index insert]
   .$edt_wdw.txt insert $idx "   end component;\n\n"
}

proc add_space {n_space} {
#-----------------------------------------------------------------------------#
# procedure which returns a string of 'n_space' spaces.                       #
#-----------------------------------------------------------------------------#
   return [string repeat " " $n_space]
}

proc is_lib_component {name} {
#-----------------------------------------------------------------------------#
# procedure which returns a '1' if 'name' is the name of a component from     #
# the CellsLib library; '0' otherwise.                                        #
#-----------------------------------------------------------------------------#
   set fp [open components/libcells "r"]
   while {![eof $fp]} {
      if {[gets $fp] == $name} { close $fp; return 1 }
   }
   close $fp
   return 0
}

proc ask_open_project {} {
#-----------------------------------------------------------------------------#
# procedure to open a project
#-----------------------------------------------------------------------------#
   global Fnt RecentProjs ask_project_rv last_select topmostWindow

   if {[winfo exists .lb]} { destroy .lb }
   toplevel  .lb
   frame     .lb.mnfr
   listbox   .lb.mnfr.lb -bg wheat -height 10 -width 30 -font $Fnt\
                         -yscrollcommand ".lb.mnfr.sb set"
   scrollbar .lb.mnfr.sb -command ".lb.mnfr.lb yview" -bg wheat
   frame     .lb.fr    -bg gold
   button    .lb.fr.ok -bg gold -text "OK" -font $Fnt -width 6 -command {
			   set cs [.lb.mnfr.lb curselection]
			   set ask_project_rv [.lb.mnfr.lb get [lindex $cs 0]]
			   destroy .lb
			}
   button    .lb.fr.cl -bg gold -text "Cancel" -font $Fnt -width 6 -command {
			   set ask_project_rv ""
			   destroy .lb
			}

   pack .lb.mnfr    -side top -fill both -expand 1
   pack .lb.mnfr.lb -side left -fill both -expand 1
   pack .lb.mnfr.sb -side right -fill y
   pack .lb.fr      -side bottom -fill x
   pack .lb.fr.ok   -side left -fill x -padx 10 -pady 3
   pack .lb.fr.cl   -side right -fill x -padx 10 -pady 3

   set Dir ""
   foreach proj $RecentProjs {
      if {$Dir == ""} {
	 set dir [file dirname $proj]
	 if {[file exist $dir]} { set Dir $dir }
      }
      if {[file isdir $proj]} { .lb.mnfr.lb insert end $proj }
   }
   if {$Dir == ""} { set Dir "~" }
   .lb.mnfr.lb insert end "<browse>"
   .lb.mnfr.lb selection set 0

   wm attributes .lb -topmost 1; set topmostWindow .lb
   wm geometry .lb +[expr [winfo pointerx .]-50]+[expr [winfo pointery .]-200]
   wm title .lb "open_project"
   grab set .lb
   tkwait window .lb

   if {$ask_project_rv == "<browse>"} {
      if {[info exist last_select] && $last_select != ""} {
	 set dir [file dirname $last_select]
	 if {[file exist $dir]} { set Dir $dir }
      }
      set wd [pwd]
      if {[catch {cd $Dir}]} { set Dir "~" }
      set last_select [tk_chooseDirectory -initialdir $Dir -title "Open a project directory"]
      cd $wd
   } else {
      set last_select $ask_project_rv
   }
   return $last_select
}

proc find_vhdl_file {operation edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to find a vhdl_file and do an operation on it                     #
#-----------------------------------------------------------------------------#
   global Fnt topmostWindow

   if {[winfo exists .lb]} { destroy .lb }
   toplevel  .lb
   frame     .lb.mnfr
   listbox   .lb.mnfr.lb -bg wheat -height 10 -width 30 -font $Fnt\
                         -yscrollcommand ".lb.mnfr.sb set"
   scrollbar .lb.mnfr.sb -command ".lb.mnfr.lb yview" -bg wheat
   frame     .lb.fr   -bg gold
   button    .lb.fr.ok -bg gold -text "OK" -font $Fnt -width 6\
                       -command "file_operation $operation $edt_wdw"
   button    .lb.fr.cl -bg gold -text "Cancel" -font $Fnt\
                       -command "destroy .lb" -width 6

   pack .lb.mnfr    -side top -fill both -expand 1
   pack .lb.mnfr.lb -side left -fill both -expand 1
   pack .lb.mnfr.sb -side right -fill y
   pack .lb.fr      -side bottom -fill x
   pack .lb.fr.ok   -side left -fill x -padx 10 -pady 3
   pack .lb.fr.cl   -side right -fill x -padx 10 -pady 3

   set files [glob -nocomplain -dir VHDL -tail *.vhd]
   foreach f [lsort $files] {
      .lb.mnfr.lb insert end $f
   }
   .lb.mnfr.lb selection set 0
   wm attributes .lb -topmost 1; set topmostWindow .lb
   wm geometry .lb +[winfo pointerx .]+[winfo pointery .]
   wm title .lb "$operation vhdl_files"
   grab set .lb
   tkwait window .lb
}

proc file_operation {operation edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to do an operation on a vhdl_file (edit, compile, delete or read) #
#-----------------------------------------------------------------------------#
   global SimLibName
   set cursel [.lb.mnfr.lb curselection]
   set fname VHDL/[.lb.mnfr.lb get [lindex $cursel 0]]
   switch $operation {
      "edit"    { edit_wdw $fname 0 "" }
      "compile" { if {[do_vcom $SimLibName $fname] > 0} read_infofile }
      "delete"  { file_delete $fname }
      "read"    { read_new_vhdlfile $fname $edt_wdw }
   }
   destroy .lb
}

proc file_delete {fname} {
   set ename [get_ename_of_vhdl_file $fname]
   if {$ename != ""} {
      if {[df_choise_dialog "WARNING:\n\
	  File $fname is used by entity $ename\n\
	  Are you sure to delete it?"] == "no"} return
   }
   if {[file exist $fname]} {
      file copy -force $fname $fname\_
      file delete $fname
      upd_status_line "file $fname deleted"
   }
}
