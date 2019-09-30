
set save_as 0
set save_name ""

proc saveas_template {} {
   global Fnt CirName save_name

   if {[winfo exists .templ]} { destroy_templ }

   toplevel	.templ
   frame	.templ.efr	-relief raised	-bd 2 -bg wheat
   frame	.templ.cfr	-relief raised	-bd 0 -bg gold
   label	.templ.efr.lbl	-text "name:"	-font $Fnt -bg wheat
   entry	.templ.efr.en	-width 17	-font $Fnt -bg wheat
   button	.templ.cfr.ok	-width 8 -text "OK"	-font $Fnt -bg gold -command "saveas_ok; destroy .templ"
   button	.templ.cfr.cnl	-width 8 -text "Cancel"	-font $Fnt -bg gold -command "destroy .templ"

   pack .templ.efr	-side top	-fill x
   pack .templ.cfr	-side bottom	-fill x
   pack .templ.efr.lbl	-side left
   pack .templ.efr.en	-side left	-padx 5 -pady 9
   pack .templ.cfr.ok	-side left	-padx 5 -pady 5
   pack .templ.cfr.cnl	-side right	-padx 5 -pady 5

   if {$save_name == ""} { set save_name $CirName }
   .templ.efr.en insert end $save_name
   focus .templ.efr.en
   update
   wm title .templ "SaveAs circuit"
   bind .templ <KeyPress-Return> { saveas_ok; destroy .templ }
   bind .templ <KeyPress-Escape> { destroy .templ }
}

proc saveas_ok {} {
   global CircuitPath ComponentPath save_as save_name

   set save_name [.templ.efr.en get]
   if {[test_name $save_name]} { return }
   set cir_files [glob -nocomplain "$CircuitPath/*.cir"];
   foreach i_cir $cir_files {
      set cir_name [lindex [split [file tail $i_cir] .] 0]
      if {$save_name == $cir_name} {
	show_mess "There is already a circuit with this name!\nUse another name!"
	return
      }
   }
   set cmp_files [glob -nocomplain "$ComponentPath/*.cmp"];
   foreach i_cmp $cmp_files {
      set comp_name [lindex [split [file tail $i_cmp] .] 0]
      if {$save_name == $comp_name} {
	show_mess "There is a component with this name!\nUse another name!"
	return
      }
   }
   set save_as 1
   write_cir_file $save_name
   set save_as 0
}

