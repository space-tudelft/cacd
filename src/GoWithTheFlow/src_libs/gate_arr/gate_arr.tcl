
proc redraw_mc {iname} {
   global y0 scale mc_arr

   .cv coords $iname [expr $scale*[lindex $mc_arr($iname) 0]]\
                     [expr $scale*($y0 - [lindex $mc_arr($iname) 1])]\
                     [expr $scale*[lindex $mc_arr($iname) 2]]\
                     [expr $scale*($y0 - [lindex $mc_arr($iname) 3])]
}

proc draw_conn {from to} {
   global mc_arr conn_arr scale

   set xf [expr $scale*([lindex $mc_arr($from) 2] + [lindex $mc_arr($from) 0])/2]
   set yf [expr $scale*([lindex $mc_arr($from) 3] + [lindex $mc_arr($from) 1])/2]
   set xt [expr $scale*([lindex $mc_arr($to) 2] + [lindex $mc_arr($to) 0])/2]
   set yt [expr $scale*([lindex $mc_arr($to) 3] + [lindex $mc_arr($to) 1])/2]
   set cname $from,$to
   switch $conn_arr($cname) {
      "1"       { .cv create line $xf $yf $xt $yt -width 1 -fill yellow -tag $cname }
      "2"       { .cv create line $xf $yf $xt $yt -width 2 -fill orange -tag $cname }
      "default" { .cv create line $xf $yf $xt $yt -width 2 -fill orange -tag $cname }
   }
}

proc rest_hilt {cname} {
   global conn_arr

   switch $conn_arr($cname) {
      "1"       { .cv itemconfigure $cname -fill yellow}
      "2"       { .cv itemconfigure $cname -fill orange}
      "default" { .cv itemconfigure $cname -fill red}
   }
}

proc reset {} {
   global mc_arr conn_arr con_list

   if {[array exists mc_arr]} { unset mc_arr }
   if {[array exists conn_arr]} { unset conn_arr }
   set con_list ""
   foreach i [.cv find all] { .cv delete $i }
}

