
proc MAX {a b} {
   if {$b > $a} {
      return $b
   } else {
      return $a
   }
}

proc get_ldm_out_files {} {
   global CellName

   .menubar.file.cmds.out delete 0 end
   set ldm_files [glob -nocomplain *.ldm]
   foreach i_ldm $ldm_files {
      set fp [open $i_ldm "r"]
      while {![eof $fp]} {
         set txt [gets $fp]
         if {[lindex $txt 0] == "ms"} {
            if {[lindex $txt 1] == $CellName} {
               .menubar.file.cmds.out add command -label $i_ldm -command "create_layout $i_ldm"
            }
            break
         }
      }
      close $fp
   }
}

proc get_layout_cells {} {
   .menubar.file.cmds.cells delete 0 end
   set fpc [open ./layout/celllist "r"]
   set ic [expr 0]
   while {![eof $fpc]} {
      set txt [gets $fpc]
      if {$txt != ""} {
         .menubar.file.cmds.cells add command -label $txt -command "read_cell $txt"
         incr ic
         if {[expr $ic % 20] == 0} {
            .menubar.file.cmds.cells entryconfigure $ic -columnbreak 1
         }
      }
   }
   close $fpc
}

proc exist_layout_cell {cell} {
   set fpc [open ./layout/celllist "r"]
   set ic [expr 0]
   while {![eof $fpc]} {
      set txt [gets $fpc]
      if {$txt == $cell} {
         close $fpc
         return 1
      }
   }
   close $fpc
   return 0
}

proc get_circuit_cells {} {
   .menubar.file.cmds.init delete 0 end
   set fpc [open ./circuit/celllist "r"]
   set ic [expr 0]
   while {![eof $fpc]} {
      set txt [gets $fpc]
      if {$txt != ""} {
         .menubar.file.cmds.init add command -label $txt -command "init_cell $txt"
         incr ic
         if {[expr $ic % 20] == 0} {
            .menubar.file.cmds.init entryconfigure $ic -columnbreak 1
         }
      }
   }
   close $fpc
}

proc get_mad_circuit_cells {} {
   .menubar.file.cmds.madonna delete 0 end
   set fpc [open ./circuit/celllist "r"]
   set ic [expr 0]
   while {![eof $fpc]} {
      set txt [gets $fpc]
      if {$txt != ""} {
         .menubar.file.cmds.madonna add command -label $txt -command "madonna_place $txt"
         incr ic
         if {[expr $ic % 20] == 0} {
            .menubar.file.cmds.madonna entryconfigure $ic -columnbreak 1
         }
      }
   }
   close $fpc
}

proc com_madonna_place {} {
   global CellName

   madonna_place $CellName
}

proc madonna_place {cell} {
   global MessText

   catch {exec sea -p $cell >& tmp_sea}
# puts "$sea_mess"
   read_cell $cell
   set MessText "Initial placement with madonna of cell $cell done."
}

proc update_region {} {
   global y0 scale mc_arr

   set xmax [expr 0]
   set ymax [expr 0]
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set cn [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($cn) 2] >= $xmax} {
         set xmax [lindex $mc_arr($cn) 2]
      }
      if {[lindex $mc_arr($cn) 3] >= $ymax} {
         set ymax [lindex $mc_arr($cn) 3]
      }
   }
   array donesearch mc_arr $idx
   set bbox [.cv bbox all]
   set xl_sr [expr 0]
   set xr_sr [MAX [expr $scale*$xmax] [.cv cget -width]]
   set yt_sr [expr 0]
   set yb_sr [MAX [expr $scale*$ymax] [.cv cget -height]]
   .cv configure -scrollregion "$xl_sr $yt_sr $xr_sr $yb_sr"
   set y0 $yb_sr
}

proc rescale {n1 n2 op} {
   redraw
}

proc redraw_mc {iname} {
   global y0 scale mc_arr

   .cv coords $iname [expr $scale*[lindex $mc_arr($iname) 0]]\
                     [expr $y0 - $scale*[lindex $mc_arr($iname) 1]]\
                     [expr $scale*[lindex $mc_arr($iname) 2]]\
                     [expr $y0 - $scale*[lindex $mc_arr($iname) 3]]
}

