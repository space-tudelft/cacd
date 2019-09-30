
proc mk_reffile {} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the reference file can be made    #
#-----------------------------------------------------------------------------#
   global RefFileName Fnt MaxNbrMenuItemsInColumn

   upd_status_line "mk_ref_file: BUSY"

   set RefFileName ""

   toplevel  .mkref
   frame     .mkref.fr       -relief raised -bd 2 -bg gold
   label     .mkref.sl -relief sunk -bd 2 -bg wheat3 -text ""\
                        -font $Fnt -anchor w -padx 10 -pady 5
   text      .mkref.txt      -width 60 -height 25 -font $Fnt -bg wheat\
                             -yscrollcommand ".mkref.sb set"
   scrollbar .mkref.sb       -command ".mkref.txt yview" -bg wheat
   menubutton .mkref.fr.file -text "File" -font $Fnt\
                             -menu .mkref.fr.file.cmds -bg gold
   menu       .mkref.fr.file.cmds -font $Fnt -bg wheat3
              .mkref.fr.file.cmds add cascade -label "Read"\
                                              -menu .mkref.fr.file.cmds.read
              .mkref.fr.file.cmds add cascade -label "Generate from"\
                                              -menu .mkref.fr.file.cmds.gen
              .mkref.fr.file.cmds add cascade -label "UpdateForBondbar" -state disabled\
                                              -menu .mkref.fr.file.cmds.bbar
              .mkref.fr.file.cmds add command -label "Save" -state disabled\
                                              -command "do_write_ref 0"
              .mkref.fr.file.cmds add command -label "Save as ..." -state disabled\
                                              -command "do_write_ref 1"
              .mkref.fr.file.cmds add command -label "Save as type res" -state disabled\
                                              -command "do_write_res 0"
              .mkref.fr.file.cmds add command -label "Save as type res as ..." -state disabled\
                                              -command "do_write_res 1"
              .mkref.fr.file.cmds add command -label "Quit"\
                                              -command {destroy .mkref; upd_status_line ""}
   menu       .mkref.fr.file.cmds.read -font $Fnt -bg wheat3 -tearoff 0
   menu       .mkref.fr.file.cmds.gen  -font $Fnt -bg wheat3 -tearoff 0
   menu       .mkref.fr.file.cmds.bbar -font $Fnt -bg wheat3 -tearoff 0

   set lst_files [lsort [glob -nocomplain "*.lst"]]
   for {set i 0} {$i < [llength $lst_files]} {incr i} {
      set i_lst [lindex $lst_files $i]
      .mkref.fr.file.cmds.gen add command -label $i_lst -command "do_mk_reffile $i_lst"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .mkref.fr.file.cmds.gen entryconfigure $i_lst -columnbreak 1
      }
   }

   set ref_files [lsort [glob -nocomplain "*.ref"]]
   for {set i 0} {$i < [llength $ref_files]} {incr i} {
      set i_ref [lindex $ref_files $i]
      .mkref.fr.file.cmds.read add command -label $i_ref -command "do_rd_reffile $i_ref"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .mkref.fr.file.cmds.read entryconfigure $i_ref -columnbreak 1
      }
   }

   set buf_files [glob -nocomplain "*.buf"]
   foreach i_buf $buf_files {
      .mkref.fr.file.cmds.bbar add command -label $i_buf -command "update_ref_bondbar $i_buf"
   }

   pack .mkref.fr      -side top -fill x
   pack .mkref.sl      -side bottom -fill x
   pack .mkref.fr.file -side left
   pack .mkref.sb      -side right -fill y
   pack .mkref.txt     -side left -fill both -expand 1
   tkwait visibility .mkref.txt
}

