
# procedure to route the connections of the circuit
# to show the picture in a nicer way. It has no influence on
# the fysical connection structure or on the generated vhdl_code
#---------------------------------------------------------------
proc do_route {} {
   global GW CellNh
   global NCell CELL_ARR
   global NConn CONN_ARR
   global NLabel NSplit NMerge N_RERR
   global LinewidthSingle LinewidthBus
   global show_occs kind

   set kind "routed"
   show_info "routing .....\n"
   .frm.cv delete router_con
   set nx [det_nx]
   set ny [det_ny]
   set x0 [det_x0]
   set y0 [det_y0]
   set N_RERR 0

   reset_arrays $x0 $y0 $nx $ny

   for {set n 0} {$n <= $NCell} {incr n} {
      set coords [.frm.cv coords $CELL_ARR($n)_bb]
      set i1 [expr round(([lindex $coords 0] - $x0)/$GW)]
      set i2 [expr round(([lindex $coords 2] - $x0)/$GW)]
      set j1 [expr round(([lindex $coords 1] - $y0)/$GW)]
      set j2 [expr round(([lindex $coords 3] - $y0)/$GW)]
      set_instance_region_occ $i1 $j1 $i2 $j2 $nx
   }
   for {set n 0} {$n <= $NSplit} {incr n} {
      set coords [.frm.cv coords S-$n.in]
      set i1 [expr round(([lindex $coords 2] - $x0)/$GW)]
      set j  [expr round(([lindex $coords 1] - $y0)/$GW)]
      set coords [.frm.cv coords S-$n.out]
      set i2 [expr round(([lindex $coords 2] - $x0)/$GW)]
      set_splitter_region_occ $i1 $j $i2 $nx
   }
   for {set n 0} {$n <= $NMerge} {incr n} {
      set coords [.frm.cv coords M-${n}_bb]
      set i1 [expr round(([lindex $coords 0] - $x0)/$GW)]
      set i2 [expr round(([lindex $coords 2] - $x0)/$GW)]
      set j1 [expr round(([lindex $coords 1] - $y0)/$GW)]
      set j2 [expr round(([lindex $coords 3] - $y0)/$GW)]
      set_merger_region_occ $i1 $j1 $i2 $j2 $nx
   }
   for {set n 0} {$n <= $NLabel} {incr n} {
      set coords [.frm.cv bbox L-$n]
      set i1 [expr round(([lindex $coords 0] - $x0)/$GW)]
      set j1 [expr round(([lindex $coords 1] - $y0)/$GW)]
      set i2 [expr round(([lindex $coords 2] - $x0)/$GW)+1]
      set j2 [expr round(([lindex $coords 3] - $y0)/$GW)]
      set_label_region_occ $i1 $j1 $i2 $j2 $nx
   }

   set net_arr(1) ""
   set nnet 0
   for {set ic 0} {$ic <= $NConn} {incr ic} {
      if {$CONN_ARR($ic) == ""} {continue}
      set con_tags [split $CONN_ARR($ic) ">"]
      set from_tag [lindex $con_tags 0]
      set to_tag [lindex $con_tags 1]
      set net_found "False"
      if {$nnet == 0} {
         incr nnet
         set net_arr($nnet) "$from_tag $to_tag"
         set ia 1
         set net_found "True"
      } else {
         for {set ia 1} {$ia <= [array size net_arr]} {incr ia} {
            if {[lsearch $net_arr($ia) $from_tag] >= 0} {
               lappend net_arr($ia) $to_tag
               set net_found "True"
               break
            }
            if {[lsearch $net_arr($ia) $to_tag] >= 0} {
               lappend net_arr($ia) $from_tag
               set net_found "True"
               break
            }
         }
      }
      if {$net_found == "False"} {
         incr nnet
         set net_arr($nnet) "$from_tag $to_tag"
         set ia $nnet
      }
      set from_coords [.frm.cv coords $to_tag]
      set to_coords [.frm.cv coords $from_tag]
      set x_from [expr round(([lindex $from_coords 2] - $x0)/$GW)]
      set y_from [expr round(([lindex $from_coords 1] - $y0)/$GW)]
      set x_to [expr round(([lindex $to_coords 2] - $x0)/$GW)]
      set y_to [expr round(([lindex $to_coords 1] - $y0)/$GW)]
      set ww [.frm.cv itemcget C-$ic -width]
      if {$ww != ""} {
	.frm.cv delete C-$ic
      } elseif {[string first "(" $CONN_ARR($ic)] < 0} {
	set ww $LinewidthSingle
      } else {
	set ww $LinewidthBus
      }
      if {[make_wire $x_to $y_to $x_from $y_from $nx $ny $x0 $y0 $ia $ww] < 0} {
	draw_con $from_tag $to_tag $ww $ic
      }
      .frm.cv raise router_con grid_line
   }
   if {$show_occs == 1} {
      show_occupied
   }
   show_info "routing DONE with $N_RERR unrouted net(s)\n"
}

