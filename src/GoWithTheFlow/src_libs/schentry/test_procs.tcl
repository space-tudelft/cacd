
# procedure to show the instances of the circuit shown
#-----------------------------------------------------
proc show_instances {} {
    global CELL_ARR NCell CirName TxtFnt HdrFnt

   if {[winfo exists .inst_wdw] == 0} {
      toplevel  .inst_wdw
      label     .inst_wdw.lbl -bg gold -text "instances of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .inst_wdw.txt -width 30 -height 25 -font $TxtFnt\
			-yscrollcommand ".inst_wdw.sb set" -bg wheat
      scrollbar .inst_wdw.sb -command ".inst_wdw.txt yview" -bg wheat

      pack .inst_wdw.lbl -side top -fill x
      pack .inst_wdw.txt -side left -fill both -expand 1
      pack .inst_wdw.sb  -side right -fill y

      wm title .inst_wdw "show_instances"
   } else {
      .inst_wdw.txt configure -state normal
      .inst_wdw.txt delete 1.0 end
      .inst_wdw.lbl configure -text "instances of $CirName:"
      wm withdraw .inst_wdw
      wm deiconify .inst_wdw
   }
   for {set i 0} {$i <= $NCell} {incr i} {
      .inst_wdw.txt insert end "    $i  $CELL_ARR($i)\n"
   }
   .inst_wdw.txt configure -state disabled
   bind .inst_wdw <Key-x> { destroy .inst_wdw }
   bind .inst_wdw <Key-u> { show_instances }
}

# procedure to show the connections of the circuit shown
#-------------------------------------------------------
proc show_conns {} {
   global CONN_ARR NConn CirName TxtFnt HdrFnt

   if {[winfo exists .conn_wdw] == 0} {
      toplevel  .conn_wdw
      label     .conn_wdw.lbl -bg gold -text "connections of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .conn_wdw.txt -width 45 -height 25 -font $TxtFnt\
			-yscrollcommand ".conn_wdw.sb set" -bg wheat
      scrollbar .conn_wdw.sb  -command ".conn_wdw.txt yview" -bg wheat

      pack .conn_wdw.lbl -side top -fill x
      pack .conn_wdw.txt -side left -fill both -expand 1
      pack .conn_wdw.sb  -side right -fill y

      wm title .conn_wdw "show_connects"
   } else {
      .conn_wdw.txt configure -state normal
      .conn_wdw.txt delete 1.0 end
      .conn_wdw.lbl configure -text "connections of $CirName:"
      wm withdraw .conn_wdw
      wm deiconify .conn_wdw
   }
   for {set i 0} {$i <= $NConn} {incr i} {
      .conn_wdw.txt insert end "    $i  $CONN_ARR($i)\n"
   }
   .conn_wdw.txt configure -state disabled
   .conn_wdw.txt see end
   bind .conn_wdw <Key-x> { destroy .conn_wdw }
   bind .conn_wdw <Key-u> { show_conns }
}

# procedure to show the ports of the circuit shown
#-------------------------------------------------
proc show_ports {} {
   global PORT_ARR NPort CirName TxtFnt HdrFnt

   if {[winfo exists .port_wdw] == 0} {
      toplevel  .port_wdw
      label     .port_wdw.lbl -bg gold -text "ports of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .port_wdw.txt -width 35 -height 25 -font $TxtFnt\
			-yscrollcommand ".port_wdw.sb set" -bg wheat
      scrollbar .port_wdw.sb  -command ".port_wdw.txt yview" -bg wheat

      pack .port_wdw.lbl -side top -fill x
      pack .port_wdw.txt -side left -fill both -expand 1
      pack .port_wdw.sb  -side right -fill y

      wm title .port_wdw "show_ports"
   } else {
      .port_wdw.txt configure -state normal
      .port_wdw.txt delete 1.0 end
      .port_wdw.lbl configure -text "ports of $CirName:"
      wm withdraw .port_wdw
      wm deiconify .port_wdw
   }
   for {set i 0} {$i <= $NPort} {incr i} {
      .port_wdw.txt insert end "    $i  $PORT_ARR($i)\n"
   }
   .port_wdw.txt configure -state disabled
   bind .port_wdw <Key-x> { destroy .port_wdw }
   bind .port_wdw <Key-u> { show_ports }
}

