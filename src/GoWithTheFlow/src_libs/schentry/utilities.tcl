
# procedure to make a grid inside a circuit_area
#-----------------------------------------------
proc mk_grid {x1 y1 x2 y2} {
   global GW GridCol

   set x $x1
   while {$x < $x2} {
	.frm.cv create line $x $y1 $x $y2 -fill $GridCol -tag "border grid_line"
	set x [expr $x+$GW]
   }
   set y $y1
   while {$y < $y2} {
	.frm.cv create line $x1 $y $x2 $y -fill $GridCol -tag "border grid_line"
	set y [expr $y+$GW]
   }
   .frm.cv lower grid_line
}

# procedure to show a message on a message_box
#---------------------------------------------
proc show_mess {message} {
   set answ [tk_messageBox -icon error -type ok -message "$message"]
}

# procedure which determines if port_tag is the input of a splitter
# it returns the number of the splitter if found, -1 otherwise
#-------------------------------------------------------------
proc is_splitter_input {port_tag} {
   set tmp [split $port_tag "."]
   if {[lindex $tmp 1] == "in"} {
      return [string range [lindex $tmp 0] 2 end]
   }
   return -1
}

# procedure to determine the type of port from its tag
#-----------------------------------------------------
proc get_porttype {port_tag} {
   global CirName InpCol OutpCol InoutCol
   global INPUT OUTPUT MN_INPUT MN_OUTPUT INOUT UNKNOWN

   set comp_name [lindex [split $port_tag .] 0]
   set port_col [.frm.cv itemcget $port_tag -fill]

   if {$port_col == $InoutCol} {
      return $INOUT
   }
   if {$comp_name == $CirName} {
      if {$port_col == $InpCol} {
         return $MN_INPUT
      } elseif {$port_col == $OutpCol} {
         return $MN_OUTPUT
      }
   } else {
      if {$port_col == $InpCol} {
         return $INPUT
      } elseif {$port_col == $OutpCol} {
         return $OUTPUT
      }
   }
   return $UNKNOWN
}

# procedure to determine the data width from the port tag
#--------------------------------------------------------
proc get_data_width {ptag} {
   global SPLIT_ARR MERGE_ARR

   if {[string range $ptag 0 1] == "S-"} {
      set p [split $ptag .]
      set n [string range [lindex $p 0] 2 end]
      set tmp [split $SPLIT_ARR($n) "<:>"]
      if {[lindex $p 1] == "in"} {
	if {[lindex $tmp 0] == ""} { return 1000 }
	return [expr [lindex $tmp 0] - [lindex $tmp 1] + 1]
      } else {
	if {[lindex $tmp 0] != ""} { return 1000 }
	return [expr [lindex $tmp 1] - [lindex $tmp 2] + 1]
      }
   } elseif {[string range $ptag 0 1] == "M-"} {
      set p [split $ptag .]
      set n [string range [lindex $p 0] 2 end]
      set tmp [split $MERGE_ARR($n) "<:>"]
      if {[string index [lindex $p 1] 0] == "i"} {
        # input
        if {[string first "<" $MERGE_ARR($n)] >= 0} { return 1 }
	if {[lindex $tmp 0] == ""} { return 1000 }
	return [expr [lindex $tmp 0] - [lindex $tmp 1] + 1]
      } else {
        # output
        if {[string first ">" $MERGE_ARR($n)] >= 0} { return 1 }
	if {[lindex $tmp 0] != ""} { return 1000 }
	return [expr [lindex $tmp 1] - [lindex $tmp 2] + 1]
      }
   }
   set tmp [split $ptag "(:)"]
   set n [llength $tmp]
   if {$n == 1} { return 1 }
   if {$n == 4} { return [expr [lindex $tmp 1] - [lindex $tmp 2] + 1] }
   tk_messageBox -icon error -type ok\
	-message "Error in the determination of data width of tag:$ptag"
   return -1
}