proc add_conn {from to} {
   global mc_arr conn_arr

   set from [string trim  $from \"]
   set to   [string trim $to \"]
   if {($from != "") && ($to != "")} {
      if {[array names conn_arr $from,$to] == ""} {
         set conn_arr($from,$to) 1
      } else {
         set conn_arr($from,$to) [expr $conn_arr($from,$to) + 1]
      }
   }
}

proc redraw {} {
   global mc_arr scale nbr_rows row_nbrs y_arr y0

   foreach i [.cv find all] { .cv delete $i }
puts "++ y0 = $y0"
#   set y0 [lindex $y_arr([lindex $row_nbrs [expr $nbr_rows - 1]]) 1]
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      set xl [expr $scale*[lindex $mc_arr($mc) 0]]
      set xr [expr $scale*[lindex $mc_arr($mc) 2]]
      set yb [expr $scale*($y0/$scale - [lindex $mc_arr($mc) 1])]
      set yt [expr $scale*($y0/$scale - [lindex $mc_arr($mc) 3])]
puts "$xl,$yb <-> $xr,$yt"
      .cv create rectangle $xl $yb $xr $yt -fill wheat3 -tag [string trim $mc \"]
   }
   array donesearch mc_arr $idx
}

proc get_length {} {
   global mc_arr conn_arr len

   set len 0
   set idx [array startsearch conn_arr]
   set con [array nextelement conn_arr $idx]
   while {$con != ""} {
      set from [lindex [split $con ","] 0]
      set to   [lindex [split $con ","] 1]
      set xf [expr ([lindex $mc_arr($from) 2] + [lindex $mc_arr($from) 0])/2]
      set yf [expr ([lindex $mc_arr($from) 3] + [lindex $mc_arr($from) 1])/2]
      set xt [expr ([lindex $mc_arr($to) 2] + [lindex $mc_arr($to) 0])/2]
      set yt [expr ([lindex $mc_arr($to) 3] + [lindex $mc_arr($to) 1])/2]
      set nbr $conn_arr($from,$to)
      set len [expr $len + $nbr*(abs($xt-$xf) + abs($yt-$yf))]
      set con [array nextelement conn_arr $idx]
   }
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

proc add2srt_arr {cell con_list} {
   global srt_arr

   foreach c $con_list {
      if {[array names tmp_arr [lindex $c 0]] != ""} {
         incr tmp_arr([lindex $c 0])
      } else {
         set tmp_arr([lindex $c 0]) 1
      }
   }
   set max_bind 0
   foreach c $con_list {
      if {$tmp_arr([lindex $c 0]) > $max_bind} {
         set max_bind $tmp_arr([lindex $c 0])
      }
   }
   set shortest 10000
   foreach c $con_list {
      if {$tmp_arr([lindex $c 0]) == $max_bind} {
         if {[llength $srt_arr([lindex $c 0])] < $shortest} {
            set shortest [llength $srt_arr([lindex $c 0])]
            set srt_nbr2add [lindex $c 0]
         }
      }
   }
   set av_tot 0
   set av_nbr 0
   foreach c $con_list {
      if {[lindex $c 0] == $srt_nbr2add} {
         set av_tot [expr $av_tot + [lindex $c 1]]
         incr av_nbr
      }
   }
   set av [expr $av_tot/$av_nbr]
   if {$av < [llength $con_list]} {
      set srt_arr($srt_nbr2add) [linsert $srt_arr($srt_nbr2add) 0 $cell]
   } else {
      set srt_arr($srt_nbr2add) [linsert $srt_arr($srt_nbr2add) end $cell]
   }
}

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

proc move_before {cell} {
   global MovCel mc_data_arr mc_arr

   set mname [lindex $mc_arr($cell) 5]
   set iname [lindex $mc_arr($MovCel) 5]
   set dx [expr [lindex $mc_data_arr($iname) 0] +\
                [lindex $mc_data_arr($iname) 1] +\
                [lindex $mc_data_arr($iname) 3] + 1]
   set xln [expr [lindex $mc_arr($cell) 0] +\
                 ([lindex $mc_data_arr($mname) 0] -\
                  [lindex $mc_data_arr($iname) 0])*40]
   set xrn [expr [lindex $mc_arr($MovCel) 2] +\
                 $xln - [lindex $mc_arr($MovCel) 0]]
   set ytn [lindex $mc_arr($cell) 1]
   set ybn [lindex $mc_arr($cell) 3]
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set c [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($c) 1] == [lindex $mc_arr($cell) 1]} {
         if {[lindex $mc_arr($c) 0] >= [lindex $mc_arr($cell) 0]} {
            set txl [expr [lindex $mc_arr($c) 0] + 40*$dx]
            set txr [expr [lindex $mc_arr($c) 2] + 40*$dx]
            set mc_arr($c) [lreplace $mc_arr($c) 0 0 $txl]
            set mc_arr($c) [lreplace $mc_arr($c) 2 2 $txr]
         }
      }
      if {[lindex $mc_arr($c) 1] == [lindex $mc_arr($MovCel) 1]} {
         if {[lindex $mc_arr($c) 0] > [lindex $mc_arr($MovCel) 0]} {
            set txl [expr [lindex $mc_arr($c) 0] - 40*$dx]
            set txr [expr [lindex $mc_arr($c) 2] - 40*$dx]
            set mc_arr($c) [lreplace $mc_arr($c) 0 0 $txl]
            set mc_arr($c) [lreplace $mc_arr($c) 2 2 $txr]
         }
      }
   }
   array donesearch mc_arr $idx
   set mc_arr($MovCel) [lreplace $mc_arr($MovCel) 0 3 $xln $ytn $xrn $ybn]
}

proc get_smallest_row {} {
   global row_nbrs row_arr

   set val 1000000
   foreach i $row_nbrs {
      if {$row_arr($i) < $val} {
         set val $row_arr($i)
         set row $i
      }
   }
   return $row
}

proc comp_list {a b} {
   if {[lindex $a 1] < [lindex $b 1]} { return -1 }
   if {[lindex $a 1] > [lindex $b 1]} { return  1 }
   return 0
}

proc shift2left {r_nbr} {
   global mc_arr y_arr row_nbrs

   set yt [lindex $y_arr([lindex $row_nbrs $r_nbr]) 0]
   set xlf 10000
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 1] == $yt} {
         if {[lindex $mc_arr($mc) 0] < $xlf} {
            set xlf [lindex $mc_arr($mc) 0]
         }
      }
   }
   puts "row = $r_nbr -> yt = $yt and xlf = $xlf"
   array donesearch mc_arr $idx
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 1] == $yt} {
         set xl [expr [lindex $mc_arr($mc) 0] - $xlf]
         set xr [expr [lindex $mc_arr($mc) 2] - $xlf]
         set mc_arr($mc) [lreplace $mc_arr($mc) 0 0 $xl]
         set mc_arr($mc) [lreplace $mc_arr($mc) 2 2 $xr]
      }
   }
   array donesearch mc_arr $idx
}

