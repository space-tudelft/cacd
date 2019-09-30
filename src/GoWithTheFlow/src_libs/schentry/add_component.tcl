
# procedure to generate the components directory, if it does not yet exist
# and fill it with the data from the basic components directory
#--------------------------------------------------------------
proc gen_comp_dir {} {
   global ComponentPath CompLibPath

   if {![file exists $ComponentPath]} {
      file mkdir $ComponentPath
      foreach i [glob $CompLibPath/*] {
         file copy $i $ComponentPath
      }
   }
}

# procedure to add a component to the circuit
#--------------------------------------------
proc do_add_component {x y comp_name nr} {
   global CompW CompH CompIn CompOut GW
   global NCell CELL_ARR
   global CellFnt PortFnt
   global InpCol OutpCol InoutCol

   set TD [expr $GW/2.]
   if {$nr < 0} { set nr [incr NCell] }
   set tag_all $comp_name~$nr

# outline
   .frm.cv create rectangle $x $y [expr $x+$CompW] [expr $y+$CompH]\
		-fill wheat -tag "$tag_all ${tag_all}_bb"
   .frm.cv create rectangle $x $y [expr $x+$CompW] [expr $y+2*$GW]\
		-fill gold -tag "$tag_all ${tag_all}_cc"
   .frm.cv create text [expr $x+$CompW/2.] [expr $y+$GW] -font $CellFnt\
		-text $comp_name -tag "$tag_all ${tag_all}_tt"

# input_ports
   set yt [expr $y+$GW]
   foreach port $CompIn {
	set yt [expr $yt+2*$GW]
	set fill $InpCol
	if {[string index $port 0] == "*"} {
	    set fill $InoutCol
	    set port [string trimleft $port *]
	}
	.frm.cv create polygon [expr $x-$TD] $yt\
		$x [expr $yt-$TD] [expr $x+$TD] $yt $x [expr $yt+$TD]\
		-fill $fill -outline black -tag "$tag_all $tag_all.$port"
	.frm.cv create text [expr $x+$GW] $yt -anchor w\
		-font $PortFnt -text $port -tag "$tag_all $tag_all.$port"
   }

# output_ports
   set x [expr $x+$CompW]
   set yt [expr $y+$GW]
   foreach port $CompOut {
	set yt [expr $yt+2*$GW]
	set fill $OutpCol
	if {[string index $port 0] == "*"} {
	    set fill $InoutCol
	    set port [string trimleft $port *]
	}
	.frm.cv create polygon [expr $x-$TD] $yt\
		$x [expr $yt-$TD] [expr $x+$TD] $yt $x [expr $yt+$TD]\
		-fill $fill -outline black -tag "$tag_all $tag_all.$port"
	.frm.cv create text [expr $x-$GW] $yt -anchor e\
		-font $PortFnt -text $port -tag "$tag_all $tag_all.$port"
   }

# add to cell_list
   set CELL_ARR($nr) $tag_all
}

# procedure to read the component definition from the component.cmp file
#-----------------------------------------------------------------------
proc read_compdef {comp_name} {
   global CompW CompH CompIn CompOut GW ComponentPath

   if {$comp_name == ""} {
      show_mess "No component_name given"
      return
   }
   set CompH [set CompW [expr 2*$GW]]
   set CompIn ""
   set CompOut ""
   if {![file exists $ComponentPath/$comp_name.cmp]} { 
      set answ [tk_messageBox -icon error -type ok -message "No component for $comp_name"]
      return
   }
   set ftmp [open $ComponentPath/$comp_name.cmp "r"]
   while {![eof $ftmp]} {
      set tt [gets $ftmp]
      if {[lindex $tt 0] == "bbox"} {
	set CompW [expr [lindex $tt 1]*$GW]
	set CompH [expr [lindex $tt 2]*$GW]
      } elseif {[lindex $tt 0] == "port"} {
	if {[lindex $tt 1] == "in"} {
	  lappend CompIn [lindex $tt 2]
	} elseif {[lindex $tt 1] == "*in"} {
	  lappend CompIn "*[lindex $tt 2]"
	} elseif {[lindex $tt 1] == "out"} {
	  lappend CompOut [lindex $tt 2]
	} else {
	  lappend CompOut "*[lindex $tt 2]"
	}
      }
   }
   close $ftmp
}

proc resize_component {inst dx dy} {
   global CompW CompH ComponentPath GW NCell CELL_ARR

   set Name [lindex [split $inst "~"] 0]
   if {![file exists $ComponentPath/$Name.cmp]} { return }

   set compP ""
   set Po ""
   set Ni 0
   set No 0
   set fp [open $ComponentPath/$Name.cmp "r"]
   while {![eof $fp]} {
      set tt [gets $fp]
      if {[lindex $tt 0] == "port"} {
	if {[string trimleft [lindex $tt 1] *] == "in"} {
	    incr Ni
	} else {
	    incr No
	    lappend Po [lindex $tt 2]
	}
	lappend compP $tt
      }
   }
   close $fp

   if {$No > $Ni} { set Ni $No }
   set compW 2
   set compH [expr 2*$Ni+2]
   while {[expr $compW*$GW+0.1] < $dx} { incr compW }
   while {[expr $compH*$GW+0.1] < $dy} { incr compH }
   set CompW [expr $compW*$GW]
   set CompH [expr $compH*$GW]

   set fp [open $ComponentPath/$Name.cmp "w"]
   puts $fp "bbox $compW $compH"
   foreach p $compP { puts $fp "$p" }
   close $fp
   show_info "File '$ComponentPath/$Name.cmp' (re)written\n"

   for {set i 0} {$i <= $NCell} {incr i} {
	if {[string first $Name~ $CELL_ARR($i)] == 0} { resize_cell $CELL_ARR($i) $Po }
   }
}

proc resize_cell {inst Po} {
   global CompW CompH CONN_ARR NConn LABEL_ARR NLabel GW

   set c [.frm.cv coord ${inst}_bb]
   set x [lindex $c 0]
   set y [lindex $c 1]
   .frm.cv coord ${inst}_bb $x $y [expr $x+$CompW] [expr $y+$CompH]
   .frm.cv coord ${inst}_cc $x $y [expr $x+$CompW] [expr $y+2*$GW]
   set t [.frm.cv coord ${inst}_tt]
   set dx [expr $x+$CompW/2.-[lindex $t 0]]
   .frm.cv move ${inst}_tt $dx 0

   set dx [expr $x+$CompW-[lindex $c 2]]
   foreach p $Po {
	set port $inst.$p
	.frm.cv move $port $dx 0
	for {set i 0} {$i <= $NLabel} {incr i} {
	    if {[lindex $LABEL_ARR($i) 1] == $port} {
		.frm.cv move L-$i $dx 0
		break
	    }
	}
	if {$i > $NLabel} {
	    for {set i 0} {$i <= $NConn} {incr i} {
		if {[string first $port $CONN_ARR($i)] >= 0} { update_conn $i }
	    }
	}
   }
}
