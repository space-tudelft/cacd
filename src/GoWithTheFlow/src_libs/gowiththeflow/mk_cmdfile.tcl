
proc mk_cmdfile {} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the command file can be made      #
#-----------------------------------------------------------------------------#
   global CmdFileName Fnt MaxNbrMenuItemsInColumn

   set CmdFileName ""

   upd_status_line "mk_cmd_file: BUSY"
   toplevel   .mkcmd
   frame      .mkcmd.fr -relief raised -bd 2 -bg gold
   label      .mkcmd.sl -relief sunk -bd 2 -bg wheat3 -text ""\
                        -font $Fnt -anchor w -padx 10 -pady 5
   text       .mkcmd.txt -width 80 -height 25 -font $Fnt\
                         -yscrollcommand ".mkcmd.sb set"\
                         -bg wheat -padx 10
   scrollbar  .mkcmd.sb      -command ".mkcmd.txt yview" -bg wheat
   menubutton .mkcmd.fr.file -text "File" -font $Fnt\
                             -menu .mkcmd.fr.file.cmds -bg gold
   menu       .mkcmd.fr.file.cmds -font $Fnt -bg wheat3
              .mkcmd.fr.file.cmds add cascade -label "Read"\
                                              -menu .mkcmd.fr.file.cmds.read
              .mkcmd.fr.file.cmds add cascade -label "Generate from"\
                                              -menu .mkcmd.fr.file.cmds.gen
              .mkcmd.fr.file.cmds add cascade -label "UpdateForBondbar" -state disabled\
                                              -menu .mkcmd.fr.file.cmds.bbar
              .mkcmd.fr.file.cmds add command -label "Save" -state disabled\
                                              -command "do_write_cmd 0"
              .mkcmd.fr.file.cmds add command -label "Save as ..." -state disabled\
                                              -command "do_write_cmd 1"
              .mkcmd.fr.file.cmds add command -label "Quit"\
                                              -command {destroy .mkcmd; upd_status_line ""}
   menu       .mkcmd.fr.file.cmds.read -font $Fnt -bg wheat3 -tearoff 0
   menu       .mkcmd.fr.file.cmds.gen  -font $Fnt -bg wheat3 -tearoff 0
   menu       .mkcmd.fr.file.cmds.bbar -font $Fnt -bg wheat3 -tearoff 0

   set lst_files [lsort [glob -nocomplain "*.lst"]]
   for {set i 0} {$i < [llength $lst_files]} {incr i} {
      set i_lst [lindex $lst_files $i]
      .mkcmd.fr.file.cmds.gen add command -label $i_lst -command "do_mk_cmdfile $i_lst 1"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .mkcmd.fr.file.cmds.gen entryconfigure $i_lst -columnbreak 1
      }
   }

   set ref_files [lsort [glob -nocomplain "*.ref"]]
   for {set i 0} {$i < [llength $ref_files]} {incr i} {
      set i_ref [lindex $ref_files $i]
      .mkcmd.fr.file.cmds.gen add command -label $i_ref -command "do_mk_cmdfile $i_ref 0"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .mkcmd.fr.file.cmds.gen entryconfigure $i_ref -columnbreak 1
      }
   }

   set cmd_files [lsort [glob -nocomplain "*.cmd"]]
   for {set i 0} {$i < [llength $cmd_files]} {incr i} {
      set i_cmd [lindex $cmd_files $i]
      .mkcmd.fr.file.cmds.read add command -label $i_cmd -command "do_rd_cmdfile $i_cmd"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .mkcmd.fr.file.cmds.read entryconfigure $i_cmd -columnbreak 1
      }
   }

   set buf_files [glob -nocomplain "*.buf"]
   foreach i_buf $buf_files {
      .mkcmd.fr.file.cmds.bbar add command -label $i_buf -command "update_for_bondbar $i_buf"
   }

   pack .mkcmd.fr      -side top -fill x
   pack .mkcmd.sl      -side bottom -fill x
   pack .mkcmd.fr.file -side left
   pack .mkcmd.sb      -side right -fill y
   pack .mkcmd.txt     -side left -fill both -expand 1
}

proc conv {chr} {
#-----------------------------------------------------------------------------#
# procedure to do a character conversion                                      #
#-----------------------------------------------------------------------------#
   switch $chr {
      D {return l}
      L {return l}
      U {return h}
      H {return h}
   }
   return x
}

