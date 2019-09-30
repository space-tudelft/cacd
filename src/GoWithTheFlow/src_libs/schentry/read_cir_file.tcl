
set file_name ""
set selnr 0

# procedure to read a circuit_file from the circuits directory
#-------------------------------------------------------------
proc read_cir_file {} {
   global CircuitPath Fnt file_name selnr

   if {[winfo exists .templ]} { destroy_templ }

   set files [glob -nocomplain $CircuitPath/*.cir]
   set nr [llength $files]
   if {$nr < 1} {
	show_mess "Sorry, no circuit files!"
	return
   }

   toplevel  .templ
   frame     .templ.f1
   listbox   .templ.f1.lb -bg wheat -height 20 -width 22 -font $Fnt -yscrollcommand ".templ.f1.sb set"
   scrollbar .templ.f1.sb -bg wheat -command ".templ.f1.lb yview"
   frame  .templ.f2    -bg gold
   button .templ.f2.ok -bg gold -text OK     -font $Fnt -width 8 -command "read_circuit 0"
   button .templ.f2.cl -bg gold -text Cancel -font $Fnt -width 8 -command "destroy .templ"

   pack .templ.f1    -side top  -fill both -expand 1
   pack .templ.f1.lb -side left -fill both -expand 1
   pack .templ.f1.sb -side right  -fill y
   pack .templ.f2    -side bottom -fill x
   pack .templ.f2.ok -side left   -fill x -padx 8 -pady 5
   pack .templ.f2.cl -side right  -fill x -padx 8 -pady 5

   set files [lsort $files]
   set nr -1
   foreach cir_file $files {
	incr nr
	set fn [file tail $cir_file]
	if {$fn == $file_name} { set selnr $nr }
	.templ.f1.lb insert end $fn
   }
   if {$selnr > $nr} { set selnr $nr }
   .templ.f1.lb selection set $selnr
   .templ.f1.lb see $selnr
   wm geometry .templ +[winfo pointerx .]+[winfo pointery .]
   wm title .templ "Read circuit"
   bind .templ <KeyPress-Return> { read_circuit 0 }
   bind .templ.f1.lb <Double-1>	 { read_circuit 0 }
   bind .templ <KeyPress-Escape> { destroy .templ }
}

proc read_circuit {fname} {
   global CircuitPath CirName NameFnt CellNh GW file_name selnr
   global CELL_ARR CONN_ARR SPLIT_ARR MERGE_ARR LABEL_ARR PORT_ARR
   global NCell NConn NSplit NMerge NLabel NPort kind

   if {$fname == "0"} {
	set selnr [.templ.f1.lb curselection]
	set file_name [.templ.f1.lb get $selnr]
	destroy .templ
   } else {
	set file_name $fname
   }

#first see if no changed and unwritten data is present in the editor
   if {[see_circ_change] == 1} {
      set answ [tk_messageBox -icon error -type yesno\
         -message "Editor contains changed data!\nMust it be saved?"]
      if {$answ == "yes"} { write_cir_file $CirName }
   }

   set NCell  -1
   set NConn  -1
   set NPort  -1
   set NLabel -1
   set NSplit -1
   set NMerge -1
   set kind "unrouted"
   .frm.cv delete all

   set CirName [lindex [split $file_name .] 0]
   set f_cir [open $CircuitPath/$file_name "r"]

   while {![eof $f_cir]} {
      if {[set txt [gets $f_cir]] == ""} { continue }

      if {[lindex $txt 0] == "I"} {
         if {[llength $txt] != 4} {
            incr NCell
            set CELL_ARR($NCell) ""
            continue
         }
         set cname [lindex [split [lindex $txt 1] "~"] 0]
         read_compdef $cname
         do_add_component [expr [lindex $txt 2]*$GW] [expr [lindex $txt 3]*$GW] $cname -1
      } elseif {[lindex $txt 0] == "S"} {
         if {[llength $txt] < 4} {
            incr NSplit
            set SPLIT_ARR($NSplit) ""
            continue
         }
	 set bi 0
         if {[llength $txt] > 4} { incr bi }
	 set range [split [lindex $txt 1] "<:>"]
	 if {[lindex $range 0] != ""} {
	    make_splitter [expr [lindex $txt 2]*$GW] [expr [lindex $txt 3]*$GW]\
		[lindex $range 0] [lindex $range 1] "input" $bi
	 } else {
	    make_splitter [expr [lindex $txt 2]*$GW] [expr [lindex $txt 3]*$GW]\
		[lindex $range 1] [lindex $range 2] "output" $bi
	 }
      } elseif {[lindex $txt 0] == "M"} {
         if {[llength $txt] < 4} {
            incr NMerge
            set MERGE_ARR($NMerge) ""
            continue
         }
	 set bi 0
         if {[llength $txt] > 5} { incr bi }
	 set range [split [lindex $txt 1] "<:>"]
	 if {[lindex $range 0] != ""} {
            set w [expr [lindex $range 0] + 1]
            set spliton "input"
         } else {
            set w [expr [lindex $range 1] + 1]
            set spliton "output"
         }
	 make_merger [expr [lindex $txt 2]*$GW] [expr [lindex $txt 3]*$GW] $w $spliton $bi
      } elseif {[lindex $txt 0] == "L"} {
	 if {[llength $txt] == 3} { make_label [lindex $txt 2] [lindex $txt 1] [incr NLabel] }
      } elseif {[lindex $txt 0] == "C"} {
         if {[llength $txt] != 2} { continue }
         set tags [split [lindex $txt 1] ">"]
         set data_width [get_data_width [lindex $tags 0]]
         if {[.frm.cv gettags [lindex $tags 0]] == ""} {
            set cmp [lindex [split [lindex $tags 0] ~] 0]
            set answ [tk_messageBox -icon error -type ok\
		-message "no port [lindex $tags 0] present: component $cmp may have changed"]
            if {$answ == "ok"} { continue }
         }
         if {[.frm.cv gettags [lindex $tags 1]] == ""} {
            set cmp [lindex [split [lindex $tags 1] ~] 0]
            set answ [tk_messageBox -icon error -type ok\
		-message "no port [lindex $tags 1] present: component $cmp may have changed"]
            if {$answ == "ok"} { continue }
         }
         if {$kind == "routed"} {
            connect [lindex $tags 0] [lindex $tags 1] $data_width 0
         } else {
            connect [lindex $tags 0] [lindex $tags 1] $data_width 1
         }
      } elseif {[lindex $txt 0] == "G"} {
         set GW [lindex $txt 1]
      } elseif {[lindex $txt 0] == "R"} {
         .frm.cv configure -scrollregion \
		"[lindex $txt 1] [lindex $txt 2] [lindex $txt 3] [lindex $txt 4]"
      } elseif {[lindex $txt 0] == "B"} {
         .frm.cv create rectangle [expr [lindex $txt 1]*$GW]\
                                  [expr [lindex $txt 2]*$GW]\
                                  [expr [lindex $txt 3]*$GW]\
                                  [expr [lindex $txt 4]*$GW]\
                                  -tag "border border_line"
         .frm.cv create rectangle [expr [lindex $txt 1]*$GW]\
                                  [expr [lindex $txt 2]*$GW]\
                                  [expr [lindex $txt 3]*$GW]\
                                  [expr ([lindex $txt 2]+$CellNh)*$GW]\
                                  -fill gold -tag "border header"
         set xtxt [expr $GW*([lindex $txt 1] + [lindex $txt 3])/2.]
         set ytxt [expr $GW*([lindex $txt 2] + $CellNh/2.)]
         .frm.cv create text $xtxt $ytxt\
		-text [lindex $txt 5] -font $NameFnt -tag "border CirName"
         # make a grid
         mk_grid [expr [lindex $txt 1]*$GW] [expr ([lindex $txt 2]+$CellNh)*$GW]\
                 [expr [lindex $txt 3]*$GW] [expr [lindex $txt 4]*$GW]
      } elseif {[lindex $txt 0] == "T"} {
	if {[llength $txt] == 5} {
	  add_term [expr [lindex $txt 3]*$GW] [expr [lindex $txt 4]*$GW]\
		[lindex $txt 1] [lindex $txt 2] [incr NPort]
	}
      } elseif {[lindex $txt 0] == "N"} {
	if {[lindex $txt 1] == "line"} {
	  .frm.cv create [lindex $txt 1] [lindex $txt 2] [lindex $txt 3] [lindex $txt 4] [lindex $txt 5]\
		-fill [lindex $txt 6] -capstyle round -width [lindex $txt 7] -tag [lindex $txt 8]
	} else {
	  .frm.cv create [lindex $txt 1] [lindex $txt 2] [lindex $txt 3] [lindex $txt 4] [lindex $txt 5]\
		-fill [lindex $txt 6] -outline [lindex $txt 6] -width [lindex $txt 7] -tag [lindex $txt 8]
	}
      } elseif {[lindex $txt 0] == "K"} {
	set kind [lindex $txt 1]
      }
   }
   close $f_cir
   release_cmds
}