# procedure to determine the number of grid_points in the x_direction
#--------------------------------------------------------------------
proc det_nx {} {
   global GW BboxId

   set bc [.frm.cv coords $BboxId]
   set nx [expr round(([lindex $bc 2] - [lindex $bc 0])/$GW)]
}

# procedure to determine the number of grid_points in the y_direction
#--------------------------------------------------------------------
proc det_ny {} {
   global GW CellNh BboxId

   set bc [.frm.cv coords $BboxId]
   set ny [expr round(([lindex $bc 3] - [lindex $bc 1])/$GW) - $CellNh]
}

# procedure to determine the first grid_point in the x_direction
#---------------------------------------------------------------
proc det_x0 {} {
   global BboxId

   set x0 [lindex [.frm.cv coords $BboxId] 0]
}

# procedure to determine the first grid_point in the y_direction
#---------------------------------------------------------------
proc det_y0 {} {
   global GW CellNh BboxId

   set y0 [expr [lindex [.frm.cv coords $BboxId] 1] + $CellNh*$GW]
}

# procedure to reset the help_arrays for the routing
#---------------------------------------------------
proc reset_arrays {x0 y0 nx ny} {
   global e_arr n_arr w_arr s_arr v_arr

   for {set i 0} {$i <= $nx} {incr i} {
      for {set j 0} {$j <= $ny} {incr j} {
         set idx [expr $j*($nx+1) + $i]
         set e_arr($idx) 0
         set w_arr($idx) 0
         if {($i == 0) || ($i == $nx)} {
            set n_arr($idx) -1
            set s_arr($idx) -1
         } else {
            set n_arr($idx) 0
            set s_arr($idx) 0
         }
         set v_arr($idx) 0
      }
   }
}

# procedure to set the help_arrays for an instance
#-------------------------------------------------
proc set_instance_region_occ {i1 j1 i2 j2 nx} {
   global e_arr n_arr w_arr s_arr

   for {set i $i1} {$i <= $i2} {incr i} {
      for {set j $j1} {$j <= $j2} {incr j} {
         set idx [expr $j*($nx+1) + $i]
         if {$i > $i1} {
            set w_arr($idx) -1
         }
         if {$i < $i2} {
            set e_arr($idx) -1
         }
         if {((($i == $i1) && ($j ==$j1) || ($j == $j2)) ||
              (($i == $i2) && ($j ==$j1) || ($j == $j2)))} {
            set w_arr($idx) -1
            set e_arr($idx) -1
         }
         set n_arr($idx) -1
         set s_arr($idx) -1
      }
   }
}