proc init_cell {cell} {
   global mc_arr conn_arr CellName scale mc_data_arr row_arr row_nbrs Dx Dx0
   global srt_arr net_arr nbr_rows y_arr y0

   reset
   set row_nbrs "0 2 3 5 6 8 9 11 12 14"
   set y_arr(0) "11 517"
   set y_arr(1) "505  1011"
   set y_arr(2) "999  1505"
   set y_arr(3) "1493  1999"
   set y_arr(4) "1987  2493"
   set y_arr(5) "2481  2987"
   set y_arr(6) "2975  3481"
   set y_arr(7) "3469  3975"
   set y_arr(8) "3963  4469"
   set y_arr(9) "4457  4963"
   set y_arr(10) "4951  5457"
   set y_arr(11) "5445  5951"
   set y_arr(12) "5939  6445"
   set y_arr(13) "6433  6939"
   set y_arr(14) "6927  7433"
   set y_arr(15) "7421  7927"
   set y_arr(16) "7915  8421"
   set y_arr(17) "8409  8915"
   foreach i $row_nbrs { set row_arr($i) 0 }
   set CellName $cell
   set err_nbr 0
   set fp [open  "|dbcat -c -s net $CellName" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} continue
      if {[string index $txt 0] == "="} continue
      if {[string range $txt 0 3] == "net:"} {
         set net [string trim [lindex [split [lindex $txt 0] ":"] 1] "\""]
         if {$net != "vdd" && $net != "vss"} {
            set net_arr($net) ""
         }
      } else {
         set cell [string trim [lindex [split [lindex $txt 1] ":"] 1] "\""]
         if {$cell == ""} {
            set cell "{}"
         }
         set term [string trim [lindex [split [lindex $txt 0] ":"] 1] "\""]
         if {$net != "vdd" && $net != "vss"} {
            lappend net_arr($net) "$cell $term"
         }
      }
   }
   set srt_arr(0) ""
   set i_last 0
   set idx [array startsearch net_arr]
   while {[array anymore net_arr $idx] == 1} {
      set net [array nextelement net_arr $idx]
      if {[llength $net_arr($net)] == 2} {
         set c_from [lindex [lindex $net_arr($net) 0] 0]
         set t_from [lindex [lindex $net_arr($net) 0] 1]
         set c_to [lindex [lindex $net_arr($net) 1] 0]
         set t_to [lindex [lindex $net_arr($net) 1] 1]
         set idx_from -1
         set idx_to   -1
         for {set ii 0} {$ii < $i_last} {incr ii} {
            set idx_from [lsearch $srt_arr($ii) $c_from]
            if {$idx_from >= 0} {
               set srt_f $ii
               break
            }
         }
         for {set ii 0} {$ii < $i_last} {incr ii} {
            set idx_to   [lsearch $srt_arr($ii) $c_to]
            if {$idx_to >= 0} {
               set srt_t $ii
               break
            }
         }
         if {$idx_from < 0 && $idx_to < 0} {
            set t0 [string index $t_from 0]
            if {$t0 == "Y" || $t0 == "Q"} {
               set srt_arr($i_last) "$c_from $c_to"
            } else {
               set srt_arr($i_last) "$c_to $c_from"
            }
            incr i_last
         }\
         elseif {$idx_from >= 0 && $idx_to < 0} {
            set t0 [string index $t_to 0]
            if {$t0 == "Y" || $t0 == "Q"} {
               set srt_arr($srt_f) [linsert $srt_arr($srt_f) 0 $c_to]
            } else {
               set srt_arr($srt_f) [linsert $srt_arr($srt_f) end $c_to]
            }
         }\
         elseif {$idx_from < 0 && $idx_to >= 0} {
            set t0 [string index $t_from 0]
            if {$t0 == "Y" || $t0 == "Q"} {
               set srt_arr($srt_t) [linsert $srt_arr($srt_t) 0 $c_from]
            } else {
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
            } else {
               set srt_arr($srt_t) [concat $srt_arr($srt_t) $srt_arr($srt_f)]
               set srt_arr($srt_f) ""
            }
         }
      }
   }
   array donesearch net_arr $idx
   catch {close $fp}
   set fp [open "|dbcat -c -s mc $CellName" r]
   set mc_list ""
   set to_place_list ""
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} continue
      if {[string index $txt 0] == "="} continue
      set bc [string trim [string range [lindex $txt 0] 5 end] "\""]
      set mc [string trim [string range [lindex $txt 1] 5 end] "\""]
      set mc_arr($mc) "0 0 0 0 0 $bc"
      lappend mc_list $mc
   }
   foreach c $mc_list {
      set found "False"
      for {set i 0} {$i < $i_last} {incr i} {
         if {[lsearch $srt_arr($i) $c] >= 0} {
            set found "True"
            break
         }
      }
      if {$found == "False"} {
         lappend to_place_list $c
      }
   }
   catch {close $fp}
   foreach c $to_place_list {
      set net_list [get_nets $c]
      set con_list [get_cons $net_list]
      add2srt_arr $c $con_list
   }
   set nxt_place_0 0
   set nxt_place_1 0
   set idx [array startsearch srt_arr]
   while {[array anymore srt_arr $idx] == 1} {
      set c_list $srt_arr([array nextelement srt_arr $idx])
      for {set i 0} {$i < [llength $c_list]} {incr i} {
         set ic [lindex $c_list $i]
         set bc [lindex $mc_arr($ic) 5]
         if {$nxt_place_0 <= $nxt_place_1} {
            set xl [expr ($nxt_place_0+$Dx+ [lindex $mc_data_arr($bc) 0])*40 + 34]
            set xr [expr $xl + 40*[lindex $mc_data_arr($bc) 3]]
            set yt [lindex $y_arr(0) 0]
            set yb [lindex $y_arr(0) 1]
            set mc_arr($ic) [lreplace $mc_arr($ic) 0 3 $xl $yt $xr $yb]
            set nxt_place_0 [expr $nxt_place_0 + $Dx +\
                             [lindex $mc_data_arr($bc) 0] +\
                             [lindex $mc_data_arr($bc) 1] +\
                             [lindex $mc_data_arr($bc) 3]]
         } else {
            set xl [expr ($nxt_place_1+$Dx+ [lindex $mc_data_arr($bc) 0])*40 + 34]
            set xr [expr $xl + 40*[lindex $mc_data_arr($bc) 3]]
            set yt [lindex $y_arr(2) 0]
            set yb [lindex $y_arr(2) 1]
            set mc_arr($ic) [lreplace $mc_arr($ic) 0 3 $xl $yt $xr $yb]
            set nxt_place_1 [expr $nxt_place_1 + $Dx +\
                             [lindex $mc_data_arr($bc) 0] +\
                             [lindex $mc_data_arr($bc) 1] +\
                             [lindex $mc_data_arr($bc) 3]]
         }
      }
   }
   array donesearch srt_arr $idx
   if {$nbr_rows > 2} {
      set tmp_list ""
      if {$nxt_place_0 <= $nxt_place_1} {
         set len [expr int(2*$nxt_place_0/$nbr_rows)*40]
      } else {
         set len [expr int(2*$nxt_place_1/$nbr_rows)*40]
      }
      set idx [array startsearch mc_arr]
      while {[array anymore mc_arr $idx] == 1} {
         set ci [array nextelement mc_arr $idx]
         lappend tmp_list "$ci [lindex $mc_arr($ci) 0]"
      }
      array donesearch mc_arr $idx
   }
   set tmp_list [lsort -command comp_list $tmp_list]
   for {set it 0} {$it < [llength $tmp_list]} {incr it} {
      set ci [lindex [lindex $tmp_list $it] 0]
      set nl [expr int([lindex [lindex $tmp_list $it] 1]/$len)]
      if {$nl > [expr $nbr_rows/2 - 1]} {
         set nl [expr $nbr_rows/2 - 1]
      }
      if {[expr $nl % 2] == 0} {
         set xl [expr [lindex $mc_arr($ci) 0] - $nl*$len]
         set xr [expr [lindex $mc_arr($ci) 2] - $nl*$len]
      } else {
         set xr [expr ($nl+1)*$len - [lindex $mc_arr($ci) 0]+34]
         set xl [expr ($nl+1)*$len - [lindex $mc_arr($ci) 2]+34]
      }
      if {[lindex $mc_arr($ci) 1] == [lindex $y_arr(0) 0]} {
         set yt [lindex $y_arr([expr 3*$nl]) 0]
         set yb [lindex $y_arr([expr 3*$nl]) 1]
      } else {
         set yt [lindex $y_arr([expr 2+3*$nl]) 0]
         set yb [lindex $y_arr([expr 2+3*$nl]) 1]
      }
      set mc_arr($ci) [lreplace $mc_arr($ci) 0 3 $xl $yt $xr $yb]
   }
   for {set r_nbr 0} {$r_nbr < $nbr_rows} {incr r_nbr} {
      shift2left $r_nbr
   }
   set y0 [lindex $y_arr([lindex $row_nbrs [expr $nbr_rows - 1]]) 1]
   puts "init: y0 = $y0"
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set ci [array nextelement mc_arr $idx]
      set xl [expr $scale*[lindex $mc_arr($ci) 0]]
      set xr [expr $scale*[lindex $mc_arr($ci) 2]]
      set yb [expr $scale*($y0 - [lindex $mc_arr($ci) 1])]
      set yt [expr $scale*($y0 - [lindex $mc_arr($ci) 3])]
      .cv create rectangle $xl $yb $xr $yt -fill wheat3 -tag [string trim $ci \"]
   }
   array donesearch mc_arr $idx
   set bbox [.cv bbox all]
   .cv configure -scrollregion "0 0 [lindex $bbox 2] [lindex $bbox 3]"
}