proc add_to_terms {name first last type nbr} {
#-----------------------------------------------------------------------------#
# procedure to fill the var and type arrays with the correct data             #
#-----------------------------------------------------------------------------#
   upvar var_arr var_arr
   upvar type_arr type_arr

   if {$name == ""} { return $nbr }

   if {$first == -1 || $last == -1} {
      set var_arr($nbr) $name
      set type_arr($nbr) $type
      incr nbr
   } elseif {$last > $first} {
      for {set i $first} {$i <= $last} {incr i} {
         set var_arr($nbr) "$name\_$first\_$last\_$i"
         set type_arr($nbr) $type
         incr nbr
      }
   } else {
      for {set i $first} {$i >= $last} {incr i -1} {
         set var_arr($nbr) "$name\_$first\_$last\_$i"
         set type_arr($nbr) $type
         incr nbr
      }
   }
   return $nbr
}

proc get_reduced_time {t_str red_fact} {
#-----------------------------------------------------------------------------#
# procedure to determine the time reduced bij the factor rad_fact which       #
# should be an integer                                                        #
#-----------------------------------------------------------------------------#
   if {$t_str == "0"} { return "0" }

   set slen [expr [string length $t_str] - $red_fact - 1]
   if {$slen < 0} {
      return "0"
   } else {
      return [string range $t_str 0 $slen]
   }
}