# procedure to set the help_arrays for a splitter
#------------------------------------------------
proc set_splitter_region_occ {i1 jm i2 nx} {
   global e_arr n_arr w_arr s_arr
   for {set i [expr $i1+1]} {$i < $i2} {incr i} {
      for {set j [expr $jm-1]} {$j <= [expr $jm+1]} {incr j} {
         set idx [expr $j*($nx+1) + $i]
         set w_arr($idx) -1
         set e_arr($idx) -1
         set n_arr($idx) -1
         set s_arr($idx) -1
      }
   }
   set idx [expr ($jm-1)*($nx+1) + $i1]
   set e_arr($idx) -1
   set s_arr($idx) -1
   set idx [expr $jm*($nx+1) + $i1]
   set e_arr($idx) -1
   set n_arr($idx) -1
   set s_arr($idx) -1
   set idx [expr ($jm+1)*($nx+1) + $i1]
   set e_arr($idx) -1
   set n_arr($idx) -1
   set idx [expr ($jm-1)*($nx+1) + $i2]
   set w_arr($idx) -1
   set s_arr($idx) -1
   set idx [expr $jm*($nx+1) + $i2]
   set w_arr($idx) -1
   set n_arr($idx) -1
   set s_arr($idx) -1
   set idx [expr ($jm+1)*($nx+1) + $i2]
   set w_arr($idx) -1
   set n_arr($idx) -1
}

# procedure to set the help_arrays for a merger
#-------------------------------------------------
proc set_merger_region_occ {i1 j1 i2 j2 nx} {
   global e_arr n_arr w_arr s_arr

   for {set i $i1} {$i <= $i2} {incr i} {
      for {set j $j1} {$j <= $j2} {incr j} {
         set idx [expr $j*($nx+1) + $i]
         if {$i > $i1} {
            set w_arr($idx) -1
         }
         if {$i < $i2} {
            set e_arr($idx) -1
         }
         if {((($i == $i1) && ($j ==$j1) || ($j == $j2)) ||
              (($i == $i2) && ($j ==$j1) || ($j == $j2)))} {
            set w_arr($idx) -1
            set e_arr($idx) -1
         }
         set n_arr($idx) -1
         set s_arr($idx) -1
      }
   }
}

# procedure to set the help_arrays for a label
#---------------------------------------------
proc set_label_region_occ {i1 j1 i2 j2 nx} {
   global e_arr n_arr w_arr s_arr
   for {set i $i1} {$i < $i2} {incr i} {
      for {set j $j1} {$j <= $j2} {incr j} {
         set idx [expr $j*($nx+1) + $i]
         set w_arr($idx) -1
         set e_arr($idx) -1
         set n_arr($idx) -1
         set s_arr($idx) -1
      }
   }
}