# procedure to write a circuit_file to the circuits directory
#------------------------------------------------------------
proc write_cir_file {circuit_name} {
   global CirName BboxId CircuitPath GW Fnt save_as
   global CELL_ARR CONN_ARR PORT_ARR LABEL_ARR SPLIT_ARR MERGE_ARR
   global NCell NConn NPort N_GND NLabel NSplit NMerge InoutCol

   if {$circuit_name == ""} { return }

   if {![file exists $CircuitPath]} {
      file mkdir $CircuitPath
   }

   if {$save_as == 1} {
	set cirname $circuit_name
   } else {
	set cirname $CirName
   }

   set f_cir [open $CircuitPath/$circuit_name.cir "w"]
   set bbox [.frm.cv coords $BboxId]
   puts $f_cir "G $GW"
   puts $f_cir "R [.frm.cv cget -scrollregion]"
   puts $f_cir "B [expr round([lindex $bbox 0]/$GW)]\
                  [expr round([lindex $bbox 1]/$GW)]\
                  [expr round([lindex $bbox 2]/$GW)]\
                  [expr round([lindex $bbox 3]/$GW)] $cirname"
   set routed_list [.frm.cv find withtag router_con]
   if {$routed_list == ""} {
      puts $f_cir "K unrouted"
   } else {
      puts $f_cir "K routed"
      for {set i 0} {$i < [llength $routed_list]} {incr i} {
         set type [.frm.cv type [lindex $routed_list $i]]
         set colour [.frm.cv itemcget [lindex $routed_list $i] -fill]
         set ww [.frm.cv itemcget [lindex $routed_list $i] -width]
         puts $f_cir "N $type [.frm.cv coords [lindex $routed_list $i]] $colour $ww router_con"
      }
      for {set i 0} {$i <= $NConn} {incr i} {
         if {[.frm.cv find withtag C-$i] != ""} {
            set type [.frm.cv type C-$i]
            set colour [.frm.cv itemcget C-$i -fill]
            set ww [.frm.cv itemcget C-$i -width]
            puts $f_cir "N $type [.frm.cv coords C-$i] $colour $ww router_con"
         }
      }
   }

   for {set i 0} {$i <= $NPort} {incr i} {
	if {$PORT_ARR($i) == ""} {continue}
	set c [.frm.cv coords T-$i]
	puts $f_cir "T $PORT_ARR($i)\
		[expr round([lindex $c 2]/$GW)]\
		[expr round([lindex $c 1]/$GW)]"
   }

   for {set i 0} {$i <= $NCell} {incr i} {
     if {[set cname $CELL_ARR($i)] == ""} {
	puts $f_cir "I"
     } else {
	set c [.frm.cv bbox ${cname}_bb]
	puts $f_cir "I $CELL_ARR($i)\
		[expr round([lindex $c 0]/$GW)]\
		[expr round([lindex $c 1]/$GW)]"
     }
   }

   for {set i 0} {$i <= $NSplit} {incr i} {
     if {$SPLIT_ARR($i) == ""} {
        puts $f_cir "S"
     } else {
        set c [.frm.cv bbox S-$i]
        set t [.frm.cv bbox S-${i}.in]
        if {[.frm.cv itemcget S-${i}.in -fill] == $InoutCol} {
            puts $f_cir "S $SPLIT_ARR($i)\
                [expr round(0.5*([lindex $t 0]+[lindex $t 2])/$GW)]\
                [expr round([lindex $c 1]/$GW)] bi"
        } else {
            puts $f_cir "S $SPLIT_ARR($i)\
                [expr round(0.5*([lindex $t 0]+[lindex $t 2])/$GW)]\
                [expr round([lindex $c 1]/$GW)]"
        }
     }
   }

   for {set i 0} {$i <= $NMerge} {incr i} {
     if {$MERGE_ARR($i) == ""} {
	puts $f_cir "M"
     } else {
	set c [.frm.cv bbox M-${i}_bb]
        set bi ""
        if {[.frm.cv find withtag M-${i}.i] != ""} {
           set spliton "o"
           if {[.frm.cv itemcget M-${i}.i -fill] == $InoutCol} { set bi " bi" }
        } else {
           set spliton "i"
           if {[.frm.cv itemcget M-${i}.o -fill] == $InoutCol} { set bi " bi" }
        }
	puts $f_cir "M $MERGE_ARR($i)\
		[expr round([lindex $c 0]/$GW)]\
		[expr round([lindex $c 1]/$GW)] $spliton$bi"
     }
   }

   for {set i 0} {$i <= $NLabel} {incr i} {
	if {[set lname $LABEL_ARR($i)] == ""} {continue}
	set pname [split [lindex $lname 1] .]
	if {$save_as == 1 && [lindex $pname 0] == $CirName} {
	    puts $f_cir "L [lindex $lname 0] $cirname.[lindex $pname 1]"
	} else {
	    puts $f_cir "L $lname"
	}
   }

   for {set i 0} {$i <= $NConn} {incr i} {
      if {[set cname $CONN_ARR($i)] == ""} {continue}
      if {$save_as == 1} {
	set iname [lindex [split $cname ">"] 0]
	set oname [lindex [split $cname ">"] 1]
	if {[lindex [split $iname .] 0] == $CirName} {
	    set iname "$cirname.[lindex [split $iname .] 1]"
	}
	if {[lindex [split $oname .] 0] == $CirName} {
	    set oname "$cirname.[lindex [split $oname .] 1]"
	}
	puts $f_cir "C $iname>$oname"
      } else {
	puts $f_cir "C $cname"
      }
   }

   close $f_cir

   if {$circuit_name == $CirName || $save_as == 1} {
	show_info "File '$CircuitPath/$circuit_name.cir' (re)written\n"
   }
}
