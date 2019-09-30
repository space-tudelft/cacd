
set tag1 0
set tag2 0

proc findport {name ptag} {
   global CompIn CompOut InpCol OutpCol InoutCol

   foreach port $CompIn {
     if {[string index $port 0] == "*"} {
	if {$port == "*$name"} {
	    if {[.frm.cv itemcget $ptag -fill] == $InoutCol} { return 1 }
	    return 0
	}
     } elseif {$port == $name} {
	if {[.frm.cv itemcget $ptag -fill] == $InpCol} { return 1 }
	return 0
     }
   }
   foreach port $CompOut {
     if {[string index $port 0] == "*"} {
	if {$port == "*$name"} {
	    if {[.frm.cv itemcget $ptag -fill] == $InoutCol} { return 1 }
	    return 0
	}
     } elseif {$port == $name} {
	if {[.frm.cv itemcget $ptag -fill] == $OutpCol} { return 1 }
	return 0
     }
   }
   return 0
}

proc move_labels {old new nr} {
   global LABEL_ARR NLabel

   for {set i 0} {$i <= $NLabel} {incr i} {
      if {[set ptag [lindex $LABEL_ARR($i) 1]] == ""} {continue}
      set inst [lindex [split $ptag "."] 0]
      if {$inst == $old} {
	.frm.cv delete L-$i
	set port [lindex [split $ptag "."] 1]
	if {[findport $port $ptag]} {
	    make_label $new~$nr.$port [lindex $LABEL_ARR($i) 0] $i
	} else {
	    set LABEL_ARR($i) ""
	}
      }
   }
   while {$NLabel >= 0 && $LABEL_ARR($NLabel) == ""} {incr NLabel -1}
}

proc move_connect {old new nr} {
   global CONN_ARR NConn

   for {set i 0} {$i <= $NConn} {incr i} {
      if {$CONN_ARR($i) == ""} {continue}
      set n [string first ">"   $CONN_ARR($i)]
      set f [string first $old. $CONN_ARR($i)]
      if {$f == 0 || $f == [expr $n+1]} {
	.frm.cv delete C-$i
	if {$f == 0} {
	    set ptag [string range $CONN_ARR($i) 0 [expr $n-1]]
	} else {
	    set ptag [string range $CONN_ARR($i) $f end]
	}
	set port [lindex [split $ptag "."] 1]
	if {[findport $port $ptag]} {
	    if {$f == 0} {
		set from_tag $new~$nr.$port
		set to_tag [string range $CONN_ARR($i) [expr $n+1] end]
	    } else {
		set from_tag [string range $CONN_ARR($i) 0 [expr $n-1]]
		set to_tag $new~$nr.$port
	    }
	    set CONN_ARR($i) $from_tag>$to_tag
	    draw_con $from_tag $to_tag [get_data_width $ptag] $i
	} else {
	    set CONN_ARR($i) ""
	}
      }
   }
   while {$NConn >= 0 && $CONN_ARR($NConn) == ""} {incr NConn -1}
}