# procedure to generate a wire for a connection
#----------------------------------------------
proc make_wire {i_start j_start i_stop j_stop nx ny x0 y0 net_nbr ww} {
   global RoutCol GW
   global list_arr nf nl end_point_found v_arr
   global e_arr w_arr n_arr s_arr
   global N_RERR

   set idx [expr $j_start*($nx+1) + $i_start]
   if {($e_arr($idx) > 0) || ($w_arr($idx) > 0)} {
      set tmp $i_start
      set i_start $i_stop
      set i_stop $tmp
      set tmp $j_start
      set j_start $j_stop
      set j_stop $tmp
   }
   set list_arr(1) {}
   lappend list_arr(1) "$i_start $j_start"
   set nl 1
   set nf 1
   set end_point_found "False"
   set idx [expr $j_start*($nx+1)+$i_start]
   set v_arr($idx) 1
   set idx [expr $j_stop*($nx+1)+$i_stop]
   set e_arr($idx) $net_nbr
   set w_arr($idx) $net_nbr
   while {1} {
      set next_iteration [update_list_arr $i_stop $j_stop\
                          $nf $nl $nx $ny $net_nbr]
      if {$next_iteration == -1} {
         incr N_RERR
         for {set i 0} {$i < [expr ($nx+1)*($ny+1)]} {incr i} {
            set v_arr($i) 0
            .frm.cv itemconfigure d$i -fill wheat
         }
         return -1
      } elseif {$next_iteration == 1} {
         break
      }
   }
   for {set i 0} {$i < [expr ($nx+1)*($ny+1)]} {incr i} {
      set v_arr($i) 0
   }
   for {set i 1} {$i <= $nl} {incr i} {
      set xe [lindex [lindex $list_arr($i) end] 0]
      set ye [lindex [lindex $list_arr($i) end] 1]
      set idx [expr $ye*($nx+1)+$xe]
      if {($e_arr($idx) == $net_nbr) ||
          ($w_arr($idx) == $net_nbr) ||
          ($n_arr($idx) == $net_nbr) ||
          ($s_arr($idx) == $net_nbr)} {
         if {($xe != $i_stop) || ($ye != $j_stop)} {
            .frm.cv create oval [expr $x0+$GW*$xe]\
                                [expr $y0+$GW*$ye]\
                                [expr $x0+$GW*$xe]\
                                [expr $y0+$GW*$ye]\
	    -width [expr $ww+4] -outline $RoutCol -fill $RoutCol -tag router_con
         }
         set i_last [expr [llength $list_arr($i)] -2]
         for {set ii 0} {$ii <= $i_last} {incr ii} {
            set xa [lindex [lindex $list_arr($i) $ii] 0]
            set ya [lindex [lindex $list_arr($i) $ii] 1]
            set xb [lindex [lindex $list_arr($i) [expr $ii+1]] 0]
            set yb [lindex [lindex $list_arr($i) [expr $ii+1]] 1]
            .frm.cv create line [expr $x0+$GW*$xa] [expr $y0+$GW*$ya]\
                            [expr $x0+$GW*$xb] [expr $y0+$GW*$yb]\
		    -capstyle round -fill $RoutCol -width $ww -tag router_con
            update
            set idx_a [expr $ya*($nx+1)+$xa]
            set idx_b [expr $yb*($nx+1)+$xb]
            if {$ii == 0} {
               set e_arr($idx_a) $net_nbr
               set w_arr($idx_a) $net_nbr
               set s_arr($idx_a) $net_nbr
               set n_arr($idx_a) $net_nbr
               if {$xa == $xb} {
                  set direction "VER"
               } else {
                  set direction "HOR"
               }
            } elseif {$xa != $xb} {
               set e_arr($idx_a) $net_nbr
               set w_arr($idx_a) $net_nbr
               if {$direction == "VER"} {
                  set direction "HOR"
                  set s_arr($idx_a) $net_nbr
                  set n_arr($idx_a) $net_nbr
               }
            } elseif {$ya != $yb} {
               set s_arr($idx_a) $net_nbr
               set n_arr($idx_a) $net_nbr
               if {$direction == "HOR"} {
                  set direction "VER"
                  set e_arr($idx_a) $net_nbr
                  set w_arr($idx_a) $net_nbr
               }
            }
            if {$ii == $i_last} {
               if {$xb < $xa} {
                  set e_arr($idx_b) $net_nbr
               }
               if {$xb > $xa} {
                  set w_arr($idx_b) $net_nbr
               }
               if {$yb < $ya} {
                  set s_arr($idx_b) $net_nbr
               }
               if {$yb > $ya} {
                  set n_arr($idx_b) $net_nbr
               }
            }
         }
         break
      }
   }
   return 1
}

# procedure to see if the grid_point east of a point is free
#-----------------------------------------------------------
proc e_free {ii jj nx ny net_nbr} {
   global list_arr w_arr v_arr

   incr ii
   if {$ii > $nx} {
      return "False"
   }
   set idx [expr $jj*($nx+1)+$ii]
   if {$v_arr($idx) != 0} {
      return "False"
   }
   if {$w_arr($idx) == $net_nbr} {
      return "Found"
   }
   if {$w_arr($idx) != 0} {
      return "False"
   }
   set v_arr($idx) 1
   return "True"
}