proc read_cell {cell} {
   global mc_arr conn_arr CellName scale y0

   reset
   set CellName $cell
   set y0 0
   set err_nbr 0
   set fp [open "|dbcat -s mc $cell" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {([lindex $txt 1] != "IMAGE") && ([llength $txt] >= 12)} {
         set mc [lindex $txt 1]
         if {[lindex $txt 0] == "Error_Marker"} {
            append mc ",$err_nbr"
            incr err_nbr
            set mc_arr($mc) "[lindex $txt 8]\
                             [lindex $txt 10]\
                             [lindex $txt 9]\
                             [lindex $txt 11] 1\
                             [lindex $txt 0]"
         } else {
            if {[lindex $txt 11] > $y0} {
               set y0 [lindex $txt 11]
            }
            set mc_arr($mc) "[lindex $txt 8]\
                             [lindex $txt 10]\
                             [lindex $txt 9]\
                             [lindex $txt 11] 0\
                             [lindex $txt 0]"
         }
      }
   }
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 5] != "Error_Marker"} {
         .cv create rectangle [expr $scale*[lindex $mc_arr($mc) 0]]\
                              [expr $scale*($y0 - [lindex $mc_arr($mc) 1])]\
                              [expr $scale*[lindex $mc_arr($mc) 2]]\
                              [expr $scale*($y0 - [lindex $mc_arr($mc) 3])]\
                              -fill wheat3 -tag [string trim $mc \"]
      } else {
         .cv create rectangle [expr $scale*[lindex $mc_arr($mc) 0]]\
                              [expr $scale*($y0 - [lindex $mc_arr($mc) 1])]\
                              [expr $scale*[lindex $mc_arr($mc) 2]]\
                              [expr $scale*($y0 - [lindex $mc_arr($mc) 3])]\
                                 -fill red -tag ERRMARK
      }
      .cv raise ERRMARK
   }
   set fp [open "|dbcat -s box $cell" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {([lindex $txt 0] == "in") || ([lindex $txt 0] == "ins")} {
         if {[expr [lindex $txt 4] - [lindex $txt 3]] == 12} {
            .cv create line [expr $scale*[lindex $txt 1]]\
                            [expr $scale*($y0 - [lindex $txt 3] - 6)]\
                            [expr $scale*[lindex $txt 2]]\
                            [expr $scale*($y0 - [lindex $txt 3] - 6)]\
                            -fill orchid1
         }\
         elseif {[expr [lindex $txt 2] - [lindex $txt 1]] == 12} {
            .cv create line [expr $scale*([lindex $txt 1] + 6)]\
                            [expr $scale*($y0 - [lindex $txt 4])]\
                            [expr $scale*([lindex $txt 1] + 6)]\
                            [expr $scale*($y0 - [lindex $txt 3])]\
                            -fill orchid3
         }
      }
   }
   catch {close $fp}
   set bbox [.cv bbox all]
   set xl_sr [lindex $bbox 0]
   set xr_sr [lindex $bbox 2]
   set yb_sr [.cv cget -height]
   set yt_sr [expr [.cv cget -height] - [lindex $bbox 3]]
   .cv configure -scrollregion "$xl_sr $yt_sr $xr_sr $yb_sr"
   redraw

