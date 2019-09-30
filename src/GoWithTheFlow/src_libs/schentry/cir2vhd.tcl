
# procedure to generate a window for the generation of the vhdl_code
# for the circuit shown
#-------------------------------------------------------------------
proc vhdl_conv {} {
   global Fnt CirName

   if {$CirName == ""} { return }
   if {[winfo exists .conv]} { destroy .conv }

# see if the picture shown is already written
  if {[see_circ_change] == 1} {
      set answ [tk_messageBox -icon error -type yesno\
         -message "Data changed:\
                   must it be (re)written for the generation of the vhdl_code?"]
      if {$answ == "yes"} { write_cir_file $CirName }
   }

   toplevel .conv
   frame  .conv.arch       -bd 2 -relief raised -bg wheat
   frame  .conv.cmd        -bd 0 -relief raised -bg gold
   label  .conv.arch.nm    -text "architecture name:" -bg wheat -font $Fnt
   entry  .conv.arch.en    -width 15 -bg wheat -font $Fnt
   button .conv.cmd.doit   -text "OK"     -bg gold -width 9 -font $Fnt -command "cir2vhd"
   button .conv.cmd.cancel -text "Cancel" -bg gold -width 9 -font $Fnt -command "destroy .conv"

   pack .conv.arch       -side top -fill x
   pack .conv.cmd        -side bottom -fill x
   pack .conv.arch.nm    -side left -pady 5
   pack .conv.arch.en    -side left -padx 5 -pady 5
   pack .conv.cmd.doit   -side left -padx 5 -pady 5
   pack .conv.cmd.cancel -side right -padx 5 -pady 5
   wm title .conv "make VHDL"

   .conv.arch.en insert end "circuit"
   bind .conv <KeyPress-Return> { cir2vhd }
   bind .conv <KeyPress-Escape> { destroy .conv }
}

# procedure to generate a vhdl signal_statement from net_nbr and sig_str
#-----------------------------------------------------------------------
proc mk_signal_string {net_nbr sig_str} {
   set t_data [split $sig_str "(:)"]
   if {[llength $t_data] == 1} {
      return "   signal net_$net_nbr: std_logic;"
   } else {
      set rh [lindex $t_data 1]
      set rl [lindex $t_data 2]
      return "   signal net_$net_nbr: std_logic_vector($rh downto $rl);"
   }
}