# procedure to see if the grid_point north of a point is free
#------------------------------------------------------------
proc n_free {ii jj nx ny net_nbr} {
   global list_arr s_arr v_arr

   incr jj -1
   if {$jj < 1} {
      return "False"
   }
   set idx [expr $jj*($nx+1)+$ii]
   if {$v_arr($idx) != 0} {
      return "False"
   }
   if {$s_arr($idx) == $net_nbr} {
      return "Found"
   }
   if {$s_arr($idx) != 0} {
      return "False"
   }
   set v_arr($idx) 1
   return "True"
}

# procedure to see if the grid_point west of a point is free
#-----------------------------------------------------------
proc w_free {ii jj nx ny net_nbr} {
   global list_arr e_arr v_arr

   incr ii -1
   if {$ii < 0} {
      return "False"
   }
   set idx [expr $jj*($nx+1)+$ii]
   if {$v_arr($idx) != 0} {
      return "False"
   }
   if {$e_arr($idx) == $net_nbr} {
      return "Found"
   }
   if {$e_arr($idx) != 0} {
      return "False"
   }
   set v_arr($idx) 1
   return "True"
}

# procedure to see if the grid_point south of a point is free
#------------------------------------------------------------
proc s_free {ii jj nx ny net_nbr} {
   global list_arr n_arr v_arr

   incr jj
   if {$jj > $ny} {
      return "False"
   }
   set idx [expr $jj*($nx+1)+$ii]
   if {$v_arr($idx) != 0} {
      return "False"
   }
   if {$n_arr($idx) == $net_nbr} {
      return "Found"
   }
   if {$n_arr($idx) != 0} {
      return "False"
   }
   set v_arr($idx) 1
   return "True"
}

# procedure to update the list of possible paths that have to be examined
#------------------------------------------------------------------------
proc update_list_arr {i_stop j_stop  n_f n_itt nx ny net_nbr} {
   global list_arr end_point_found nl nf il

   set list_incr "False"
   for {set il $n_f} {$il <= $n_itt} {incr il} {
      set ii [lindex [lindex $list_arr($il) end] 0]
      set jj [lindex [lindex $list_arr($il) end] 1]
      set e_status [e_free $ii $jj $nx $ny $net_nbr]
      if {$e_status == "True"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {[expr $ii+1] $jj}"
         set list_incr "True"
      } elseif {$e_status == "Found"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {[expr $ii+1] $jj}"
         return 1
      }
      set n_status [n_free $ii $jj $nx $ny $net_nbr]
      if {$n_status == "True"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {$ii [expr $jj-1]}"
         set list_incr "True"
      } elseif {$n_status == "Found"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {$ii [expr $jj-1]}"
         return 1
      }
      set w_status [w_free $ii $jj $nx $ny $net_nbr]
      if {$w_status == "True"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {[expr $ii-1] $jj}"
         set list_incr "True"
      } elseif {$w_status == "Found"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {[expr $ii-1] $jj}"
         return 1
      }
      set s_status [s_free $ii $jj $nx $ny $net_nbr]
      if {$s_status == "True"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {$ii [expr $jj+1]}"
         set list_incr "True"
      } elseif {$s_status == "Found"} {
         incr nl
         set list_arr($nl) "$list_arr($il) {$ii [expr $jj+1]}"
         return 1
      }
   }
   set nf [expr $n_itt + 1]
   if {$list_incr == "True"} { return 0 }
   return -1
}

# procedure to remove route data and to draw connections in the origional way
#----------------------------------------------------------------------------
proc do_unroute {} {
   global CONN_ARR NConn kind

   .frm.cv delete router_con
   set kind "unrouted"
   for {set i 0} {$i <= $NConn} {incr i} {
	if {$CONN_ARR($i) == ""} {continue}
	.frm.cv delete C-$i
	set pl [split $CONN_ARR($i) ">"]
	set p1 [lindex $pl 0]
	set p2 [lindex $pl 1]
	draw_con $p1 $p2 [check_data_width $p1 $p2] $i
	#.frm.cv raise C-$i grid_line
   }
}