#   set fp [open "|dbcat -c -s net $cell" r]
#   while {![eof $fp]} {
#      set txt [gets $fp]
#      if {[string first net: $txt] == 0} {
#         set netname [string range [lindex $txt 0] 4 end]
#         set ns [string range [lindex $txt 3] 8 end]
#         if {($netname != "\"vss\"") && ($netname != "\"vdd\"")} {
#            set sub_list ""
#            for {set i 0} {$i < $ns} {incr i} {
#               set txt [gets $fp]
#               set sub [string range [lindex $txt 1] 5 end]
#               lappend sub_list $sub
#            }
#            for {set i 0} {$i < [llength $sub_list]} {incr i} {
#               for {set ii [expr $i+1]} {$ii < [llength $sub_list]} {incr ii} {
#                  add_conn [lindex $sub_list $i] [lindex $sub_list $ii]
#               }
#            }
#         }
#      }
#   }
}

proc write_ldm {} {
   global CellName mc_arr

   set fp [open $CellName.ldm "w"]
   puts $fp ":: no-origin mode LDM"
   puts $fp "ms $CellName"
parray mc_arr
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 5] != "Error_Marker"} {
         set x [expr [lindex $mc_arr($mc) 0] - 34]
         set row [expr ([lindex $mc_arr($mc) 1] -7)/494]
         if {($row % 2) ==0} {
            set y [expr $row*494]
            set mirror "False"
         } else {
            set y [expr $row*494 + 528]
            set mirror "True"
         }
         if {$mirror == "True"} {
            puts $fp "mc <$mc> [lindex $mc_arr($mc) 5] mx $x $y"
         } else {
            puts $fp "mc <$mc> [lindex $mc_arr($mc) 5] $x $y"
         }
      }
   }
   array donesearch mc_arr $idx
   puts $fp "me"
   close $fp
}