proc is_klok {t_nbr} {
#-----------------------------------------------------------------------------#
# procedure to check if a port is a clock and what its period is              #
#-----------------------------------------------------------------------------#
   set idx [.mkref.txt search DATA_SECTION 1.0]
   set ln_start [expr [lindex [split $idx .] 0] + 1]
   set ln_last [expr [lindex [split [.mkref.txt index end] .] 0] -1]
   set idx [.mkref.txt search < $idx]
   set sep_col [lindex [split $idx .] 1]
   set col_nbr [expr $sep_col+$t_nbr]
   set prev_val "U"
   set prev_time -1
   set dt_hl -1
   set dt_lh -1
   set n_chg 0
   for {set i $ln_start} {$i < $ln_last} {incr i} {
      set new_val [.mkref.txt get $i.$col_nbr]
      set tmp_time_str [string trimleft [.mkref.txt get $i.0 $i.$sep_col].0]
      set new_time [expr double($tmp_time_str)]
      if {$new_val != $prev_val} {
         incr n_chg
         set prev_val $new_val
         if {$new_val == "L"} {
            if {$prev_time < 0} {
               set prev_time $new_time
            } elseif {$dt_hl < 0} {
               set dt_hl [expr $new_time - $prev_time]
               set prev_time $new_time
            } elseif {$new_time - $prev_time != $dt_hl} {
               return "-1 $n_chg"
            } else {
               set prev_time $new_time
            }
         }\
         elseif {$new_val == "H"} {
            if {$prev_time < 0} {
               set prev_time $new_time
            } elseif {$dt_lh < 0} {
               set dt_lh [expr $new_time - $prev_time]
               set prev_time $new_time
            } elseif {$new_time - $prev_time != $dt_lh} {
               return "-1 $n_chg"
            } else {
               set prev_time $new_time
            }
         }
      }
   }
   if {$dt_hl < 0 || $dt_lh < 0} {
      return "-1 $n_chg"
   } else {
      return "[expr $dt_hl + $dt_lh] $n_chg"
   }
}

proc write_terms {name first last type} {
#-----------------------------------------------------------------------------#
# procedure write the correct terminal_names to the ref_file                  #
#-----------------------------------------------------------------------------#
   if {$name == ""} return

   if {$type == "I"} {
      set type "INP"
   } elseif {$type == "O"} {
      set type "OUT"
   } else {
      set type "INOUT"
   }

   if {$first == -1 || $last == -1} {
      .mkref.txt insert end "$type $name;\n"
   } elseif {$last > $first} {
      for {set i $first} {$i <= $last} {incr i} {
         .mkref.txt insert end "$type $name\_$first\_$last\_$i;\n"
      }
   } else {
      for {set i $first} {$i >= $last} {incr i -1} {
         .mkref.txt insert end "$type $name\_$first\_$last\_$i;\n"
      }
   }
}