# interface to generate a label
#------------------------------
proc mk_label {} {
   global CirName Cmd Fnt LABEL_ARR NLabel XPressed YPressed tag1 tag2 INOUT MN_INPUT OUTPUT

   if {[winfo exists .templ]} { destroy_templ }

   if {$Cmd == "RENAME"} {
	set tags [.frm.cv gettags [.frm.cv find closest $XPressed $YPressed]]
	set tag1 [lindex $tags 0]
	set tag2 [lindex $tags 1]

	if {[string range $tag1 0 1] == "L-"} {
	    set i [string range $tag1 2 end]
	    set name [lindex $LABEL_ARR($i) 0]
	} elseif {[set i [string first "." $tag1]] > 0 && [string range $tag2 0 1] == "T-"} {
	    incr i
	    set name [string range $tag1 $i end]
	} elseif {[string first "~" $tag1] > 0} {
	    set oldname [lindex [split $tag1 "~"] 0]
	    if {[set i [.lb curselection]] == ""} {
		show_mess "You must first select a component name!"
		return
	    }
	    if {[set name [.lb get $i]] == $CirName} {
		show_mess "You cannot rename component to itself!"
		return
	    }
	    if {$name == $oldname} {return}
	    set answ [tk_messageBox -icon warning -type yesno \
		-message "Renaming component\n'$oldname' into '$name'\nAre you sure?"]
	    if {$answ != "yes"} {return}
	    read_compdef $name
	    set i [lindex [split $tag1 "~"] 1]
	    set c [.frm.cv coords ${tag1}_bb]
	    do_add_component [lindex $c 0] [lindex $c 1] $name $i
	    move_labels $tag1 $name $i
	    move_connect $tag1 $name $i
	    .frm.cv delete $tag1
	    return
	} else { return }
   } else {
	if {[set tag1 [get_port_tag $XPressed $YPressed]] == ""} {
	    show_mess "Label can only be added to a port!"
	    return
	}
	set comp_name [lindex [split $tag1 .] 0]
	set nr ""
	if {[string range $comp_name 0 1] == "S-"} {
	    set nr [string range $comp_name 2 end]
	}

	set type [get_porttype $tag1]

	if {$Cmd == "ADD_VDD" || $Cmd == "ADD_VSS"} {
	    if {[port_is_connected $tag1] > 0} {
		show_mess "Port is already connected!\nNo special label possible!"
		return
	    }
	    if {$type == $MN_INPUT || $type == $OUTPUT} {
		show_mess "Special label cannot be added to a driver port!"
		return
	    }
	    if {$type == $INOUT} {
		show_mess "Special label cannot be added to a bidirectional port!"
		return
	    }
	    if {$nr != "" && [get_data_width $tag1] > 1} {
		show_mess "Splitter input data width > 1!"
		return
	    }
	    incr NLabel
	    if {$Cmd == "ADD_VDD"} {
		make_label $tag1 "vdd" $NLabel
	    } else {
		make_label $tag1 "vss" $NLabel
	    }
	    return
	}
	set rv [port_is_connected $tag1]
	if {$rv == 1} {
	    show_mess "Port has already a label!\nUse rename label?"
	    return
	}
	if {$rv < 0 && $type != $MN_INPUT && $type != $OUTPUT && $type != $INOUT} {
	    show_mess "Label can only be added to a driver port!  Use copy label?"
	    return
	}
	set name [lindex [split $tag1 .] 1]$nr
   }
   set i [string first "(" $name]
   if {$i > 0} {
	set name [string range $name 0 [expr $i-1]]
   }
   toplevel    .templ
   frame       .templ.efr      -relief raised -bd 2 -bg wheat
   frame       .templ.cfr      -relief raised -bd 0 -bg gold
   label       .templ.efr.lbl  -text "name:" -font $Fnt -bg wheat
   entry       .templ.efr.en   -width 16 -font $Fnt -bg wheat
   button .templ.cfr.ok  -text "OK"     -width 8 -font $Fnt -bg gold -command "mk_label_ok"
   button .templ.cfr.cnl -text "Cancel" -width 8 -font $Fnt -bg gold -command "destroy .templ"

   pack .templ.efr      -side top    -fill x
   pack .templ.cfr      -side bottom -fill x
   pack .templ.efr.lbl  -side left   -padx 5
   pack .templ.efr.en   -side left   -padx 5 -pady 9
   pack .templ.cfr.ok   -side left   -padx 5 -pady 5
   pack .templ.cfr.cnl  -side right  -padx 5 -pady 5

   .templ.efr.en insert end $name
   focus .templ.efr.en

   if {$Cmd == "ADD_LABEL"} {
	wm title .templ "add LABEL"
   } elseif {[string range $tag1 0 1] == "L-"} {
	wm title .templ "rename LABEL"
   } else {
	wm title .templ "rename PORT"
   }
   bind .templ <KeyPress-Return> { mk_label_ok }
   bind .templ <KeyPress-Escape> { destroy .templ }
}

proc mk_label_ok {} {
   global Cmd tag1 tag2
   if {$Cmd == "RENAME"} {
	set rv [do_rename $tag1 $tag2]
   } else {
	set rv [do_mk_label $tag1]
   }
   if {$rv} {
	destroy .templ
   } else {
	raise .templ
   }
}

