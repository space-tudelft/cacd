
proc read_cell {cell} {
   global mc_arr conn_arr CellName scale y0 box_arr

   reset
   set CellName $cell
   set y0 [expr 0]
   set err_nbr [expr 0]
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
         }\
         else {
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
   set nb [expr 0]
   set fp [open "|dbcat -s box $cell" r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {([lindex $txt 0] == "in") || ([lindex $txt 0] == "ins")} {
         if {[expr [lindex $txt 4] - [lindex $txt 3]] == 12} {
            set box_arr($nb) "[lindex $txt 1]\
                              [expr [lindex $txt 3] - 6]\
                              [lindex $txt 2]
                              [expr [lindex $txt 3] - 6]"
            incr nb
         }\
         elseif {[expr [lindex $txt 2] - [lindex $txt 1]] == 12} {
            set box_arr($nb) "[expr [lindex $txt 1] + 6]\
                              [lindex $txt 3]\
                              [expr [lindex $txt 1] + 6]\
                              [lindex $txt 4]"
            incr nb
         }
      }
   }
   catch {close $fp}
   redraw
}

proc write_cell {} {
   global CellName WriteCellName

   set WriteCellName $CellName
   if {[check_layout_cell]} {
      write_ldm
      create_layout $CellName.ldm
   }
}

proc write_as_cell {} {
   global CellName WriteCellName

   if {[ask_write_name]} {
      if {[check_layout_cell]} {
         write_ldm
         create_layout $WriteCellName.ldm
      }
   }
}

proc check_layout_cell {} {
   global WriteCellName

   if {[exist_layout_cell $WriteCellName]} {
      if {[tk_dialog .confirm_overw "" "Cell $WriteCellName already exists, are you sure ?" \
          questhead 1 Yes No] != 0} {
          return 0
      }
   }
   return 1
}

proc write_ldm {} {
   global WriteCellName MessText

   set fp [open $WriteCellName.ldm "w"]
   do_write_ldm $fp
   close $fp
   set MessText "file [pwd]/$WriteCellName.ldm (re)written"
}

proc write_as_ldm {} {
   global MessText

   set types {
      {{LDM Files}       {.ldm}        }
      {{All Files}        *            }
   }
   set file_name [tk_getSaveFile -filetypes $types]
   set fp [open $file_name "w"]
   do_write_ldm $fp
   close $fp
   set MessText "file $file_name (re)written"
}

proc do_write_ldm {fp} {
   global WriteCellName mc_arr

   puts $fp ":: no-origin mode LDM"
   puts $fp "ms $WriteCellName"
   set idx [array startsearch mc_arr]
   while {[array anymore mc_arr $idx] == 1} {
      set mc [array nextelement mc_arr $idx]
      if {[lindex $mc_arr($mc) 5] != "Error_Marker"} {
         set x [expr [lindex $mc_arr($mc) 0] - 34]
         set row [expr ([lindex $mc_arr($mc) 1] -7)/494]
         if {($row % 2) ==0} {
            set y [expr $row*494]
            set mirror "False"
         }\
         else {
            set y [expr $row*494 + 528]
            set mirror "True"
         }
         if {$mirror == "True"} {
            puts $fp "mc <$mc> [lindex $mc_arr($mc) 5] mx $x $y"
         }\
         else {
            puts $fp "mc <$mc> [lindex $mc_arr($mc) 5] $x $y"
         }
      }
   }
   array donesearch mc_arr $idx
   puts $fp "me"
}

proc create_layout {fname} {
   global MessText

   catch {exec cldm -f $fname} MessText
}