proc create_layout {} {
   global CellName

   exec cldm -f $CellName.ldm
}

proc show_errors {} {
   global mc_arr scale y0

   set idx [array startsearch mc_arr]
   set nbr_errs 0
   while {[array anymore mc_arr $idx] == 1} {
   set cell [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($cell) 4] > 0} {
         incr nbr_errs
         set net [lindex [split $cell ","] 0]
         set xc  [expr $scale*([lindex $mc_arr($cell) 0] + [lindex $mc_arr($cell) 2])/2]
         set yc  [expr $scale*($y0 - ([lindex $mc_arr($cell) 1] + [lindex $mc_arr($cell) 3])/2)]
         if {[array names err_arr $net] == ""} {
            set err_arr($net) "$xc $yc"
         } else {
            set err_arr($net) "$err_arr($net) $xc $yc"
         }
      }
   }
   array donesearch mc_arr $idx
   if {$nbr_errs > 0} {
      set idx [array startsearch err_arr]
      set err [array nextelement err_arr $idx]
      while {$err != ""} {
         .cv create line $err_arr($err) -fill red -tag ERROR
         set err [array nextelement err_arr $idx]
      }
      .cv raise ERROR
      array donesearch err_arr $idx
   }
}

proc hide_errors {} {
   .cv delete withtag ERROR
}

proc show_cons {} {
   global conn_arr

   set idx [array startsearch conn_arr]
   set c [array nextelement conn_arr $idx]
   while {$c != ""} {
      .cv delete withtag $c
      set from [lindex [split $c ,] 0]
      set to   [lindex [split $c ,] 1]
      draw_conn $from $to
      set c [array nextelement conn_arr $idx]
   }
   array donesearch conn_arr $idx
}

proc hide_cons {} {
   global conn_arr

   set idx [array startsearch conn_arr]
   set c [array nextelement conn_arr $idx]
   while {$c != ""} {
      .cv delete withtag $c
      set c [array nextelement conn_arr $idx]
   }
   array donesearch conn_arr $idx
}