# procedure to generate the vhdl code for the circuit shown
#----------------------------------------------------------
proc cir2vhd {} {
   global CirName CircuitPath ComponentPath VhdlPath i_arr p_arr s_arr m_arr c_arr ni np nc ns nm
   global MAIN_PORT S_FULL_PORT S_PART_PORT COMP_PORT show_refs
   global EntFileName
   global ArchFileName
   global Fnt EditFnt
   global Textindent

# read the libcells file to determine if a component stemms from a library
   set fp [open $ComponentPath/libcells "r"]
   set Libcellslist ""
   while {![eof $fp]} {
	if {[set txt [gets $fp]] != ""} { lappend Libcellslist $txt }
   }
   close $fp

   set VSS_FOUND "False"
   set arch_name "[.conv.arch.en get]"
   if {$arch_name == ""} {
	show_mess "No architecture name given!"
	raise .conv
	return
   }
   if {![regexp {^[A-Za-z][A-Za-z0-9_-]*$} $arch_name]} {
	show_mess "No correct architecture name!\nUse another name!"
	raise .conv
	return
   }
   if {![file exists $VhdlPath]} { file mkdir $VhdlPath }

   set EntFileName "$VhdlPath/$CirName.vhd"
   set re1 ""
   if {[file exist $EntFileName]} {
      file copy -force $EntFileName $EntFileName\_
      set re1 "re"
   }
   set ArchFileName "$VhdlPath/$CirName-$arch_name.vhd"
   set re2 ""
   if {[file exist $ArchFileName]} {
      file copy -force $ArchFileName $ArchFileName\_
      set re2 "re"
   }

   set f_cir [open $CircuitPath/$CirName.cir "r"]
   set f_ent [open $EntFileName "w"]
   set f_arch [open $ArchFileName "w"]
   set np -1
   set ni -1
   set nc -1
   set ns -1
   set nm -1
   set nl -1
   while {![eof $f_cir]} {
      set txt [gets $f_cir]
      if {[lindex $txt 0] == "T"} {
         if {[llength $txt] > 1} {
            incr np
            set p_arr($np) "[lindex $txt 1] [lindex $txt 2]"
         }
      } elseif {[lindex $txt 0] == "I"} {
         if {[llength $txt] > 1} {
            incr ni
            set i_arr($ni) [lindex $txt 1]
         }
      } elseif {[lindex $txt 0] == "C"} {
         if {[llength $txt] > 1} {
            incr nc
            set c_arr($nc) [lindex $txt 1]
         }
      } elseif {[lindex $txt 0] == "S"} {
         incr ns
         if {[llength $txt] > 1} {
            set s_arr($ns) [lindex $txt 1]
         } else {
            set s_arr($ns) ""
         }
      } elseif {[lindex $txt 0] == "M"} {
         incr nm
         if {[llength $txt] > 1} {
            set m_arr($nm) [lindex $txt 1]
         } else {
            set m_arr($nm) ""
         }
      } elseif {[lindex $txt 0] == "L"} {
         if {[llength $txt] > 1} {
            incr nl
            set l_arr($nl) "[lindex $txt 1] [lindex $txt 2]"
         }
      }
   }
   close $f_cir

# make the entity
   puts $f_ent "library IEEE;"
   puts $f_ent "use IEEE.std_logic_1164.ALL;\n"
   puts $f_ent "entity $CirName is"
   for {set i 0} {$i <= $np} {incr i} {
      set type [lindex $p_arr($i) 0]
      if {[string index $type 0] == "*"} {
         set type "inout"
      }
      set name [split [lindex $p_arr($i) 1] "(:)"]
      if {[llength $name] == 1} {
         set p_string "$name:$type std_logic"
      } else {
         set rh [lindex $name 1]
         set rl [lindex $name 2]
         set name [lindex $name 0]
         set p_string "$name:$type std_logic_vector($rh downto $rl)"
      }
      if {$i == 0} {
         puts $f_ent "   port ($p_string;"
      } elseif {$i == $np} {
         puts $f_ent "         $p_string);"
      } else {
         puts $f_ent "         $p_string;"
      }
   }
   puts $f_ent "end $CirName;\n"

# start the architecture generation
   puts $f_arch "library IEEE;"
   puts $f_arch "use IEEE.std_logic_1164.ALL;\n"

# see if library_components are present
   for {set i 0} {$i <= $ni} {incr i} {
      set cmp_name [lindex [split $i_arr($i) "~"] 0]
      if {[lsearch -exact $Libcellslist $cmp_name] >= 0} {
         puts $f_arch "library CellsLib;"
         break
      }
   }
   puts $f_arch "architecture $arch_name of $CirName is"

# add the component definitions
# also add the names of the component ports to the contents of i_arr
   set cmp_list ""
   for {set i 0} {$i <= $ni} {incr i} {
      set cmp_name [lindex [split $i_arr($i) "~"] 0]
      if {[lsearch -exact $cmp_list $cmp_name] >= 0} {
         for {set ii 0} {$ii < $i} {incr ii} {
            if {[lindex [split $i_arr($ii) "~"] 0] == $cmp_name} {
               for {set j 1} {$j < [llength $i_arr($ii)]} {incr j} {
                  lappend i_arr($i) [lindex $i_arr($ii) $j]
               }
               break
            }
         }
         continue
      }
      puts $f_arch "   component $cmp_name"
      set f_cmp [open $ComponentPath/$cmp_name.cmp "r"]
      set first_port "True"
      while {![eof $f_cmp]} {
         set txt [gets $f_cmp]
         if {([lindex $txt 0] == "port") && ([llength $txt] == 3)} {
            set type [lindex $txt 1]
            if {[string index $type 0] == "*"} {
               set type "inout"
            }
            set t_name [lindex $txt 2]
            set name [split $t_name "(:)"]
            if {[llength $name] == 1} {
               set p_string "$name:$type std_logic"
            } else {
               set rh [lindex $name 1]
               set rl [lindex $name 2]
               set name [lindex $name 0]
               set p_string "$name:$type std_logic_vector($rh downto $rl)"
            }
            lappend i_arr($i) $t_name
            if {$first_port == "True"} {
               puts -nonewline $f_arch "      port ($p_string"
               set first_port "False"
            } else {
               puts -nonewline $f_arch ";\n            $p_string"
            }
         }
      }
      puts $f_arch ");\n   end component;"
      lappend cmp_list $cmp_name
      close $f_cmp
   }

# add the signal definitions and install the connections to the components
# first from the labels
   set label_list ""
   set net_list ""
   set ref_list ""
   for {set i 0} {$i <= $nl} {incr i} {
      set l_name [lindex $l_arr($i) 0]
      set name [split $l_name "(:)"]
      if {[lsearch -exact $label_list $l_name] == -1} {
         lappend label_list "$l_name"
         if {"$CirName.$l_name" != [lindex $l_arr($i) 1]} {
            if {[llength $name] == 1} {
               puts $f_arch "   signal $l_name: std_logic;"
            } else {
               set rh [lindex $name 1]
               set rl [lindex $name 2]
               set name [lindex $name 0]
               set p_string "   signal $name: std_logic_vector($rh downto $rl);"
               puts $f_arch "$p_string"
            }
         }
      }
      set port_name [lindex $l_arr($i) 1]
      lappend ref_list [lindex $name 0]
      lappend net_list $port_name
   }

# then from the connections
# first determine the nets from the outputs
# make references and signals from the main_ports
   set n_net -1
   for {set i 0} {$i <= $nc} {incr i} {
      set from_port [lindex [split $c_arr($i) ">"] 0]
      set to_port   [lindex [split $c_arr($i) ">"] 1]
      set from_cell [lindex [split $from_port .] 0]
      set to_cell   [lindex [split $to_port .] 0]
      set from_term [lindex [split $from_port .] 1]
      set to_term   [lindex [split $to_port .] 1]
      set ref_name [lindex [split $from_term "(:)"] 0]
      set inp_type  [get_input_port_type $from_cell $CirName]
      set outp_type [get_output_port_type $to_cell $CirName]
      if {$inp_type == $MAIN_PORT} {
         if {[lsearch $net_list $from_port] == -1} {
            lappend net_list $from_port
            lappend ref_list $ref_name
         }
      }
      if {$outp_type == $MAIN_PORT} {
         if {[lsearch $net_list $to_port] == -1} {
            if {$inp_type == $MAIN_PORT && [lsearch $net_list $from_port] >= 0} {
               lappend net_list $to_port
               lappend ref_list $ref_name
            } else {
               lappend net_list $to_port
               incr n_net
               lappend ref_list "net_$n_net"
               set signal_str [mk_signal_string $n_net $to_term]
               puts $f_arch $signal_str
               if {$inp_type == $COMP_PORT} {
                  lappend net_list $from_port
                  lappend ref_list "net_$n_net"
               }
            }
         }
      }
   }

# make references and signals from the component output_ports
   for {set i 0} {$i <= $nc} {incr i} {
      set from_port [lindex [split $c_arr($i) ">"] 0]
      set from_cell [lindex [split $from_port .] 0]
      set from_term [lindex [split $from_port .] 1]
      set to_port   [lindex [split $c_arr($i) ">"] 1]
      set to_cell   [lindex [split $to_port .] 0]
      set to_term   [lindex [split $to_port .] 1]
      set inp_type  [get_input_port_type $from_cell $CirName]
      if {$inp_type == $COMP_PORT} {
         if {[lsearch $net_list $from_port] == -1} {
            lappend net_list $from_port
            set idx [lsearch $net_list $to_port]
            if {$idx == -1} {
               incr n_net
               lappend ref_list "net_$n_net"
               set signal_str [mk_signal_string $n_net $from_term]
               puts $f_arch $signal_str
            } else {
               lappend ref_list [lindex $ref_list $idx]
            }
         }
      }
   }

# make references and signals from the component input_ports
   for {set i 0} {$i <= $nc} {incr i} {
      set from_port [lindex [split $c_arr($i) ">"] 0]
      set from_cell [lindex [split $from_port .] 0]
      set from_term [lindex [split $from_port .] 1]
      set to_port   [lindex [split $c_arr($i) ">"] 1]
      set to_cell   [lindex [split $to_port .] 0]
      set to_term   [lindex [split $to_port .] 1]
      set outp_type [get_output_port_type $to_cell $CirName]
      if {$outp_type == $COMP_PORT} {
         if {[lsearch $net_list $to_port] == -1} {
            lappend net_list $to_port
            set idx [lsearch $net_list $from_port]
            if {$idx == -1} {
               incr n_net
               lappend ref_list "net_$n_net"
               lappend net_list $from_port
               lappend ref_list "net_$n_net"
               set signal_str [mk_signal_string $n_net $to_term]
               puts $f_arch $signal_str
            } else {
               lappend ref_list [lindex $ref_list $idx]
            }
         }
      }
   }
# make references and signals from splitters
   for {set i 0} {$i <= $nc} {incr i} {
      set from_port [lindex [split $c_arr($i) ">"] 0]
      set from_cell [lindex [split $from_port .] 0]
      set from_term [lindex [split $from_port .] 1]
      set to_port   [lindex [split $c_arr($i) ">"] 1]
      set to_cell   [lindex [split $to_port .] 0]
      set to_term   [lindex [split $to_port .] 1]
      set inp_type  [get_input_port_type $from_cell $CirName]
      set outp_type [get_output_port_type $to_cell $CirName]
      if {($outp_type == $S_PART_PORT) || ($outp_type == $S_FULL_PORT)} {
         if {[lsearch $net_list $to_port] == -1} {
            lappend net_list $to_port
            set idx [lsearch $net_list $from_port]
            if {$idx == -1} {
               incr n_net
               lappend ref_list "net_$n_net"
               if {($inp_type == $S_PART_PORT) || ($inp_type == $S_FULL_PORT)} {
                  lappend net_list $from_port
                  lappend ref_list "net_$n_net"
                  set dw [get_data_width $from_port]
                  if {$dw == 1} {
                     puts $f_arch "   signal net_$n_net: std_logic;"
                  } else {
                     puts $f_arch "   signal net_$n_net:\
                                   std_logic_vector([expr $dw-1] downto 0);"
                  }
               } else {
                  set signal_str [mk_signal_string $n_net $to_term]
                  puts $f_arch $signal_str
               }
            } else {
               lappend ref_list [lindex $ref_list $idx]
            }
         }
      } elseif {($inp_type == $S_PART_PORT) || ($inp_type == $S_FULL_PORT)} {
         if {[lsearch $net_list $from_port] == -1} {
            lappend net_list $from_port
            set idx [lsearch $net_list $to_port]
            if {$idx == -1} {
               incr n_net
               lappend ref_list net_$n_net
               set signal_str [mk_signal_string $n_net $from_term]
               puts $f_arch $signal_str
            } else {
               lappend ref_list [lindex $ref_list $idx]
            }
         }
      }
   }
   if {$show_refs == 1} {
      show_references $net_list $ref_list
   }

# generate the begin keyword
   puts $f_arch "begin"

# generate the connections of the nets to the main ports
   for {set i 0} {$i <= $np} {incr i} {
      set p_name [lindex $p_arr($i) 1]
      set pp_name [lindex [split $p_name "(:)"] 0]
      set idx [lsearch $net_list $CirName.$p_name]
      if {$idx >= 0} {
         set r_name [lindex $ref_list $idx]
         if {$pp_name != $r_name} {
            set type [lindex $p_arr($i) 0]
            if {$type == "out"} {
               puts $f_arch "   $pp_name <= $r_name; "
            } else {
               puts $f_arch "   $r_name <= $pp_name; "
            }
         }
      }
   }

# generate the equivalence statements from the splitters
   for {set i 0} {$i <= $ns} {incr i} {
      if {$s_arr($i) != ""} {
         set idi [lsearch $net_list S-$i.in]
         if {$idi < 0} {
            continue
         }
         set in_signal [lindex $ref_list $idi]
         set ido [lsearch $net_list S-$i.out]
         if {$ido < 0} {
            continue
         }
         set out_signal [lindex $ref_list $ido]
         set range_txt [lindex $s_arr($i) 0]
         set rhl [split [string trim $range_txt "<>"] ":"]
         set rh [lindex $rhl 0]
         set rl [lindex $rhl 1]
         if {$rh == $rl} {
            set range_str "($rh)"
         } else {
            set range_str "($rh downto $rl)"
         }
         if {[string index $range_txt 0] == ">"} {
            puts $f_arch "   $out_signal <= $in_signal$range_str;"
         } else {
            puts $f_arch "   $out_signal$range_str <= $in_signal;"
         }
      }
   }

# generate the equivalence statements from the mergers
   for {set i 0} {$i <= $nm} {incr i} {
      if {$m_arr($i) != ""} {
         set range_txt [lindex $m_arr($i) 0]
         set rhl [split [string trim $range_txt "<>"] ":"]
         set rh [lindex $rhl 0]
         set rl [lindex $rhl 1]
         for {set j 0} {$j <= $rh} {incr j} {
            if {[string first ">" $range_txt 0] >= 0} {
               set idi [lsearch $net_list M-$i.i]
            } else {
               set idi [lsearch $net_list M-$i.i$j]
            }
            if {$idi >= 0} {
               set in_signal [lindex $ref_list $idi]
            }

            if {[string first "<" $range_txt 0] >= 0} {
               set ido [lsearch $net_list M-$i.o]
            } else {
               set ido [lsearch $net_list M-$i.o$j]
            }
            if {$ido >= 0} {
               set out_signal [lindex $ref_list $ido]
            }

            if {$idi >= 0 && $ido >= 0} {
               set range_str "($j)"
               if {[string index $range_txt 0] == ">"} {
                  puts $f_arch "   $out_signal <= $in_signal$range_str;"
               } else {
                  puts $f_arch "   $out_signal$range_str <= $in_signal;"
               }
            }
         }
      }
   }

# generate the assign_statements from the vss, vdd and gnd labels
   set vdd_done "False"
   set vss_done "False"
   set gnd_done "False"
   for {set i 0} {$i <= $nl} {incr i} {
      set l_name [lindex $l_arr($i) 0]
      if {([string tolower $l_name] == "vdd") && ($vdd_done == "False")} {
         puts $f_arch "   vdd <= '1';"
         set vdd_done "True"
      } elseif {([string tolower $l_name] == "vss") && ($vss_done == "False")} {
         puts $f_arch "   vss <= '0';"
         set vss_done "True"
      } elseif {([string tolower $l_name] == "gnd") && ($gnd_done == "False")} {
         puts $f_arch "   gnd <= '0';"
         set gnd_done "True"
      }
   }

# generate the instantiation of the components
   for {set i 0} {$i <= $ni} {incr i} {
      set tmp [lindex $i_arr($i) 0]
      set c_name [lindex [split $tmp "~"] 0]
      set c_nbr  [lindex [split $tmp "~"] 1]
      set init_str "   ${c_name}_${c_nbr} : $c_name port map ("
      set indent_len [string length $init_str]
      set cidx $indent_len
      set space_str ""
      for {set isp 0} {$isp < $cidx} {incr isp} {
         append space_str " "
      }
      puts -nonewline $f_arch "$init_str"
      set last [expr [llength $i_arr($i)]- 1]
      for {set ii 1} {$ii <= $last} {incr ii} {
         set c_str "[lindex $i_arr($i) $ii]"
         set idx [lsearch $net_list $tmp.$c_str]
         set pr_str "[lindex [split $c_str "(:)" ] 0]"
         if {$idx < 0} {
            append pr_str " => OPEN"
         } else {
            append pr_str " => [lindex $ref_list $idx]"
         }
         set len [string length $pr_str]
         set cidx [expr $cidx + $len + 2]
         if {$cidx < 78} {
            if {$ii == $last} {
               puts $f_arch "$pr_str);"
            } else {
               puts -nonewline $f_arch "$pr_str, "
            }
         } else {
            set cidx [expr $indent_len + $len]
            if {$ii == $last} {
               puts $f_arch "\n${space_str}${pr_str});"
            } else {
               puts -nonewline $f_arch "\n${space_str}${pr_str},"
            }
         }
      }
   }

# generate the closing statement of the architecture
   puts $f_arch "end $arch_name;\n"

  close $f_ent
  close $f_arch
  destroy .conv

  show_info "File '$EntFileName' with entity '$CirName' ${re1}written\n"
  show_info "File '$ArchFileName' with architecture '$arch_name' ${re2}written\n"
  show_info "to compile these files use GoWithTheFlow\n"

  #set EditFnt $Fnt
  #set Textindent "8"
  #do_vcom "work" $EntFileName
  #do_vcom "work" $ArchFileName
}