proc do_rename {tag1 tag2} {
   global NLabel LABEL_ARR CirName XPressed YPressed
   global NConn CONN_ARR NPort PORT_ARR

   set rename_name [.templ.efr.en get]

   if {[test_name $rename_name]} { return 0 }

   if {[string range $tag1 0 1] == "L-"} {
	set i [string range $tag1 2 end]
	set name [lindex $LABEL_ARR($i) 0]
	set i [string first "(" $name]
	if {$i > 0} {
	    append rename_name [string range $name $i end]
	} elseif {[is_special_label $rename_name]} {
	    for {set i 0} {$i <= $NLabel} {incr i} {
		if {[lindex $LABEL_ARR($i) 0] == $name} {
		    set port_tag [lindex $LABEL_ARR($i) 1]
		    set coords [.frm.cv coords $port_tag]
		    set comp_name [lindex [split $port_tag .] 0]
		    if {[is_driver2 [lindex $coords 0] $comp_name]} {
			show_mess "Name '$rename_name' cannot be connected to a driver port!\nUse another name!"
			return 0
		    }
		}
	    }
	}
	for {set i 0} {$i <= $NLabel} {incr i} {
	    if {[lindex $LABEL_ARR($i) 0] == $name} {
		.frm.cv delete L-$i
		make_label [lindex $LABEL_ARR($i) 1] $rename_name $i
	    }
	}
	return 1
   }

   if {[is_special_label $rename_name]} {
	show_mess "Name is special label!\nUse another name!"
	return 0
   }

#check for equal port names
   set new_term_name [string tolower $rename_name]
   for {set i 0} {$i <= $NPort} {incr i} {
     set pname [lindex $PORT_ARR($i) 1]
     if {$pname != ""} {
	if {[string tolower [lindex [split $pname "("] 0]] == $new_term_name} {
	   show_mess "Port name already found!\nUse another name!"
	   return 0
	}
     }
   }

   set i [string first "(" $tag1]
   if {$i > 0} {
	append rename_name [string range $tag1 $i end]
   }

   destroy .templ
   .frm.cv delete $tag2
   set origin [.frm.cv coords border_line]
   set x_l [lindex $origin 0]
   set x_r [lindex $origin 2]
   if {[expr abs($XPressed-$x_l)] > [expr abs($XPressed-$x_r)]} { set x_l $x_r }
   set i [string range $tag2 2 end]
   add_term $x_l $YPressed [lindex $PORT_ARR($i) 0] $rename_name $i

   for {set i 0} {$i <= $NLabel} {incr i} {
      if {[lindex $LABEL_ARR($i) 1] == $tag1} {
         .frm.cv delete L-$i
         make_label $CirName.$rename_name [lindex $LABEL_ARR($i) 0] $i
         return 1
      }
   }
   for {set i 0} {$i <= $NConn} {incr i} {
      set plist [split $CONN_ARR($i) >]
      if {[lindex $plist 0] == $tag1} {
         set CONN_ARR($i) "$CirName.$rename_name>[lindex $plist 1]"
      }
      if {[lindex $plist 1] == $tag1} {
         set CONN_ARR($i) "[lindex $plist 0]>$CirName.$rename_name"
      }
   }
   return 1
}

# procedure to make a copy of a label
#------------------------------------
proc copy_label {x y move} {
   global CirName SelTag LABEL_ARR NLabel INOUT MN_INPUT OUTPUT

   if {[set to_port [get_port_tag $x $y]] == ""} {
	show_mess "Label can only be added to a port!"
	return
   }

   if {[port_is_connected $to_port] > 0} {
	show_mess "Port is already connected, so no label possible!"
	return
   }

   set idx [string range $SelTag 2 end]
   set from_port [lindex $LABEL_ARR($idx) 1]
   set typf [get_porttype $from_port]
   set type [get_porttype $to_port]
   if {$typf == $INOUT} {
     if {$type != $INOUT} {
	show_mess "Label may only connect bidirectional ports!"
	return
     }
   } elseif {$type == $INOUT} {
	show_mess "Label may only connect non-bidirectional ports!"
	return
   }
   if {$move} {
     if {$type != $typf} {
	show_mess "Label cannot move to different port type!"
	return
     }
   } elseif {$type == $MN_INPUT || $type == $OUTPUT} {
	show_mess "Label may not connect 2 driver ports!"
	return
   }

   set label_text [lindex $LABEL_ARR($idx) 0]
   set comp_t [lindex [split $to_port .] 0]
   if {$type != $INOUT} {
	if {$typf != $MN_INPUT && $typf != $OUTPUT} {
# from_port is not a driver, lookup the driver port
	  for {set i 0} {$i <= $NLabel} {incr i} {
	    if {[lindex $LABEL_ARR($i) 0] == $label_text && $i != $idx} {
		set typf [get_porttype [set from_port [lindex $LABEL_ARR($i) 1]]]
		if {$typf == $MN_INPUT || $typf == $OUTPUT} {break}
	    }
	  }
	}
	set comp_f [lindex [split $from_port .] 0]
	if {$comp_f != $CirName && $comp_t == $comp_f} {
	  show_mess "Label may not connect ports of same component!"
	  return
	}
   }

   set label_width [get_data_width $label_text]
   if {[is_splitter_input $to_port] < 0} {
     if {[get_data_width $to_port] != $label_width} {
	show_mess "Label data width != width of port!"
	return
     }
   } else {
	set dw [get_data_width $to_port]
	if {$dw == 1000} {
	    if {$label_width < [get_min_width $comp_t]} {
		show_mess "Label data width < minimum width of splitter input!"
		return
	    }
	} elseif {$label_width != $dw} {
		show_mess "Label data width != width of splitter input!"
		return
	}
   }

   if {$move} {
	.frm.cv delete $SelTag
	make_label $to_port $label_text $idx
   } else {
	make_label $to_port $label_text [incr NLabel]
   }
}