proc do_mk_cmdfile {file_name is_lst} {
#-----------------------------------------------------------------------------#
# procedure to generate the command_file                                      #
#-----------------------------------------------------------------------------#
   global CmdFileName

   .mkcmd.sl configure -text "generating cmd_file from file $file_name"
   update
   set CmdFileName "[lindex [split $file_name .] 0].cmd"
   set t_new 10
   .mkcmd.txt delete 1.0 end
   set nt 0
   set term_name ""
   set term_first -1
   set term_last -1
   set term_type "I"
   if {$is_lst == 1} {
      set def_file_name "[lindex [split $file_name .] 0].def"
      set f_def [open $def_file_name]
      while {![eof $f_def]} {
         set tt [gets $f_def]
         if {$tt == ""} {
            continue
         }
         set idx [expr [string last / [lindex $tt 0]] +1]
         set term [string range [lindex $tt 0] $idx end]
         set tmp [split $term "\(\)"]
         if {[lindex $tmp 0] != $term_name} {
            set nt [add_to_terms $term_name $term_first $term_last $term_type $nt]
            set term_name [lindex $tmp 0]
            if {[llength $tt] > 3} {
               set term_type  "IO"
            } else {
               set term_type [lindex $tt 2]
            }
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
      set nt [add_to_terms $term_name $term_first $term_last $term_type $nt]
      set time_unit_ind 3
   }\
   else {
      set f_ref [open $file_name]
      while {![eof $f_ref]} {
         set tt [gets $f_ref]
         if {$tt == "" || [string index $tt 0] == "#"} {
            continue
         }
         set l [split $tt " ;#"]
         if {[lindex $l 0] == "INP"} {
            set nt [add_to_terms [lindex $l 1] -1 -1 "I" $nt]
         }\
         elseif {[lindex $l 0] == "OUT"} {
            set nt [add_to_terms [lindex $l 1] -1 -1 "O" $nt]
         }\
         elseif {[lindex $l 0] == "INOUT"} {
            set nt [add_to_terms [lindex $l 1] -1 -1 "IO" $nt]
         }\
         elseif {[lindex $l 0] == "TIME_UNIT"} {
            set time_unit [regsub "s" [lindex $l 1] ""]
            set time_unit_ind [expr round(log10([expr $time_unit]/[expr 1.0e-12]))]
         }
      }
      close $f_ref
   }
   set first_val "True"
   set f [open $file_name]
   set min_nbr_zeros 100
   set max_time_str_length 0
   while {![eof $f]} {
      set tt [gets $f]
      if {$tt == "" || [string index $tt 0] != " "} {
         continue
      }
      regsub "<" $tt " " tt
      if {[lindex $tt 0] == 0} {
         continue
      }
      set strlen [string length [lindex $tt 0]]
      if {$strlen > $max_time_str_length} {
         set max_time_str_length $strlen
      }
      set red_len [string length [string trimright [lindex $tt 0] "0"]]
      set nbr_zeros [expr $strlen -$red_len]
      if {$nbr_zeros < $min_nbr_zeros} {
         set min_nbr_zeros $nbr_zeros
      }
   }
   if {$max_time_str_length - $min_nbr_zeros > 8} {
      set rf [expr $max_time_str_length - $min_nbr_zeros - 8]
   } elseif {$time_unit_ind + $min_nbr_zeros <= 3} {
      set rf $min_nbr_zeros
   } else {
      set rf 0
   }
   incr time_unit_ind $rf
   seek $f 0
   while {![eof $f]} {
      set tt [gets $f]
      if {$tt == "" || [string index $tt 0] != " "} {
         continue
      }
      regsub "<" $tt " " tt
      if {$first_val == "True"} {
         set first_val "False"
         set v_new [lindex $tt 1]
         for {set i 0} {$i < $nt} {incr i} {
            if {$type_arr($i) == "I"} {
               set chr [conv [string index $v_new $i]]
               append val($i) "$chr"
#               set t_prev($i) [expr double([lindex $tt 0].0)]
               set t_prev($i) [get_reduced_time [lindex $tt 0] $rf]
            }
         }
      }\
      else {
         set v_prev $v_new
#         set t_new [expr double([lindex $tt 0].0)]
         set t_new [get_reduced_time [lindex $tt 0] $rf]
         set v_new [lindex $tt 1]
         for {set i 0} {$i < $nt} {incr i} {
            if {($type_arr($i) == "I") &&\
                ([string index $v_new $i] != [string index $v_prev $i])} {
                set diff [expr $t_new -$t_prev($i)]
                set diff [lindex [split $diff .] 0]
               append val($i) "*$diff"
               set chr [conv [string index $v_new $i]]
               append val($i) " $chr"
               set t_prev($i) $t_new
            }
         }
      }
   }
   close $f
   .mkcmd.txt insert end "set vdd = h*~\n"
   .mkcmd.txt insert end "set vss = l*~\n"
   for {set i 0} {$i < $nt} {incr i} {
      if {$type_arr($i) == "I"} {
         set sp_str "\\\n"
         .mkcmd.txt insert end "set $var_arr($i) ="
         set eql_pos [lindex [split [.mkcmd.txt index "end-1c"] .] 1]
         if {$eql_pos > 0} {
            while {$eql_pos > 0} {
               append sp_str " "
               incr eql_pos -1
            }
         }
         set nts [llength $val($i)]
         set ii 0
         set ns 0
         while {$ii < $nts-2} {
            set ref1 [lindex $val($i) $ii]
            set ref2 [lindex $val($i) $ii+1]
            if {[lindex $val($i) $ii+2] == $ref1 &&\
                [lindex $val($i) $ii+3] == $ref2} {
               incr ii 2
               incr ns
               continue
            }
            if {$ns == 0} {
               .mkcmd.txt insert end " [lindex $val($i) $ii]$sp_str"
               incr ii
            } else {
               .mkcmd.txt insert end " ($ref1 $ref2)*[expr $ns+1]$sp_str"
               incr ii 2
               set ns 0
            }
         }
         if {$ii == $nts - 2} {
            .mkcmd.txt insert end " [lindex $val($i) $ii]"
            incr ii
         }
         if {$ii == $nts - 1} {
            .mkcmd.txt insert end " [lindex $val($i) $ii]*~\n"
         }
      }
   }
   switch $time_unit_ind {
   1 { .mkcmd.txt insert end "\noption sigunit   = 10p"
       .mkcmd.txt insert end "\noption outacc    = 1p"
     }
   2 { .mkcmd.txt insert end "\noption sigunit   = 100p"
       .mkcmd.txt insert end "\noption outacc    = 10p"
     }
   3 { .mkcmd.txt insert end "\noption sigunit   = 1n"
       .mkcmd.txt insert end "\noption outacc    = 100p"
     }
   4 { .mkcmd.txt insert end "\noption sigunit   = 10n"
       .mkcmd.txt insert end "\noption outacc    = 1n"
     }
   5 { .mkcmd.txt insert end "\noption sigunit   = 100n"
       .mkcmd.txt insert end "\noption outacc    = 10n"
     }
   6 { .mkcmd.txt insert end "\noption sigunit   = 1u"
       .mkcmd.txt insert end "\noption outacc    = 100n"
     }
   7 { .mkcmd.txt insert end "\noption sigunit   = 10u"
       .mkcmd.txt insert end "\noption outacc    = 1u"
     }
   8 { .mkcmd.txt insert end "\noption sigunit   = 100u"
       .mkcmd.txt insert end "\noption outacc    = 10u"
     }
   }
   .mkcmd.txt insert end "\noption level     = 3"
   .mkcmd.txt insert end "\noption initialize random = on"
   set t_new_rnd [lindex [split $t_new .] 0]
   .mkcmd.txt insert end "\noption simperiod = $t_new_rnd\n"
   .mkcmd.txt insert end "\nprint \\\n"
   for {set i 0} {$i < $nt-1} {incr i} {
      .mkcmd.txt insert end "      $var_arr($i),\\\n"
   }
   .mkcmd.txt insert end "      $var_arr($i)"
   .mkcmd.txt insert end "\n"
   .mkcmd.txt insert end "\nplot \\\n"
   for {set i 0} {$i < $nt-1} {incr i} {
      .mkcmd.txt insert end "      $var_arr($i),\\\n"
   }
   .mkcmd.txt insert end "      $var_arr($i)"
   .mkcmd.txt insert end "\n"
   .mkcmd.txt insert end "/*\n"
   .mkcmd.txt insert end "*%\n"
   .mkcmd.txt insert end "tstep 0.1n\n"
   .mkcmd.txt insert end "trise 0.5n\n"
   .mkcmd.txt insert end "tfall 0.5n\n"
   .mkcmd.txt insert end "*%\n"
   .mkcmd.txt insert end ".options cptime = 1000\n"
   .mkcmd.txt insert end "*%\n"
   .mkcmd.txt insert end "*/\n"
   set nl [lindex [split [.mkcmd.txt index end] .] 0]
   if {$min_nbr_zeros < $rf} {
      .mkcmd.sl configure\
          -text "generation of cmd_data from file '$file_name' ready\n\
                 ***** input_time(s) have been trunctuated *****"
   }\
   else {
      .mkcmd.sl configure\
          -text "generation of cmd_data from file '$file_name' ready"
   }
   .mkcmd.fr.file.cmds entryconfigure "UpdateForBondbar" -state normal
   .mkcmd.fr.file.cmds entryconfigure "Save" -state normal
   .mkcmd.fr.file.cmds entryconfigure "Save as ..." -state normal
   update
}

proc do_write_cmd {askfilename} {
#-----------------------------------------------------------------------------#
# procedure to write the command_file to a file                               #
#-----------------------------------------------------------------------------#
   global CmdFileName

   if {$CmdFileName == ""} {
      .mkcmd.sl configure -text "First read or generate a command file!"
      return
   }

   if {$askfilename} {
      set types {
           {{CMD Files} {.cmd}}
           {{All files}  *    }
      }
      set name [tk_getSaveFile -initialdir . -initialfile $CmdFileName -filetypes $types]
      if {$name == ""} return
   } else {
      set name $CmdFileName
   }

   .mkcmd.sl configure -text "Saving file '$name' ....."
   update
   set fpo [open $name w]
   puts $fpo [.mkcmd.txt get 1.0 end]
   close $fpo
   .mkcmd.sl configure -text "File '$name' (over)written"
   update
}

proc update_for_bondbar {buf_file_name} {
#-----------------------------------------------------------------------------#
# procedure to change the terminal_names to the terminal_names of the bondbar #
#-----------------------------------------------------------------------------#
   global CmdFileName

   if {$CmdFileName == ""} {
      .mkcmd.sl configure -text "First read or generate a command file!"
      return
   }

   .mkcmd.sl configure -text "updating for bondbar using file '$buf_file_name'"
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

   set len [string length $CmdFileName]
   if {[string last "_bb.cmd" $CmdFileName] != $len-7} {
      # We modify CmdFileName such that it ends on _bb.cmd
      set CmdFileName [string replace $CmdFileName $len-4 $len-4 "_bb."]
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
            set sp [.mkcmd.txt search -count cc [lindex $t_name $i] $sp end]
            if {$sp == ""} {
               break
            }
            set ws [.mkcmd.txt index "$sp wordstart"]
            set we [.mkcmd.txt index "$sp wordend"]
            if {($ws == $sp) && ($we == [.mkcmd.txt index "$sp+${cc}c"])} {
               .mkcmd.txt delete $ws $we
               .mkcmd.txt insert $ws [lindex $t_new $i]
            }
            set sp $we
         }
         set t_done [lreplace $t_done $i $i "done"]
      }
   }
   .mkcmd.sl configure -text "Now editing file '$CmdFileName' for bondbar"
   update
}

proc do_rd_cmdfile {cmd_file} {
#-----------------------------------------------------------------------------#
# procedure to read a command_file                                            #
#-----------------------------------------------------------------------------#
   global CmdFileName

   .mkcmd.sl configure -text "reading file '$cmd_file' ....."
   update
   .mkcmd.txt delete 1.0 end
   set CmdFileName $cmd_file

   set f_cmd [open $CmdFileName]
   .mkcmd.txt insert end [read $f_cmd]
   close $f_cmd

   .mkcmd.sl configure -text "file '$CmdFileName' read"
   .mkcmd.fr.file.cmds entryconfigure "UpdateForBondbar" -state normal
   .mkcmd.fr.file.cmds entryconfigure "Save" -state normal
   .mkcmd.fr.file.cmds entryconfigure "Save as ..." -state normal
   update
}