# procedure to determine the type of an input port
#-------------------------------------------------
proc get_input_port_type {cell_name cir_name} {
   global MAIN_PORT S_FULL_PORT S_PART_PORT COMP_PORT s_arr m_arr

   if {$cell_name == $cir_name} {
      return $MAIN_PORT
   }
   if {[string range $cell_name 0 1] == "S-"} {
      set idx [string range $cell_name 2 end]
      if {[string index $s_arr($idx) 0] == ">"} {
         return $S_PART_PORT
      } else {
         return $S_FULL_PORT
      }
   }
   if {[string range $cell_name 0 1] == "M-"} {
      set idx [string range $cell_name 2 end]
      if {[string index $m_arr($idx) 0] == ">"} {
         return $COMP_PORT
      } else {
         return $S_FULL_PORT
      }
   }
   return $COMP_PORT
}

# procedure to determine the type of an output port
#--------------------------------------------------
proc get_output_port_type {cell_name cir_name} {
   global MAIN_PORT S_FULL_PORT S_PART_PORT COMP_PORT s_arr m_arr

   if {$cell_name == $cir_name} {
      return $MAIN_PORT
   }
   if {[string range $cell_name 0 1] == "S-"} {
      set idx [string range $cell_name 2 end]
      if {[string index $s_arr($idx) 0] != "<"} {
         return $S_PART_PORT
      } else {
         return $S_FULL_PORT
      }
   }
   if {[string range $cell_name 0 1] == "M-"} {
      set idx [string range $cell_name 2 end]
      if {[string index $m_arr($idx) 0] != "<"} {
         return $COMP_PORT
      } else {
         return $S_FULL_PORT
      }
   }
   return $COMP_PORT
}