# procedure to add a label to a port
#-----------------------------------
proc do_mk_label {port_tag} {
   global CONN_ARR NConn NLabel

   set connect [port_is_connected $port_tag]
   set label_list [split [.templ.efr.en get] "#"]
   set label_text [lindex $label_list 0]

   if {[is_special_label $label_text]} {
	if {$connect > 0} {
	  show_mess "Connect cannot be changed into special label!"
	} else {
	  show_mess "Special label cannot be added to a driver port!"
	}
	return 1
   }

   if {[test_name $label_text]} { return 0 }

   set data_w [get_data_width $port_tag]

   if {$connect > 0} {
# set port_tag to driver port (if needed)
# and get new data_w (if needed)
     for {set i 0} {$i <= $NConn} {incr i} {
	set l [split $CONN_ARR($i) ">"]
	set p1 [lindex $l 0]
	set p2 [lindex $l 1]
	if {$p1 == $port_tag || $p2 == $port_tag} {
	    if {$p2 == $port_tag} {
		set port_tag $p1
		set p2 $p1
	    }
	    if {$data_w == 1000} { set data_w [get_data_width $p2] }
	    break
	}
     }
   }

   if {$data_w > 1} {
     if {$data_w == 1000} {
	set comp_name [lindex [split $port_tag .] 0]
	set data_w [get_min_width $comp_name]
	set lw [lindex $label_list 1]
	if {$lw != ""} {
	    if {![regexp {^[1-9][0-9]*$} $lw]} {
		show_mess "Label width must be a number!\nTry again!"
		return 0
	    }
	    if {$lw > $data_w} { set data_w $lw }
	}
     }
     append label_text "([expr $data_w-1]:0)"
   }

   if {[check_label_text $label_text $port_tag $connect] < 0} { return 0 }

   if {$connect > 0} {
     for {set i 0} {$i <= $NConn} {incr i} {
	set l [split $CONN_ARR($i) ">"]
	set p1 [lindex $l 0]
	set p2 [lindex $l 1]
	if {$p1 == $port_tag || $p2 == $port_tag} {
	    set CONN_ARR($i) ""
	    .frm.cv delete C-$i
	    incr NLabel
	    if {$p1 == $port_tag} {
		make_label $p2 $label_text $NLabel
	    } else {
		make_label $p1 $label_text $NLabel
	    }
	}
     }
   }

   incr NLabel
   make_label $port_tag $label_text $NLabel
   return 1
}

# procedure to see if there is no label with the same name and a different data width or
# there is no port with the same name that is not connected with a label with the same name
#----------------------------------------------------------------------------
proc check_label_text {label_text port_tag connect} {
   global NLabel LABEL_ARR NPort PORT_ARR CirName

   set l_txt [string tolower $label_text]
   set proper_name [lindex [split $l_txt "("] 0]
   set data_width [get_data_width $label_text]

   for {set i 0} {$i <= $NLabel} {incr i} {
	if {$LABEL_ARR($i) == ""} {continue}
	set name [string tolower [lindex [split [lindex $LABEL_ARR($i) 0] "("] 0]]
	if {$name == $proper_name} {
            show_mess "There is already a label with this name!  Use another name!"
            return -1
	}
   }

   set comp_name [lindex [split $port_tag .] 0]
   set port_name [lindex [split $port_tag .] 1]
   for {set i 0} {$i <= $NPort} {incr i} {
	if {$PORT_ARR($i) == ""} {continue}
	set port [lindex $PORT_ARR($i) 1]
	if {$port == $port_name && $comp_name == $CirName} {continue}
	if {$connect > 0 && [in_same_connect $port_tag $CirName.$port]} {continue}
	set name [string tolower [lindex [split $port "("] 0]]
	if {$name == $proper_name} {
	    set coords [.frm.cv coords $CirName.$port]
	    if {[is_driver2 [lindex $coords 0] $CirName]} {
		show_mess "There is another driver port with this name!  Use another name!"
		return -1
	    }
	    if {[lindex [split $port "("] 1] != [lindex [split $l_txt "("] 1]} {
		show_mess "There is another port with this name, which has another data width!  Use another name!"
		return -1
	    }
	    if {[port_is_connected $CirName.$port] > 0} {
		show_mess "There is a connected port with this name!  Use another name!"
		return -1
	    }
	    #if {$comp_name == $CirName} {
		set answ [tk_messageBox -icon warning -type yesno \
		    -message "There is a port with such a name!  Are you sure?"]
		if {$answ != "yes"} { return -1 }
	    #}
	    incr NLabel
	    make_label $CirName.$port $label_text $NLabel
	}
   }

   return 1
}