proc do_mk_reffile {lst_file_name} {
#-----------------------------------------------------------------------------#
# procedure to generate the reference_file                                    #
#-----------------------------------------------------------------------------#
   global RefFileName MyWd

   .mkref.sl configure -text "generating ref_file from file $lst_file_name"
   update
   .mkref.txt delete 1.0 end
   set defFileName "[lindex [split $lst_file_name .] 0].def"
   set RefFileName "[lindex [split $lst_file_name .] 0].ref"
   set f_def [open $defFileName]
   set nt 0
   set term_name ""
   set term_first -1
   set term_last -1
   set term_type "I"
   set dt_string [clock format [clock seconds] -format "%a %d %b %Y %H:%M:%S"]
   .mkref.txt insert end "# Tabular Link File created on $dt_string\n"
   .mkref.txt insert end "# By Program: design_flow\n"
   .mkref.txt insert end "# From Source: $MyWd/$lst_file_name\n"
   while {![eof $f_def]} {
      set tt [gets $f_def]
      if {$tt == ""} {
         continue
      }
      incr nt
      set idx [expr [string last "/" [lindex $tt 0]]+1]
      if {[llength $tt] > 3} {
         set next_term_type "IO"
      } else {
         set next_term_type [lindex $tt 2]
      }
      set type([lindex $tt 1]) $next_term_type
      set next_term [string range [lindex $tt 0] $idx end]
      set tmp [split $next_term "\(\)"]
      if {[lindex $tmp 0] != $term_name} {
         if {$term_name != ""} {
            write_terms $term_name $term_first $term_last $term_type
         }
         set term_name [lindex $tmp 0]
         set term_type $next_term_type
         if {[llength $tmp] > 1} {
            set term_first [lindex $tmp 1]
         } else {
            set term_first -1
         }
      }\
      else {
         set term_last [lindex $tmp 1]
      }
   }
   close $f_def
   write_terms $term_name $term_first $term_last $term_type
   .mkref.txt insert end "PERIOD\n"
   .mkref.txt insert end "TIME_UNIT 1.0e-10s;\n"
   .mkref.txt insert end "TABLE_FORMAT D<${nt}c;\n"
   .mkref.txt insert end "DATA_SECTION\n"
   set fp [open $lst_file_name]
   while {![eof $fp]} {
      set tt [gets $fp]
      if {$tt == ""} {
         continue
      }
      .mkref.txt insert end [format "%15s<" [lindex $tt 0]0]
      set lasttime [lindex $tt 0]
      set valstr [lindex $tt 1]
      for {set i 0} {$i < [string length $valstr]} {incr i} {
         # if {$type([expr $i+1]) == "W"} continue
         switch [string index $valstr $i] {
            D { .mkref.txt insert end "L" }
            0 { .mkref.txt insert end "L" }
            L { .mkref.txt insert end "l" }
            U { .mkref.txt insert end "H" }
            1 { .mkref.txt insert end "H" }
            H { .mkref.txt insert end "h" }
            N { .mkref.txt insert end "X" }
            Z { .mkref.txt insert end "X" }
            X { .mkref.txt insert end "x" }
            T { .mkref.txt insert end "x" }
            ? { .mkref.txt insert end "x" }
         }
      }
      .mkref.txt insert end "\n"
    }
    close $fp
    set period [expr double($lasttime) * 10]
    set trm_clk -1
    # require at at least 4 periods for a clock signal (nr transitions > 7)
    set n_tr 7
    for {set ic 1} {$ic <= $nt} {incr ic} {
       set new_per [is_klok $ic]
       if {[lindex $new_per 0] > 0 && [lindex $new_per 1] > $n_tr} {
          set period [lindex $new_per 0]
          set n_tr   [lindex $new_per 1]
          set trm_clk [expr $ic - 1]
       }
    }
   set idx [.mkref.txt search PERIOD 1.0]
   set period [lindex [split $period .] 0]
   set last [expr [string length $period] -2]
   set period [string range $period 0 $last]
   .mkref.txt insert [lindex [split $idx .] 0].end " ${period}ns;"
   .mkref.txt insert 4.0 "# Clock_column: $trm_clk\n"
   .mkref.sl configure -text "generation of ref_data from file '$lst_file_name' ready"
   .mkref.fr.file.cmds entryconfigure "UpdateForBondbar" -state normal
   .mkref.fr.file.cmds entryconfigure "Save" -state normal
   .mkref.fr.file.cmds entryconfigure "Save as ..." -state normal
   .mkref.fr.file.cmds entryconfigure "Save as type res" -state normal
   .mkref.fr.file.cmds entryconfigure "Save as type res as ..." -state normal
   update
}

proc do_write_ref {askfilename} {
#-----------------------------------------------------------------------------#
# procedure to write the reference_file to a file                             #
#-----------------------------------------------------------------------------#
   global RefFileName

   if {$RefFileName == ""} {
      .mkref.sl configure -text "First read or generate a ref file!"
      return
   }

   if {$askfilename} {
      set types {
           {{REF Files} {.ref}}
           {{All files}  *    }
      }
      set name [tk_getSaveFile -initialdir . -initialfile $RefFileName -filetypes $types]
      if {$name == ""} return
   } else {
      set name $RefFileName
   }

   set fpo [open $name w]
   puts $fpo [.mkref.txt get 1.0 end]
   close $fpo
   .mkref.sl configure -text "file '$name' (over)written"
   update
}