# procedure to determine the minimum data width to connect to a splitter
#----------------------------------------------------------------------
proc get_min_width {tag} {
   global SPLIT_ARR MERGE_ARR

   set idx [string range $tag 2 end]
   if {[string range $tag 0 1] == "S-"} {
      set tmp [split $SPLIT_ARR($idx) "<:>"]
   } elseif {[string range $tag 0 1] == "M-"} {
      set tmp [split $MERGE_ARR($idx) "<:>"]
   }
   if {[lindex $tmp 0] != ""} {
	set min [lindex $tmp 0]
   } else {
	set min [lindex $tmp 1]
   }
   return [incr min]
}

# procedure to get the port_tag from the port at coords x y
#----------------------------------------------------------
proc get_port_tag {x y} {
   global GW
   set tags [.frm.cv gettags [.frm.cv find enclosed [expr $x-$GW] [expr $y-$GW] [expr $x+$GW] [expr $y+$GW]]]
   set p [lindex $tags 0]
   if {[string first "." $p] > 0} { return $p }
   set p [lindex $tags 1]
   if {[string first "." $p] > 0} { return $p }
   return ""
}

# procedure to get the coords where the port must be connected
#-------------------------------------------------------------
proc get_con_coords {port} {
   set c [.frm.cv coords $port]
   return "[lindex $c 2] [lindex $c 1]"
}

# procedure which return the maximum value of the two inputs
#-----------------------------------------------------------
proc MAX {var1 var2} {
   if {$var1 >= $var2} { return $var1 }
   return $var2
}

# procedure which return the minimum value of the two inputs
#-----------------------------------------------------------
proc MIN {var1 var2} {
   if {$var1 <= $var2} { return $var1 }
   return $var2
}

# procedure to check if the outline is totally inside the border_line
# returns 1 if this is the case: 0 otherwise
#-------------------------------------------
proc check_area_inside {} {
   global BboxId CellNh GW

   set bbox [.frm.cv coords $BboxId]
   set xlb [lindex $bbox 0]
   set xrb [lindex $bbox 2]
   set ytb [expr [lindex $bbox 1] + $CellNh*$GW]
   set ybb [lindex $bbox 3]
   set cbox [.frm.cv coords outline]
   set xlc [lindex $cbox 0]
   set xrc [lindex $cbox 2]
   set ytc [lindex $cbox 1]
   set ybc [lindex $cbox 3]
   if {$xlc <= $xlb} { return 0 }
   if {$ytc <= $ytb} { return 0 }
   if {$xrc >= $xrb} { return 0 }
   if {$ybc >= $ybb} { return 0 }
   return 1
}

# procedure to check if the coordinates x,y are inside the circuit_area
# returns 1 if this is the case: 0 otherwise
#-------------------------------------------
proc check_point_inside {x y} {
   global BboxId CellNh GW

   set bbox [.frm.cv coords $BboxId]
   set xlb [lindex $bbox 0]
   set xrb [lindex $bbox 2]
   set ytb [expr [lindex $bbox 1] + $CellNh*$GW]
   set ybb [lindex $bbox 3]
   if {$x < $xlb} { return 0 }
   if {$y < $ytb} { return 0 }
   if {$x > $xrb} { return 0 }
   if {$y > $ybb} { return 0 }
   return 1
}

# procedure to check if the coordinates y is inside the circuit_area
# returns 1 if this is the case: 0 otherwise
#-------------------------------------------
proc check_y_inside {y} {
   global BboxId CellNh GW

   set bbox [.frm.cv coords $BboxId]
   set ytb [expr [lindex $bbox 1] + $CellNh*$GW]
   set ybb [lindex $bbox 3]
   if {$y <= $ytb} { return 0 }
   if {$y >= $ybb} { return 0 }
   return 1
}

# procedure to see if a circuit has been modified
# returns 1 if this is the case: 0 otherwise
#-------------------------------------------
proc see_circ_change {} {
   global CirName CircuitPath

   if {$CirName == ""} {
# schematic entry is empty
      return 0
   }
   if {[file exists $CircuitPath/$CirName.cir] == 0} {
# no file of the circuit made so far has been made
      return 1
   }
# make a tempory file of the circuit present
   write_cir_file {Tmp-Cmp-Circ}
   if {[file size $CircuitPath/Tmp-Cmp-Circ.cir] != \
       [file size $CircuitPath/$CirName.cir]} {
      file delete $CircuitPath/Tmp-Cmp-Circ.cir
      return 1
   }

   set rv 0
   set fp_new [open $CircuitPath/Tmp-Cmp-Circ.cir "r"]
   set fp_ref [open $CircuitPath/$CirName.cir "r"]
   while {![eof $fp_new]} {
      if {[gets $fp_new] != [gets $fp_ref]} {
	incr rv
	break
      }
   }
   close $fp_new
   close $fp_ref
   file delete $CircuitPath/Tmp-Cmp-Circ.cir
   return $rv
}

