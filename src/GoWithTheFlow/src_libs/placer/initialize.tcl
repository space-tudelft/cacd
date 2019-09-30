
proc set_mc_data {} {
   global mc_data_arr
#
# The given numbers are:
# number of empty columns needed before the cell
# number of empty columns needed after the cell
# number of free rows in the cell
# the width of the cell in nbr of rows
# a list of the terminals and their position in rows
#
   set mc_data_arr(iv110)    "0 1 5 2 {A 0 Y 2}"
   set mc_data_arr(no210)    "0 0 5 3 {A 0 B 1 Y 2}"
   set mc_data_arr(no310)    "0 0 5 4 {A 0 B 1 C 2 Y 3}"
   set mc_data_arr(na210)    "0 0 5 3 {A 0 B 1 Y 2}"
   set mc_data_arr(na310)    "0 0 5 4 {A 0 B 1 C 2 Y 3}"
   set mc_data_arr(ex210)    "0 0 5 7 {A -1 B 4 Y 2}"
   set mc_data_arr(buf40)    "1 0 5 6 {A -1 Y 5}"
   set mc_data_arr(tbuf10)   "0 0 3 7 {A 0 E 2 Y 6}"
   set mc_data_arr(tinv10)   "0 0 4 5 {A 3 E 0 Y 4}"
   set mc_data_arr(mu111)    "1 1 4 7 {S -1 B 2 A 4 Y 7}"
   set mc_data_arr(mu210)    "1 0 2 16 {S1 -1 S2 10 D 2 C 3 B 7 A 6 Y 14}"
   set mc_data_arr(de211)    "0 0 2 12 {B 0 Y0 3 Y1 5 A 6 Y3 9 Y2 11}"
   set mc_data_arr(dfn10)    "0 0 3 17 {CK 0 D 4 Q 16}"
   set mc_data_arr(dfr11)    "1 0 3 21 {D -1 R 2 CK 4 Q 20}"
   set mc_data_arr(dfa11)    "0 0 2 19 {D 5 R 0 CK 1 Q 18}"
   set mc_data_arr(osc10)    "1 0 3 16 {XI -1 E 0 XO 10 F 15}"
   set mc_data_arr(ln3x3)    "1 0 7 10 {g -1 s 0 d 9}"
   set mc_data_arr(lp3x3)    "1 0 7 10 {g -1 d 1 s 9}"
   set mc_data_arr(mir_nin)  "0 0 6 19 {in 0 g 3}"
   set mc_data_arr(mir_nout) "0 0 7 19 {in 0 out 1 g 17}"
   set mc_data_arr(mir_pin)  "0 0 6 19 {in 0 g 17}"
   set mc_data_arr(mir_pout) "0 0 7 19 {in 0 out 1 g 17}"
}

proc com_init_cell {} {
   global CellName

   init_cell $CellName
}