proc do_write_res {askfilename} {
#-----------------------------------------------------------------------------#
# procedure to write the reference_file to a file                             #
#-----------------------------------------------------------------------------#
   global RefFileName

   if {$RefFileName == ""} {
      .mkref.sl configure -text "First read or generate a ref file!"
      return
   }

   regsub ".ref$" $RefFileName ".res" ResFileName

   if {$askfilename} {
      set types {
           {{RES Files} {.res}}
           {{All files}  *    }
      }
      set name [tk_getSaveFile -initialdir . -initialfile $ResFileName -filetypes $types]
      if {$name == ""} return
   } else {
      set name $ResFileName
   }

   set fpo [open $name w]

   set idx [.mkref.txt search TIME_UNIT 1.0]
   set i [lindex [split $idx .] 0]
   set tt [.mkref.txt get $i.0 $i.end]
   set l [split $tt " ;#"]
   set time_unit [regsub "s" [lindex $l 1] ""]
   puts -nonewline $fpo "$time_unit"

   set data_body 0
   set ln_last [expr [lindex [split [.mkref.txt index end] .] 0] -1]
   for {set i 1} {$i < $ln_last} {incr i} {
      set tt [.mkref.txt get $i.0 $i.end]

      if {$tt == "" || [string index $tt 0] == "#"} {
         continue
      }

      if {$data_body} {
         regsub "<" $tt "" tt
         regsub -all "H" $tt "h" tt
         regsub -all "L" $tt "l" tt
         regsub -all "X" $tt "x" tt
         puts $fpo $tt
         continue
      }

      set l [split $tt " ;#"]
      if {[lindex $l 0] == "INP"} {
         puts -nonewline $fpo " ( [lindex $l 1] )"
      }\
      elseif {[lindex $l 0] == "OUT"} {
         puts -nonewline $fpo " ( [lindex $l 1] )"
      }\
      elseif {[lindex $l 0] == "INOUT"} {
         puts -nonewline $fpo " ( [lindex $l 1] )"
      }\
      elseif {[lindex $l 0] == "DATA_SECTION"} {
         puts $fpo ""
         set data_body 1
      }
   }

   close $fpo
   .mkref.sl configure -text "file '$name' (over)written"
   update
}

proc update_ref_bondbar {buf_file_name} {
#-----------------------------------------------------------------------------#
# procedure to change the terminal_names to the terminal_names of the bondbar #
#-----------------------------------------------------------------------------#
   global RefFileName

   if {$RefFileName == ""} {
      .mkref.sl configure -text "First read or generate a ref file!"
      return
   }

   .mkref.sl configure -text "updating for bondbar using file '$buf_file_name'"
   update
   set t_name {}
   set t_new  {}
   set t_done {}
   set f_buf [open $buf_file_name]
   while {![eof $f_buf]} {
      set tt [gets $f_buf]
      if {[llength $tt] == 3} {
         lappend t_name [lindex $tt 2]
         lappend t_new  [lindex $tt 0]
         lappend t_done "to_do"
      }
   }
   close $f_buf

   set len [string length $RefFileName]
   if {[string last "_bb.ref" $RefFileName] != $len-7} {
      # We modify RefFileName such that it ends on _bb.cmd
      set RefFileName [string replace $RefFileName $len-4 $len-4 "_bb."]
   }

   while {[lsearch $t_done to_do] >= 0} {
      for {set i 0} {$i < [llength $t_name]} {incr i} {
         if {[lindex $t_done $i] == "done"} {
            continue
         }
         set idx [lsearch $t_name [lindex $t_new $i]]
         if {[lindex $t_done $idx] == "to_do"} {
            continue
         }
         set sp "1.0"
         while {1} {
            set sp [.mkref.txt search -count cc [lindex $t_name $i] $sp end]
            if {$sp == ""} {
               break
            }
            set ws [.mkref.txt index "$sp wordstart"]
            set we [.mkref.txt index "$sp wordend"]
            if {($ws == $sp) && ($we == [.mkref.txt index "$sp+${cc}c"])} {
               .mkref.txt delete $ws $we
               .mkref.txt insert $ws [lindex $t_new $i]
            }
            set sp $we
         }
         set t_done [lreplace $t_done $i $i "done"]
      }
   }
   .mkref.sl configure -text "Now editing file '$RefFileName' for bondbar"
   update
}

proc do_rd_reffile {ref_file} {
#-----------------------------------------------------------------------------#
# procedure to read a reference_file                                          #
#-----------------------------------------------------------------------------#
   global RefFileName

   .mkref.sl configure -text "reading file '$ref_file' ....."
   update
   .mkref.txt delete 1.0 end
   set RefFileName $ref_file
   set f_ref [open $RefFileName]
   .mkref.txt insert end [read $f_ref]
   close $f_ref
   .mkref.sl configure -text "file '$RefFileName' read"
   .mkref.fr.file.cmds entryconfigure "UpdateForBondbar" -state normal
   .mkref.fr.file.cmds entryconfigure "Save" -state normal
   .mkref.fr.file.cmds entryconfigure "Save as ..." -state normal
   .mkref.fr.file.cmds entryconfigure "Save as type res" -state normal
   .mkref.fr.file.cmds entryconfigure "Save as type res as ..." -state normal

   update
}