# procedure to show the labels of the circuit shown
#--------------------------------------------------
proc show_labels {} {
   global LABEL_ARR NLabel CirName TxtFnt HdrFnt

   if {[winfo exists .label_wdw] == 0} {
      toplevel  .label_wdw
      label     .label_wdw.lbl -bg gold -text "labels of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .label_wdw.txt -width 45 -height 25 -font $TxtFnt\
			-yscrollcommand ".label_wdw.sb set" -bg wheat
      scrollbar .label_wdw.sb  -command ".label_wdw.txt yview" -bg wheat

      pack .label_wdw.lbl -side top -fill x
      pack .label_wdw.txt -side left -fill both -expand 1
      pack .label_wdw.sb  -side right -fill y

      wm title .label_wdw "show_labels"
   } else {
      .label_wdw.txt configure -state normal
      .label_wdw.txt delete 1.0 end
      .label_wdw.lbl configure -text "labels of $CirName:"
      wm withdraw .label_wdw
      wm deiconify .label_wdw
   }
   for {set i 0} {$i <= $NLabel} {incr i} {
      .label_wdw.txt insert end "    $i  $LABEL_ARR($i)\n"
   }
   .label_wdw.txt configure -state disabled
   bind .label_wdw <Key-x> { destroy .label_wdw }
   bind .label_wdw <Key-u> { show_labels }
}

# procedure to show the bus connectors of the circuit shown
#-----------------------------------------------------
proc show_busconnectors {} {
   global SPLIT_ARR NSplit CirName TxtFnt HdrFnt

   if {[winfo exists .buscon_wdw] == 0} {
      toplevel  .buscon_wdw
      label     .buscon_wdw.lbl -bg gold -text "bus connectors of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .buscon_wdw.txt -width 25 -height 25 -font $TxtFnt\
			-yscrollcommand ".buscon_wdw.sb set" -bg wheat
      scrollbar .buscon_wdw.sb  -command ".buscon_wdw.txt yview" -bg wheat

      pack .buscon_wdw.lbl -side top -fill x
      pack .buscon_wdw.txt -side left -fill both -expand 1
      pack .buscon_wdw.sb  -side right -fill y

      wm title .buscon_wdw "show_buscon"
   } else {
      .buscon_wdw.txt configure -state normal
      .buscon_wdw.txt delete 1.0 end
      .buscon_wdw.lbl configure -text "bus connectors of $CirName:"
      wm withdraw .buscon_wdw
      wm deiconify .buscon_wdw
   }
   for {set i 0} {$i <= $NSplit} {incr i} {
      .buscon_wdw.txt insert end "    $i  $SPLIT_ARR($i)\n"
   }
   .buscon_wdw.txt configure -state disabled
   bind .buscon_wdw <Key-x> { destroy .buscon_wdw }
   bind .buscon_wdw <Key-u> { show_buscon }
}

# procedure to show the bus expanders of the circuit shown
#-----------------------------------------------------
proc show_busexpanders {} {
   global MERGE_ARR NMerge CirName TxtFnt HdrFnt

   if {[winfo exists .busexp_wdw] == 0} {
      toplevel  .busexp_wdw
      label     .busexp_wdw.lbl -bg gold -text "bus expanders of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .busexp_wdw.txt -width 25 -height 25 -font $TxtFnt\
			-yscrollcommand ".busexp_wdw.sb set" -bg wheat
      scrollbar .busexp_wdw.sb  -command ".busexp_wdw.txt yview" -bg wheat

      pack .busexp_wdw.lbl -side top -fill x
      pack .busexp_wdw.txt -side left -fill both -expand 1
      pack .busexp_wdw.sb  -side right -fill y

      wm title .busexp_wdw "show_busexpanders"
   } else {
      .busexp_wdw.txt configure -state normal
      .busexp_wdw.txt delete 1.0 end
      .busexp_wdw.lbl configure -text "bus expanders of $CirName:"
      wm withdraw .busexp_wdw
      wm deiconify .busexp_wdw
   }
   for {set i 0} {$i <= $NMerge} {incr i} {
      .busexp_wdw.txt insert end "    $i  $MERGE_ARR($i)\n"
   }
   .busexp_wdw.txt configure -state disabled
   bind .busexp_wdw <Key-x> { destroy .busexp_wdw }
   bind .busexp_wdw <Key-u> { show_busexpanders }
}