proc init_cell {cell} {
   global mc_arr conn_arr CellName scale mc_data_arr row_arr row_nbrs Dx Dx0
   global srt_arr net_arr nbr_rows y_arr y0 MessText debugtxt distanceNotUniform

   if {$debugtxt} {
      .debtxt delete 1.0 end
   }

   set MessText "Making an initial placement for cell $cell ......\n\
                 number of rows: $nbr_rows\n\
                 horizontal distance between the cells: $Dx"

   update
   reset
   set row_nbrs "0 2 3 5 6 8 9 11 12 14 15 17 18 20 21 22 23 25 26 28 29 31 32 34 35 37 38 40 41 43 44 46 47 49 50 52 53 55 56 58 59"

   set y1 [expr 11]
   set y2 [expr 517]
   for {set i [expr 0]} {$i <= 66} {incr i} {
      set y_arr($i) "$y1 $y2"
      set y1 [expr $y1 + 494]
      set y2 [expr $y2 + 494]
   }

   foreach i $row_nbrs {
      set row_arr($i) [expr 0]
   }
   set CellName $cell
   set err_nbr [expr 0]

   # Read connections (nets)
   set fp [open  "|dbcat -c -s net $CellName" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} {
         continue
      }\
      elseif {[string index $txt 0] == "="} {
         continue
      }\
      elseif {[string range $txt 0 3] == "net:"} {
         set net [string trim [lindex [split [lindex $txt 0] ":"] 1] "\""]
         if {($net != "vdd") && ($net != "vss")} {
            set net_arr($net) ""
         }
      }\
      else {
         set cell [string trim [lindex [split [lindex $txt 1] ":"] 1] "\""]
         if {$cell != ""} {
            set term [string trim [lindex [split [lindex $txt 0] ":"] 1] "\""]
            if {($net != "vdd") && ($net != "vss")} {
               lappend net_arr($net) "$cell $term"
            }
         }
      }
   }

   set srt_arr(0) ""
   set i_last [expr 0]
   set idx [array startsearch net_arr]
   while {[array anymore net_arr $idx] == 1} {
      set net [array nextelement net_arr $idx]
      if {$debugtxt} {
          .debtxt insert end "NET: $net [llength $net_arr($net)]\n"
      }
      if {[llength $net_arr($net)] == 2} {
         set c_from [lindex [lindex $net_arr($net) 0] 0]
         set t_from [lindex [lindex $net_arr($net) 0] 1]
         set c_to [lindex [lindex $net_arr($net) 1] 0]
         set t_to [lindex [lindex $net_arr($net) 1] 1]
         set idx_from [expr -1]
         set idx_to   [expr -1]
         for {set ii [expr 0]} {$ii < $i_last} {incr ii} {
            set idx_from [lsearch $srt_arr($ii) $c_from]
            if {$idx_from >= 0} {
               set srt_f $ii
               break
            }
         }
         for {set ii [expr 0]} {$ii < $i_last} {incr ii} {
            set idx_to   [lsearch $srt_arr($ii) $c_to]
            if {$idx_to >= 0} {
               set srt_t $ii
               break
            }
         }
         if {($idx_from < 0) && ($idx_to < 0)} {
            set t0 [string index $t_from 0]
            if {($t0 == "Y") || ($t0 == "Q")} {
               set srt_arr($i_last) "$c_from $c_to"
            }\
            else {
               set srt_arr($i_last) "$c_to $c_from"
            }
            incr i_last
         }\
         elseif {($idx_from >= 0) && ($idx_to < 0)} {
            set t0 [string index $t_to 0]
            if {($t0 == "Y") || ($t0 == "Q")} {
               set srt_arr($srt_f) [linsert $srt_arr($srt_f) 0 $c_to]
            }\
            else {
               set srt_arr($srt_f) [linsert $srt_arr($srt_f) end $c_to]
            }
         }\
         elseif {($idx_from < 0) && ($idx_to >= 0)} {
            set t0 [string index $t_from 0]
            if {($t0 == "Y") || ($t0 == "Q")} {
               set srt_arr($srt_t) [linsert $srt_arr($srt_t) 0 $c_from]
            }\
            else {
               set srt_arr($srt_t) [linsert $srt_arr($srt_t) end $c_from]
            }
         }\
         elseif {$srt_f != $srt_t} {
            set f_before $idx_from
            set f_after [expr [llength $srt_arr($srt_f)] - $idx_from - 1]
            set t_before $idx_to
            set t_after [expr [llength $srt_arr($srt_t)] - $idx_to - 1]
            if {[expr $f_after + $t_before] < [expr $t_after + $f_before]} {
               set srt_arr($srt_f) [concat $srt_arr($srt_f) $srt_arr($srt_t)]
               set srt_arr($srt_t) ""
            }\
            else {
               set srt_arr($srt_t) [concat $srt_arr($srt_t) $srt_arr($srt_f)]
               set srt_arr($srt_f) ""
            }
         }
      }
   }
   array donesearch net_arr $idx
   catch {close $fp}
   if {$debugtxt} {
      for {set i [expr 0]} {$i < $i_last} {incr i} {
         .debtxt insert end "SRT $i: $srt_arr($i)\n"
      }
   }

   # Read cells
   set fp [open "|dbcat -c -s mc $CellName" r]
   set mc_list ""
   set to_place_list ""
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} {
         continue
      }
      if {[string index $txt 0] == "="} {
         continue
      }
      set bc [string trim [string range [lindex $txt 0] 5 end] "\""]
      set mc [string trim [string range [lindex $txt 1] 5 end] "\""]
      set mc_arr($mc) "0 0 0 0 0 $bc"
      lappend mc_list $mc
   }
   catch {close $fp}

   foreach c $mc_list {
      set found "False"
      for {set i [expr 0]} {$i < $i_last} {incr i} {
         if {[lsearch $srt_arr($i) $c] >= 0} {
            set found "True"
            break
         }
      }
      if {$found == "False"} {
         lappend to_place_list $c
      }
   }
   if {$debugtxt} {
      .debtxt insert end "MC: $mc_list\n"
      .debtxt insert end "TO PLACE: $to_place_list\n"
   }

   # First place the cells in one (long) sequence.

   foreach c $to_place_list {
      set MessText "placing $c"
      update
      set net_list [get_nets $c]
      set con_list [get_cons $net_list]
      add2srt_arr $c $con_list
   }

   # Second all cells are placed on the two lowest rows: rows with index 0 and 2

   # count the cells
   set nr_cells [expr 0]
   set idx [array startsearch srt_arr]
   while {[array anymore srt_arr $idx] == 1} {
      set c_list $srt_arr([array nextelement srt_arr $idx])
      if {$c_list == ""} {
         continue
      }
      for {set i [expr 0]} {$i < [llength $c_list]} {incr i} {
         incr nr_cells
      }
   }
   array donesearch srt_arr $idx

   set nxt_place_0 [expr 0]
   set nxt_place_1 [expr 0]
   set nr [expr 0]
   set idx [array startsearch srt_arr]
   while {[array anymore srt_arr $idx] == 1} {
      set c_list $srt_arr([array nextelement srt_arr $idx])
      if {$c_list == ""} {
         continue
      }
      for {set i [expr 0]} {$i < [llength $c_list]} {incr i} {
         set ic [lindex $c_list $i]
         set bc [lindex $mc_arr($ic) 5]
         if {[info exists mc_data_arr($bc)] == 0} {
            set MessText "Row placement can NOT be done: component $bc is not a standard cell!"
            update
            return 
         }
         if {$distanceNotUniform} {
            if {$nr < [expr $nr_cells / 2]} {
               set r [expr (2.0 * $nr)/$nr_cells]
            }\
            else {
               set r [expr (2.0 * ($nr_cells - $nr))/$nr_cells]
            }
            set r [expr [MAX $r 0.2]]
            set d [expr int(2 * $Dx * $r)]
         }\
         else {
            set d $Dx
         }
         if {$nxt_place_0 <= $nxt_place_1} {
            set xl [expr ($nxt_place_0+$d+\
                               [lindex $mc_data_arr($bc) 0])*40 + 34]
            set xr [expr $xl + 40*[lindex $mc_data_arr($bc) 3]]
            set yt [lindex $y_arr(0) 0]
            set yb [lindex $y_arr(0) 1]
            set mc_arr($ic) [lreplace $mc_arr($ic) 0 3 $xl $yt $xr $yb]
            set nxt_place_0 [expr $nxt_place_0 + $d +\
                             [lindex $mc_data_arr($bc) 0] +\
                             [lindex $mc_data_arr($bc) 1] +\
                             [lindex $mc_data_arr($bc) 3]]
         }\
         else {
            set xl [expr ($nxt_place_1+$d+\
                               [lindex $mc_data_arr($bc) 0])*40 + 34]
            set xr [expr $xl + 40*[lindex $mc_data_arr($bc) 3]]
            set yt [lindex $y_arr(2) 0]
            set yb [lindex $y_arr(2) 1]
            set mc_arr($ic) [lreplace $mc_arr($ic) 0 3 $xl $yt $xr $yb]
            set nxt_place_1 [expr $nxt_place_1 + $d +\
                             [lindex $mc_data_arr($bc) 0] +\
                             [lindex $mc_data_arr($bc) 1] +\
                             [lindex $mc_data_arr($bc) 3]]
         }
         if {$debugtxt} {
            .debtxt insert end "STEP 2: $bc $ic $i $d ($xl $xr $yb $yt)\n"
         }
         incr nr
      }
   }
   array donesearch srt_arr $idx

   # Now we are going to use the other rows as well.

   set tmp_list ""
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set ci [array nextelement mc_arr $idx]
      lappend tmp_list "$ci [lindex $mc_arr($ci) 0]"
   }
   array donesearch mc_arr $idx
   set tmp_list [lsort -command comp_list $tmp_list]

   # len will be the width of our layout
   set nxt_place_max [MAX $nxt_place_0 $nxt_place_1]
   if {$nbr_rows > 2} {
      set l [expr int(2 * $nxt_place_max / $nbr_rows)]
      if {[expr $l * $nbr_rows] < [expr 2 * $nxt_place_max]} { incr l }
      set len [expr $l*40]
   }\
   else {
      set len [expr $nxt_place_max*40]
   }

   for {set it [expr 0]} {$it < [llength $tmp_list]} {incr it} {
      set ci [lindex [lindex $tmp_list $it] 0]
      # nl denotes the row pair index for the cell (i.e. $nl * 2 <= $nbr_rows)
      set nl [expr int([lindex [lindex $tmp_list $it] 1]/$len)]
      if {$nl > [expr $nbr_rows/2 - 1]} {
         # This may lead to a position left of y-axis
         if {$debugtxt} {
            .debtxt insert end "ERROR: $nl > [expr $nbr_rows/2 - 1]\n"
         }
         set nl [expr $nbr_rows/2 - 1]
      }
      if {[expr $nl % 2] == 0} {
         # For an even row pair index we place from left to right
         set xl [expr [lindex $mc_arr($ci) 0] - $nl*$len]
         set xr [expr [lindex $mc_arr($ci) 2] - $nl*$len]
      }\
      else {
         # For an odd row pair index we place from right to left 
         set xr [expr ($nl+1)*$len - [lindex $mc_arr($ci) 0] +34]
         set xl [expr ($nl+1)*$len - [lindex $mc_arr($ci) 2] +34]
      }
      if {[lindex $mc_arr($ci) 1] == [lindex $y_arr(0) 0]} {
         set yt [lindex $y_arr([expr 3*$nl]) 0]
         set yb [lindex $y_arr([expr 3*$nl]) 1]
      }\
      else {
         set yt [lindex $y_arr([expr 2+3*$nl]) 0]
         set yb [lindex $y_arr([expr 2+3*$nl]) 1]
      }
      set mc_arr($ci) [lreplace $mc_arr($ci) 0 3 $xl $yt $xr $yb]
   }
   for {set r_nbr [expr 0]} {$r_nbr < $nbr_rows} {incr r_nbr} {
      shift2left $r_nbr
   }
   redraw
   set MessText "Initial placement in rows of cell $CellName done."
}

