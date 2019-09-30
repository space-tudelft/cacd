
set new_tname ""

# interface to give a name and width to a port
#---------------------------------------------
proc mk_termname {} {
   global Fnt new_tname XPressed YPressed
   upvar #0 data_width data_width bidir bidir

   if {[winfo exists .templ]} {
	set new_tname [.templ.nfr.en get]
	destroy .templ
   } else {
	set data_width 1
	set bidir 0
   }

   toplevel  .templ
   frame     .templ.nfr      -relief raised -bd 2 -bg wheat
   frame     .templ.wfr      -relief raised -bd 2 -bg wheat
   frame     .templ.tfr      -relief raised -bd 2 -bg wheat
   frame     .templ.cfr      -relief raised -bd 0 -bg gold
   label     .templ.nfr.lbl  -text "name:" -bg wheat -font $Fnt
   entry     .templ.nfr.en   -width 16 -bg wheat -font $Fnt
   label     .templ.wfr.lbl  -text "width:" -bg wheat -font $Fnt
   tk_optionMenu .templ.wfr.om data_width 1 2 3 4 5 6 7 8 9 10 11 12 13\
                                          14 15 16 17 18 19 20 21 22 23 24
   checkbutton .templ.tfr.btn  -text "bidirectional" -variable bidir -bg wheat -font $Fnt
   button .templ.cfr.ok  -text "OK"     -width 8 -bg gold -font $Fnt -command "set_term_name"
   button .templ.cfr.cnl -text "Cancel" -width 8 -bg gold -font $Fnt -command "destroy_templ"
   .templ.wfr.om configure -bg wheat -font $Fnt
   .templ.wfr.om.menu configure -bg wheat -font $Fnt

   pack .templ.nfr      -side top    -fill x
   pack .templ.wfr      -side top    -fill x
   pack .templ.tfr      -side top    -fill x
   pack .templ.cfr      -side bottom -fill x
   pack .templ.nfr.lbl  -side left  -padx 4 -pady 3
   pack .templ.nfr.en   -side left  -padx 4 -pady 3
   pack .templ.wfr.lbl  -side left  -padx 4
   pack .templ.wfr.om   -side right -padx 4
   pack .templ.tfr.btn  -side left  -padx 4 -pady 2
   pack .templ.cfr.ok   -side left  -padx 5 -pady 5
   pack .templ.cfr.cnl  -side right -padx 5 -pady 5

   .templ.nfr.en insert end $new_tname
   focus .templ.nfr.en
   update

   wm title .templ "add PORT"
   bind .templ <KeyPress-Return> { set_term_name }
   bind .templ <KeyPress-Escape> { destroy_templ }
}

# procedure to check the port name etc. before adding it to the circuit
#----------------------------------------------------------------------
proc set_term_name {} {
   global CirName new_tname NPort PORT_ARR NLabel LABEL_ARR XPressed YPressed
   upvar #0 data_width data_width bidir bidir

   set new_tname [.templ.nfr.en get]

   if {[test_name $new_tname]} {
	raise .templ
	return
   }
   if {[is_special_label $new_tname]} {
	show_mess "Name is special label!\nUse another name!"
	raise .templ
	return
   }

   set pname [string tolower $new_tname]
   for {set i 0} {$i <= $NPort} {incr i} {
     if {$PORT_ARR($i) == ""} {continue}
     if {[string tolower [lindex [split [lindex $PORT_ARR($i) 1] "("] 0]] == $pname} {
	show_mess "Port name already found!\nUse another name!"
	raise .templ
	return
     }
   }

   set origin [.frm.cv coords border_line]
   set x_l [lindex $origin 0]
   set x_r [lindex $origin 2]

   set driver 0
   if {[expr abs($XPressed-$x_l)] <= [expr abs($XPressed-$x_r)]} { incr driver }

   set answ "no"
   for {set i 0} {$i <= $NLabel} {incr i} {
     if {$LABEL_ARR($i) == ""} {continue}
     set label_text [lindex $LABEL_ARR($i) 0]
     if {[string tolower [lindex [split $label_text "("] 0]] == $pname} {
	if {$driver || $data_width != [get_data_width $label_text]} {
	    show_mess "Port name is label!\nUse another name!"
	    raise .templ
	    return
	} else {
	    set answ [tk_messageBox -icon warning -type yesno \
		-message "There is a label with such a name!  Are you sure?"]
	    if {$answ != "yes"} {
		raise .templ
		return
	    }
	    break
	}
     }
   }

   if {$driver} {
      if {$bidir == 0} {
         set type "in"
      } else {
         set type "*in"
      }
   } else {
      set x_l $x_r
      if {$bidir == 0} {
         set type "out"
      } else {
         set type "*out"
      }
   }

   if {$data_width == "1"} {
	set pname $new_tname
   } else {
	set pname "$new_tname\([expr $data_width-1]:0\)"
   }
   incr NPort
   destroy_templ
   add_term $x_l $YPressed $type $pname $NPort

   if {$answ == "yes"} {
	incr NLabel
	make_label $CirName.$pname $label_text $NLabel
   }
}

# procedure to add a port to the circuit
#---------------------------------------
proc add_term {xt yt type name nr} {
   global GW CirName PortFnt PORT_ARR
   global InpCol OutpCol InoutCol

   set TD [expr $GW/2.]

   if {$type == "in"} {
      .frm.cv create polygon [expr $xt-$TD] $yt\
                         $xt [expr $yt-$TD]\
                         [expr $xt+$TD] $yt\
                         $xt [expr $yt+$TD]\
                         -fill $InpCol -outline black\
			-tag "$CirName.$name T-$nr"
      .frm.cv create text [expr $xt-2*$TD] $yt -anchor e -text $name\
			-tag "$CirName.$name T-$nr" -font $PortFnt
   } elseif {$type == "*in"} {
      .frm.cv create polygon [expr $xt-$TD] $yt\
                         $xt [expr $yt-$TD]\
                         [expr $xt+$TD] $yt\
                         $xt [expr $yt+$TD]\
                         -fill $InoutCol -outline black\
			-tag "$CirName.$name T-$nr"
      .frm.cv create text [expr $xt-2*$TD] $yt -anchor e -text $name\
			-tag "$CirName.$name T-$nr" -font $PortFnt
   } elseif {$type == "out"} {
      .frm.cv create polygon [expr $xt-$TD] $yt\
                         $xt [expr $yt-$TD]\
                         [expr $xt+$TD] $yt\
                         $xt [expr $yt+$TD]\
                         -fill $OutpCol -outline black\
			-tag "$CirName.$name T-$nr"
      .frm.cv create text [expr $xt+2*$TD] $yt -anchor w -text $name\
			-tag "$CirName.$name T-$nr" -font $PortFnt
   } else {
      .frm.cv create polygon [expr $xt-$TD] $yt\
                         $xt [expr $yt-$TD]\
                         [expr $xt+$TD] $yt\
                         $xt [expr $yt+$TD]\
                         -fill $InoutCol -outline black\
			-tag "$CirName.$name T-$nr"
      .frm.cv create text [expr $xt+2*$TD] $yt -anchor w -text $name\
			-tag "$CirName.$name T-$nr" -font $PortFnt
   }
   set PORT_ARR($nr) "$type $name"
}