# procedure to show the tags of the circuit shown
#------------------------------------------------
proc show_tags {} {
   global CirName TxtFnt HdrFnt

   if {[winfo exists .tag_wdw] == 0} {
      toplevel  .tag_wdw
      label     .tag_wdw.lbl -bg gold -text "tags of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .tag_wdw.txt -width 45 -height 25 -font $TxtFnt\
			-yscrollcommand ".tag_wdw.sb set" -bg wheat
      scrollbar .tag_wdw.sb  -command ".tag_wdw.txt yview" -bg wheat

      pack .tag_wdw.lbl -side top -fill x
      pack .tag_wdw.txt -side left -fill both -expand 1
      pack .tag_wdw.sb  -side right -fill y

      wm title .tag_wdw "show_tags"
   } else {
      .tag_wdw.txt configure -state normal
      .tag_wdw.txt delete 1.0 end
      .tag_wdw.lbl configure -text "tags of $CirName:"
      wm withdraw .tag_wdw
      wm deiconify .tag_wdw
   }
   set tgs [.frm.cv find all]
   foreach i $tgs {
      .tag_wdw.txt insert end "    $i  [.frm.cv gettags $i]\n"
   }
   .tag_wdw.txt configure -state disabled
   bind .tag_wdw <Key-x> { destroy .tag_wdw }
}

# procedure to show the referencess in the circuit shown
#-------------------------------------------------------
proc show_references {net_list ref_list} {
   global CirName TxtFnt HdrFnt

   if {[winfo exists .ref_wdw] == 0} {
      toplevel  .ref_wdw
      label     .ref_wdw.lbl -bg gold -text "net_references of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .ref_wdw.txt -width 50 -height 25 -font $TxtFnt -bg wheat\
			-yscrollcommand ".ref_wdw.sb set"
      scrollbar .ref_wdw.sb  -command ".ref_wdw.txt yview" -bg wheat

      pack .ref_wdw.lbl -side top -fill x
      pack .ref_wdw.txt -side left -fill both -expand 1
      pack .ref_wdw.sb  -side right -fill y

      wm title .ref_wdw "show_references"
   } else {
      .ref_wdw.txt configure -state normal
      .ref_wdw.txt delete 1.0 end
      .ref_wdw.lbl configure -text "net_references of $CirName:"
      wm withdraw .ref_wdw
      wm deiconify .ref_wdw
   }
   for {set i 0} {$i < [llength $ref_list]} {incr i} {
      set show_string [format "  %4d  %-25s %s" $i\
                      [lindex $net_list $i] [lindex $ref_list $i]]
      .ref_wdw.txt insert end "$show_string\n"
   }
   .ref_wdw.txt configure -state disabled
   bind .ref_wdw <Key-x> { destroy .ref_wdw }
}

# procedure to show the occupation status in the circuit shown
#-------------------------------------------------------------
proc show_occupied {} {
   global e_arr w_arr n_arr s_arr
   global CirName TxtFnt HdrFnt

   set nx [det_nx]
   set ny [det_ny]
   set x0 [det_x0]
   set y0 [det_y0]
   if {[winfo exists .occ_wdw] == 0} {
      toplevel  .occ_wdw
      label     .occ_wdw.lbl -bg gold -text "grid occupation of $CirName:"\
			-bd 4 -relief ridge -pady 5 -font $HdrFnt
      label     .occ_wdw.lbt -bg gold\
			-text "     x     y  e_arr  w_arr  n_arr  s_arr"\
			-anchor w -bd 4 -relief ridge -pady 5 -font $HdrFnt
      text      .occ_wdw.txt -width 50 -height 25 -font $TxtFnt\
			-yscrollcommand ".occ_wdw.sb set" -bg wheat
      scrollbar .occ_wdw.sb -command ".occ_wdw.txt yview" -bg wheat

      pack .occ_wdw.lbl -side top -fill x
      pack .occ_wdw.lbt -side top -fill x
      pack .occ_wdw.txt -side left -fill both -expand 1
      pack .occ_wdw.sb  -side right -fill y

      wm title .occ_wdw "show_occupied"
   } else {
      .occ_wdw.txt configure -state normal
      .occ_wdw.txt delete 1.0 end
      .occ_wdw.lbl configure -text "grid occupation of $CirName:"
      wm withdraw .occ_wdw
      wm deiconify .occ_wdw
   }
   for {set ix 0} {$ix <= $nx} {incr ix} {
      for {set jy 0} {$jy <= $ny} {incr jy} {
         set idx [expr $jy*($nx+1)+$ix]
         set str [format " %5d %5d  %5d  %5d  %5d  %5d" $ix $jy $e_arr($idx)\
                           $w_arr($idx) $n_arr($idx) $n_arr($idx)]
         .occ_wdw.txt insert end "$str\n"
      }
   }
   .occ_wdw.txt configure -state disabled
   bind .occ_wdw <Key-x> { destroy .occ_wdw }
}
