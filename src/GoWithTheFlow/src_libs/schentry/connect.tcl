
# procedure to see if two valid tags are present for the connection
#------------------------------------------------------------------
proc do_connect {x0 y0 nr idx} {
   global SelTag CirName CONN_ARR NConn MN_INPUT OUTPUT INOUT

   if {[set to_tag [get_port_tag $x0 $y0]] == ""} {
	show_mess "Connection must end on a port!"
	return 0
   }

   if {$nr < 0} {
	set from_tag $SelTag
   } else {
	set pl [split $CONN_ARR($nr) ">"]
	set from_tag [lindex $pl $idx]
	set i 0
	if {$i == $idx} { incr i }
	set dw [get_data_width [lindex $pl $i]]
   }

   if {$to_tag == $from_tag} {
#	show_mess "You cannot connect a port to itself!"
	return 0
   }
   if {[has_label $to_tag] >= 0} {
      show_mess "Connections to ports which have a label are forbidden!"
      return 0
   }

   set ftype [get_porttype $from_tag]
   set ttype [get_porttype $to_tag]

   if {$ftype == $INOUT || $ttype == $INOUT} {
	if {$ftype != $ttype} {
	    show_mess "Both ports must be of type bidirectional!"
	    return 0
	}
	set conn_width [check_data_width $from_tag $to_tag]
	if {$conn_width <= 0} { return 0 }

	if {[set i [is_connected $from_tag]] >= 0 && $idx == 0} {
	    set from_tag [lindex [split $CONN_ARR($i) ">"] 0]
	    if {[set i [is_connected $to_tag]] >= 0} {
		set to_tag [lindex [split $CONN_ARR($i) ">"] 0]
		if {$from_tag == $to_tag} {
		    show_mess "Both ports are already connected!"
		    return 0
		}
		for {set i 0} {$i <= $NConn} {incr i} {
		    set plist [split $CONN_ARR($i) >]
		    if {[lindex $plist 0] == $to_tag} {
			.frm.cv delete C-$i
			set CONN_ARR($i) "$from_tag>[lindex $plist 1]"
			draw_con $from_tag [lindex $plist 1] $conn_width $i
		    }
		}
	    }
	} elseif {[set i [is_connected $to_tag]] >= 0} {
	    set to_tag $from_tag
	    set from_tag [lindex [split $CONN_ARR($i) ">"] 0]
	} elseif {$idx == 1} {
	    set tmp $from_tag
	    set from_tag $to_tag
	    set to_tag $tmp
	}

	if {$nr < 0} {
	    connect $from_tag $to_tag $conn_width 1
	} else {
	    if {$dw != 1000 && $dw != $conn_width} {
		show_mess "Connection cannot be made:\
		    data width of the ports is not the same!"
		return 0
	    }
	    set CONN_ARR($nr) $from_tag>$to_tag
	}
	return 1
   }

   set fdriv 0
   set tdriv 0
   if {$ftype == $MN_INPUT || $ftype == $OUTPUT} { incr fdriv }
   if {$ttype == $MN_INPUT || $ttype == $OUTPUT} { incr tdriv }
   if {$fdriv == $tdriv} {
	if {$fdriv} {
	    show_mess "You cannot connect two driver ports!"
	    return 0
	} elseif {$nr >= 0} {
	    show_mess "You must connect to a driver port!"
	    return 0
	}
	set i [is_inout_receiver $from_tag]
	set j [is_inout_receiver $to_tag]
	if {$i >= 0 && $j >= 0} {
	    show_mess "You cannot connect from two driver ports!"
	    return 0
	}
	if {$i < 0} {
	  if {$j < 0} {
	    show_mess "You must first make a connection with a driver port!"
	    return 0
	  }
	  set to_tag $from_tag
	  set i $j
	}
	set from_tag [lindex [split $CONN_ARR($i) ">"] 0]
   } elseif {$tdriv} {
	if {[is_inout_receiver $from_tag] >= 0} {
	  if {$nr < 0 || $idx == 0} {
	    show_mess "You cannot connect two driver ports!"
	    return 0
	  }
	}
	set tmp $from_tag
	set from_tag $to_tag
	set to_tag $tmp
   } else {
	if {[is_inout_receiver $to_tag] >= 0} {
	    show_mess "You cannot connect two driver ports!"
	    return 0
	}
   }

   set comp [lindex [split $from_tag "."] 0]
   if {$comp != $CirName && [lindex [split $to_tag "."] 0] == $comp} {
      show_mess "You cannot connect ports of same component!"
      return 0
   }

   set conn_width [check_data_width $from_tag $to_tag]
   if {$conn_width < 1} { return 0 }

   if {$nr < 0} {
	connect $from_tag $to_tag $conn_width 1
   } else {
	if {$dw != 1000 && $dw != $conn_width} {
	    show_mess "Connection cannot be made:\
		data width of the ports is not the same!"
	    return 0
	}
	set CONN_ARR($nr) $from_tag>$to_tag
   }
   return 1
}