proc reset {} {
   global mc_arr conn_arr box_arr err_arr net_arr srt_arr con_list

   if {[array exists mc_arr]} {
      unset mc_arr
   }
   if {[array exists conn_arr]} {
      unset conn_arr
   }
   if {[array exists box_arr]} {
      unset box_arr
   }
   if {[array exists err_arr]} {
      unset err_arr
   }
   if {[array exists srt_arr]} {
      unset srt_arr
   }
   if {[array exists net_arr]} {
      unset net_arr
   }
   set con_list ""
   foreach i [.cv find all] {
      .cv delete $i
   }
}

proc redraw {} {
   global mc_arr box_arr err_arr
   global scale nbr_rows row_nbrs y_arr y0 CurCel ShowCons ShowErrs

   update_region
   foreach i [.cv find all] {
      .cv delete $i
   }
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      set xl [expr $scale*[lindex $mc_arr($mc) 0]]
      set xr [expr $scale*[lindex $mc_arr($mc) 2]]
      set yb [expr $y0 - $scale*[lindex $mc_arr($mc) 1]]
      set yt [expr $y0 - $scale*[lindex $mc_arr($mc) 3]]
      if {[lindex $mc_arr($mc) 5] != "Error_Marker"} {
         if {$mc == $CurCel} {
            .cv create rectangle $xl $yb $xr $yt -fill blue -tag [string trim $mc \"]
         } else {
            .cv create rectangle $xl $yb $xr $yt -fill wheat3 -tag [string trim $mc \"]
         }
      } else {
         .cv create rectangle $xl $yb $xr $yt -fill red -tag ERRMARK
      }
   }
   array donesearch mc_arr $idx
   .cv raise ERRMARK
   if {$ShowCons != 0} {
      for {set i [expr 0]} {$i < [array size box_arr]} {incr i} {
         .cv create line [expr $scale*[lindex $box_arr($i) 0]]\
                         [expr $y0 - $scale*[lindex $box_arr($i) 1]]\
                         [expr $scale*[lindex $box_arr($i) 2]]\
                         [expr $y0 - $scale*[lindex $box_arr($i) 3]]\
                         -fill orchid1
      }
   }
   if {($ShowErrs != 0) && [array exists err_arr]} {
      set idx [array startsearch err_arr]
      while {[array anymore err_arr $idx] == 1} {
         set err [array nextelement err_arr $idx]
         set ec_list ""
         for {set i [expr 0]} {$i < [llength $err_arr($err)]} {incr i} {
             if {[expr $i % 2 ] == 0} {
                lappend ec_list [expr $scale*[lindex $err_arr($err) $i]]
             } else {
                lappend ec_list [expr $y0 - $scale*[lindex $err_arr($err) $i]]
             }
         }
         .cv create line $ec_list -fill red -tag ERROR
      }
      array donesearch err_arr $idx
      .cv raise ERROR
   }
}

proc comp_list {a b} {
   if {[lindex $a 1] < [lindex $b 1]} {
      return -1
   } elseif {[lindex $a 1] > [lindex $b 1]} {
      return 1
   } else {
      return 0
   }
}

proc show_errors {} {
   global mc_arr err_arr scale y0

   set idx [array startsearch mc_arr]
   set nbr_errs [expr 0]
   while {[array anymore mc_arr $idx] == 1} {
   set cell [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($cell) 4] > 0} {
         incr nbr_errs
         set net [lindex [split $cell ","] 0]
         set xc [expr ([lindex $mc_arr($cell) 0] + [lindex $mc_arr($cell) 2])/2]
         set yc [expr ([lindex $mc_arr($cell) 1] + [lindex $mc_arr($cell) 3])/2]
         if {[array names err_arr $net] == ""} {
            set err_arr($net) "$xc $yc"
         } else {
            set err_arr($net) "$err_arr($net) $xc $yc"
         }
      }
   }
   array donesearch mc_arr $idx
   redraw
}