# procedure to see if a port has a connection to it
# return_values:
# -1: not connected
#  1: connected via a label
#  2: connected via a wire
#--------------------------------------------------
proc port_is_connected {port} {
   if {[is_connected $port] >= 0} {return 2}
   if {[has_label $port] >= 0} {return 1}
   return -1
}

proc in_same_connect {port1 port2} {
   global CONN_ARR NConn
   for {set i 0} {$i <= $NConn} {incr i} {
	if {$CONN_ARR($i) == ""} {continue}
	set pl [split $CONN_ARR($i) ">"]
	if {[lindex $pl 0] == $port1 && [lindex $pl 1] == $port2} {return 1}
   }
   return 0
}

# procedure to see if the label has enough bits for input to a the splitter
#--------------------------------------------------------------------------
proc check_min_label_width {label_text s_idx} {
   global SPLIT_ARR

   set data_width [get_data_width $label_text]
   set rng [string trim $SPLIT_ARR($s_idx) "<>"]
   set rb1 [lindex [split $rng ":"] 0]
   set rb2 [lindex [split $rng ":"] 1]
   set min_width [expr abs($rb2 - $rb1)]
   incr min_width
   if {$data_width < $min_width} {
	show_mess "Label data width < width of splitter input!"
	return -1
   }
   return 1
}

# procedure to see if a label is a gnd, vss or vdd label
#-------------------------------------------------------
proc is_special_label {name} {
   set name [string tolower $name]
   foreach i "gnd vss vdd" {
      if {$i == $name} { return 1 }
   }
   return 0
}

proc make_label {port_tag label_text nr} {
   global CirName LABEL_ARR GW PortFnt

   set coords [.frm.cv coords $port_tag]
   set comp_name [lindex [split $port_tag .] 0]

   if {$comp_name == $CirName} {
	set first_tag $port_tag
   } else {
	set first_tag $comp_name
   }

   set LABEL_ARR($nr) "$label_text $port_tag"

   set x [lindex $coords 0]
   set y [lindex $coords 1]
   set TD [expr $GW/2. + 2]
   if {[is_driver2 $x $comp_name]} {
	set x [lindex $coords 4]
	set tmp [.frm.cv create polygon $x $y\
		[expr $x+$GW] [expr $y-$GW+1]\
		[expr $x+$GW+1] [expr $y-$GW+1]\
		[expr $x+$GW+1] [expr $y+$GW-1]\
		[expr $x+$GW] [expr $y+$GW-1]\
		-fill green -tag "L-$nr $first_tag"]
	.frm.cv create text [expr $x+$TD] $y -text $label_text -anchor w\
		-fill black -tag "L-$nr $first_tag" -font $PortFnt
	set coords [.frm.cv bbox L-$nr]
	set str_width [expr [lindex $coords 2] - [lindex $coords 0] + 2]
	.frm.cv coords $tmp $x $y\
		[expr $x+$GW] [expr $y-$GW+1]\
		[expr $x+$str_width] [expr $y-$GW+1]\
		[expr $x+$str_width] [expr $y+$GW-1]\
		[expr $x+$GW] [expr $y+$GW-1]
   } else {
	set tmp [.frm.cv create polygon $x $y\
		[expr $x-$GW] [expr $y-$GW+1]\
		[expr $x-$GW-1] [expr $y-$GW+1]\
		[expr $x-$GW-1] [expr $y+$GW-1]\
		[expr $x-$GW] [expr $y+$GW-1]\
		-fill green -tag "L-$nr $first_tag"]
	.frm.cv create text [expr $x-$TD] $y -text $label_text -anchor e\
		-fill black -tag "L-$nr $first_tag" -font $PortFnt
	set coords [.frm.cv bbox L-$nr]
	set str_width [expr [lindex $coords 2] - [lindex $coords 0] + 2]
	.frm.cv coords $tmp $x $y\
		[expr $x-$GW] [expr $y-$GW+1]\
		[expr $x-$str_width] [expr $y-$GW+1]\
		[expr $x-$str_width] [expr $y+$GW-1]\
		[expr $x-$GW] [expr $y+$GW-1]
   }
}