# procedure to check the widths of the ports of the connection
# it returns the width of the connection if check is ok; -1 otherwise
#--------------------------------------------------------------------
proc check_data_width {from_tag to_tag} {
   set data_width_from [get_data_width $from_tag]
   set data_width_to [get_data_width $to_tag]
   if {$data_width_from < 0} { return -1 }
   if {$data_width_to   < 0} { return -1 }
   if {($data_width_from != 1000) &&
       ($data_width_to != 1000) &&
       ($data_width_from != $data_width_to)} {
      show_mess "Connection cannot be made:\
		data width of the ports is not the same!"
      return -1
   }
   if {$data_width_from == 1000} {
      set min_width [get_min_width [lindex [split $from_tag "."] 0]]
      if {$data_width_to < $min_width} {
         show_mess "Split is out of range!"
         return -1
      }
   }
   if {$data_width_to == 1000} {
      set min_width [get_min_width [lindex [split $to_tag "."] 0]]
      if {$data_width_from < $min_width} {
         show_mess "Split is out of range!"
         return -1
      }
   }
   set conn_width $data_width_from
   return $conn_width
}

# procedure to actually make the connection
#------------------------------------------
proc connect {from_tag to_tag conn_width draw_it} {
   global NConn CONN_ARR

   incr NConn
   set CONN_ARR($NConn) $from_tag>$to_tag
   if {$draw_it == 1} {
      draw_con $from_tag $to_tag $conn_width $NConn
   }
 # .frm.cv raise C-$NConn grid_line
}

# procedure to draw a connection
#-------------------------------
proc draw_con {from_tag to_tag conn_width nr} {
   global ConnCol LinewidthSingle LinewidthBus

   set coords_1 [get_con_coords $from_tag]
   set coords_2 [get_con_coords $to_tag]
   if {$conn_width == 1} {
	.frm.cv create line [lindex $coords_1 0] [lindex $coords_1 1]\
		[lindex $coords_2 0] [lindex $coords_2 1]\
		-width $LinewidthSingle -fill $ConnCol -tag C-$nr
   } else {
	.frm.cv create line [lindex $coords_1 0] [lindex $coords_1 1]\
		[lindex $coords_2 0] [lindex $coords_2 1]\
		-width $LinewidthBus -fill $ConnCol -tag C-$nr
   }
}

# procedure to update the connection coordinates when its port has moved
#-----------------------------------------------------------------------
proc update_conn {nr} {
   global CONN_ARR
   set p [split $CONN_ARR($nr) ">"]
   set a [get_con_coords [lindex $p 0]]
   set b [get_con_coords [lindex $p 1]]
   .frm.cv coords C-$nr [lindex $a 0] [lindex $a 1] [lindex $b 0] [lindex $b 1]
}

proc change_in_conn {from_tag idx} {
   global LABEL_ARR NLabel INOUT

   set answ [tk_messageBox -icon warning -type yesno\
	-message "Do you want to change labels into wire connections!\nAre you sure?"]
   if {$answ != "yes"} { return }

   set label_name [lindex $LABEL_ARR($idx) 0]
   set conn_width [get_data_width $label_name]

   if {![is_driver $from_tag]} {
     for {set i 0} {$i <= $NLabel} {incr i} {
	if {$i == $idx} {continue}
	if {[lindex $LABEL_ARR($i) 0] == $label_name} {
	   if {[is_driver [lindex $LABEL_ARR($i) 1]]} {
		set from_tag [lindex $LABEL_ARR($i) 1]
		set idx $i
		break
	   }
	}
     }
   }
   for {set i 0} {$i <= $NLabel} {incr i} {
     if {[lindex $LABEL_ARR($i) 0] == $label_name} {
	if {$i != $idx} { connect $from_tag [lindex $LABEL_ARR($i) 1] $conn_width 1 }
	.frm.cv delete L-$i
	set LABEL_ARR($i) ""
     }
   }
}