proc get_nets {cell} {
   global net_arr

   set ret_list ""
   set idx [array startsearch net_arr]
   while {[array anymore net_arr $idx] == 1} {
      set net [array nextelement net_arr $idx]
      foreach c $net_arr($net) {
         if {[lindex $c 0] == $cell} {
            lappend ret_list $net
            break
         }
      }
   }
   return $ret_list
}

proc get_cons {net_list} {
   global srt_arr net_arr

   set ret_list ""
   foreach net $net_list {
      foreach n $net_arr($net) {
         set c [lindex $n 0]
         set idx [array startsearch srt_arr]
         while {[array anymore srt_arr $idx] == 1} {
            set nn [array nextelement srt_arr $idx]
            set i [lsearch $srt_arr($nn) $c]
            if {$i >= 0}  {
               lappend ret_list "$nn $i"
            }
         }
         array donesearch srt_arr $idx
      }
   }
   return $ret_list
}

proc add2srt_arr {cell con_list} {
   global srt_arr
   global debugtxt

   foreach c $con_list {
      if {[array names tmp_arr [lindex $c 0]] != ""} {
         incr tmp_arr([lindex $c 0])
      }\
      else {
         set tmp_arr([lindex $c 0]) [expr 1]
      }
   }
   set max_bind [expr 0]
   foreach c $con_list {
      if {$tmp_arr([lindex $c 0]) > $max_bind} {
         set max_bind $tmp_arr([lindex $c 0])
      }
   }
   set shortest 1000000
   set srt_nbr2add 0
   foreach c $con_list {
      if {$tmp_arr([lindex $c 0]) == $max_bind} {
         if {[llength $srt_arr([lindex $c 0])] < $shortest} {
            set shortest [llength $srt_arr([lindex $c 0])]
            set srt_nbr2add [lindex $c 0]
         }
      }
   }
   set av_tot [expr 0]
   set av_nbr [expr 0]
   foreach c $con_list {
      if {$debugtxt} {
         .debtxt insert end "C LINDEX SRT : $c [lindex $c 0] $srt_nbr2add\n"
      }
      if {[lindex $c 0] == $srt_nbr2add} {
         set av_tot [expr $av_tot + [lindex $c 1]]
         incr av_nbr
      }
   }
   if {$debugtxt} {
      .debtxt insert end "AV: $av_tot $av_nbr\n"
      update
   }

   if {$av_nbr > 0} {
      set av [expr $av_tot/$av_nbr]
   }\
   else {
      set av [expr 0]
   }

   if {$av < [llength $con_list]} {
      set srt_arr($srt_nbr2add) [linsert $srt_arr($srt_nbr2add) 0 $cell]
   }\
   else {
      set srt_arr($srt_nbr2add) [linsert $srt_arr($srt_nbr2add) end $cell]
   }
}

proc shift2left {r_nbr} {
   global debugtxt mc_arr y_arr row_nbrs

   set yt [lindex $y_arr([lindex $row_nbrs $r_nbr]) 0]
   set xlf [expr 10000]
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 1] == $yt} {
         if {[lindex $mc_arr($mc) 0] < $xlf} {
            set xlf [lindex $mc_arr($mc) 0]
         }
      }
   }
   array donesearch mc_arr $idx
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 1] == $yt} {
         set xl [expr [lindex $mc_arr($mc) 0] - $xlf]
         set xr [expr [lindex $mc_arr($mc) 2] - $xlf]
         set mc_arr($mc) [lreplace $mc_arr($mc) 0 0 $xl]
         set mc_arr($mc) [lreplace $mc_arr($mc) 2 2 $xr]
         if {$debugtxt} {
            if {$xl < 0 || $xr < 0} {
                set bc [lindex $mc_arr($mc) 5]
               .debtxt insert end "PLACE 3 $bc $xl $xr $xlf\n"
            }
         }
      }
   }
   array donesearch mc_arr $idx
}