# procedure to show info_text in the lower text_window
#-----------------------------------------------------
proc show_info {info_txt} {
   .fr3.txt insert end $info_txt
   .fr3.txt see end
}

# procedure to determine if a port has a label
#---------------------------------------------
proc has_label {port} {
   global LABEL_ARR NLabel

   for {set i 0} {$i <= $NLabel} {incr i} {
	if {[lindex $LABEL_ARR($i) 1] == $port} { return $i }
   }
   return -1
}

# procedure to determine if the port is a driver_port
#----------------------------------------------------
proc is_driver {port_tag} {
   global MN_INPUT OUTPUT
   set type [get_porttype $port_tag]
   if {$type == $MN_INPUT || $type == $OUTPUT} { return 1 }
   return 0
}

proc is_driver2 {x comp_name} {
   global CirName BboxId

   if {$comp_name == $CirName} {
      set bb [.frm.cv coords $BboxId]
      set xl [lindex $bb 0]
      set xr [lindex $bb 2]
      if {($x < $xl) || ($x < $xr && ([expr $x-$xl] < [expr $xr-$x]))} { return 1 }
   } else {
      set bb [.frm.cv coords $comp_name]
      set xl [lindex $bb 0]
      set xr [lindex $bb 2]
      if {($x > $xr) || ($x > $xl && ([expr $x-$xl] > [expr $xr-$x]))} { return 1 }
   }
   return 0
}

# procedure to determine if a inout port is a receiver port
# returns 1 if true, 0 otherwise
#-------------------------------
proc is_inout_receiver {port} {
   global NConn CONN_ARR

   for {set i 0} {$i <= $NConn} {incr i} {
      if {[lindex [split $CONN_ARR($i) ">"] 1] == $port} { return $i }
   }
   return -1
}

# procedure to determine if a inout port is a driver port
# returns 1 if true, 0 otherwise
#-------------------------------
proc is_inout_driver {port} {
   global NConn CONN_ARR

   for {set i 0} {$i <= $NConn} {incr i} {
      if {[lindex [split $CONN_ARR($i) ">"] 0] == $port} { return $i }
   }
   return -1
}

proc is_connected {port} {
   global NConn CONN_ARR

   for {set i 0} {$i <= $NConn} {incr i} {
	set pl [split $CONN_ARR($i) ">"]
	if {$port == [lindex $pl 0] || $port == [lindex $pl 1]} { return $i }
   }
   return -1
}

# procedure to test for reserved VHDL words
proc reserved_word {name} {
   set n [string tolower $name]
   foreach w "abs all and bus end for if in inout is map mod nand new nor not of on or out port to use xnor xor" {
      if {$w == $n} { return 1 }
   }
   return 0
}

# procedure to test for correct name
proc test_name {name} {
   if {$name == ""} {
	show_mess "No name given!"
	return 1
   }
   if {![regexp {^[A-Za-z][A-Za-z0-9_]*$} $name]} {
	show_mess "No correct name!\nUse another name!"
	return 1
   }
   if {[string index $name end] == "_"} {
	show_mess "Name ends with '_'!\nUse another name!"
	return 1
   }
   if {[string match "*__*" $name]} {
	show_mess "Name contains '__'!\nUse another name!"
	return 1
   }
   set max_name_len 14
   if {[string length $name] > $max_name_len} {
	show_mess "Name too long (> $max_name_len)!\nUse a shorter name!"
	return 1
   }
   if {[reserved_word $name]} {
	show_mess "Name is reserved word!\nUse another name!"
	return 1
   }
   return 0
}
