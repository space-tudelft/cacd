
# procedure to pretty print a statement list
#-------------------------------------------
proc print_list {fp p_list} {
   set llen 79
   set ident [string length [lindex $p_list 0]]
   set len 0
   for {set idx 0} {$idx < [llength $p_list]} {incr idx} {
      set t_len [string length [lindex $p_list $idx]]
      if {[expr $len + $t_len] <= $llen} {
         if {$idx == 0} {
            puts -nonewline $fp "[lindex $p_list $idx]"
            set len [expr $len + $t_len]
         } else {
            puts -nonewline $fp "[lindex $p_list $idx] "
            set len [expr $len + $t_len + 1]
	 }
      } else {
         puts -nonewline $fp [format "\n%${ident}s%s" " " "[lindex $p_list $idx] "]
         set len [expr $ident + $t_len + 1]
      }
   }
   puts $fp ""
}

# procedure to get the range of a port
#-------------------------------------
proc find_range {ct_arr cell port} {
   upvar $ct_arr tmp

   for {set i 0} {$i < [llength $tmp($cell)]} {incr i} {
      if {[lindex [lindex $tmp($cell) $i] 0] == $port} {
         set rh [lindex [lindex $tmp($cell) $i] 1]
         set rl [lindex [lindex $tmp($cell) $i] 2]
         return "${rh}_${rl}"
      }
   }
   return ""
}

# procedure to see if a port is already present in the net_arr
#-------------------------------------------------------------
proc find_term_in_netarr {net_arr port} {
   upvar $net_arr tmp

   for {set i 0} {$i < [array size tmp]} {incr i} {
      foreach j $tmp($i) {
         if {$j == $port} { return $i }
      }
   }
   return -1
}

# procedure to add the ports of a connection to a net
#----------------------------------------------------
proc add_to_netarr {net_arr net_no ftn1 ftn2} {
   upvar $net_arr tmp
   upvar $net_no nn
   set i1 [find_term_in_netarr tmp $ftn1]
   set i2 [find_term_in_netarr tmp $ftn2]
   if {($i1 < 0) && ($i2 < 0)} {
      lappend tmp($nn) $ftn1 $ftn2
      incr nn
   } elseif {($i1 >= 0) && ($i2 < 0)} {
      lappend tmp($i1) $ftn2
   } elseif {($i1 < 0) && ($i2 >= 0)} {
      lappend tmp($i2) $ftn1
   } else {
      puts "huh, i1 = $i1 and i2 = $i2: how is this possible"
   }
}

# procedure to generate the sls code for the circuit shown
#---------------------------------------------------------
proc sls_conv {} {
   global CircuitPath CirName SlsPath ComponentPath DbName

   if {$CirName == ""} { return }

# see if the picture shown is already written
  if {[see_circ_change] == 1} {
      set answ [tk_messageBox -icon error -type yesno\
         -message "Data changed:\
                   must it be (re)written for the generation of the sls_code?"]
      if {$answ == "yes"} { write_cir_file $CirName }
   }

   if {![file exists $SlsPath]} { file mkdir $SlsPath }

   set sls_file_name "$SlsPath/$CirName.sls"
   set re ""
   if {[file exist $sls_file_name]} {
      file copy -force $sls_file_name $sls_file_name\_
      set re "re"
   }

   set f_cir [open $CircuitPath/$CirName.cir "r"]
   set f_sls [open $sls_file_name "w"]
   set llen 80
   set np -1
   set ni -1
   set ns -1
   set nm -1
   set nc -1
   set nl -1
   set nn 0
   set net_arr(0) ""
   set net_no 0
   set ct_arr($CirName) ""
   while {![eof $f_cir]} {
      set txt [gets $f_cir]
      if {[lindex $txt 0] == "T"} {
         if {[llength $txt] > 1} {
            incr np
            set ttype [lindex $txt 1]
            set tlist [split [lindex $txt 2] "(:)"]
            if {[llength $tlist] > 1} {
               set basic_tname [lindex $tlist 0]
               set rh [lindex $tlist 1]
               set rl [lindex $tlist 2]
               set p_arr($np) "$basic_tname ${basic_tname}_${rh}_$rl $rh $rl $ttype"
               lappend ct_arr($CirName) "$basic_tname $rh $rl"
            } else {
               set p_arr($np) "$tlist $tlist 0 0 $ttype"
               lappend ct_arr($CirName) "$tlist"
            }
         }
      } elseif {[lindex $txt 0] == "I"} {
         if {[llength $txt] > 1} {
            incr ni
            set basic_iname [lindex [split [lindex $txt 1] "~"] 0]
            set i_arr($ni) "[lindex $txt 1] $basic_iname"
            if {![array exists celterm_arr] ||\
                 [array names ct_arr $basic_iname] == ""} {
               set f_cmp [open $ComponentPath/$basic_iname.cmp "r"]
               set ct_arr($basic_iname) ""
               while {![eof $f_cmp]} {
                  set txt [gets $f_cmp]
                  if {([lindex $txt 0] == "port") && ([llength $txt] == 3)} {
                     set t_name [lindex $txt 2]
                     set name [split $t_name "(:)"]
                     if {[llength $name] == 1} {
                        lappend ct_arr($basic_iname) $name
                     } else {
                        set rh [lindex $name 1]
                        set rl [lindex $name 2]
                        set name [lindex $name 0]
                        lappend ct_arr($basic_iname) "$name $rh $rl"
                     }
                  }
               }
               close $f_cmp
            }
         }
      } elseif {[lindex $txt 0] == "S"} {
         incr ns
         if {[llength $txt] > 1} {
            if {[string index [lindex $txt 1] 0] == ">"} {
               set s_type "O"
            } else {
               set s_type "I"
            }
            set range [split [string trim [lindex $txt 1] "<>"] ":"]
            set rh [lindex $range 0]
            set rl [lindex $range 1]
            set s_arr($ns) "$s_type $rh $rl"
         } else {
            set s_arr($ns) ""
         }
      } elseif {[lindex $txt 0] == "M"} {
         incr nm
         if {[llength $txt] > 1} {
            if {[string index [lindex $txt 1] 0] == ">"} {
               set m_type "O"
            } else {
               set m_type "I"
            }
            set range [split [string trim [lindex $txt 1] "<>"] ":"]
            set rh [lindex $range 0]
            set rl [lindex $range 1]
            set m_arr($nm) "$m_type $rh $rl"
         } else {
            set m_arr($nm) ""
         }
      } elseif {[lindex $txt 0] == "C"} {
         if {[llength $txt] > 1} {
            incr nc
            set from [lindex [split [lindex $txt 1] ">"] 0]
            set to   [lindex [split [lindex $txt 1] ">"] 1]
            set inst_from [lindex [split $from "."] 0]
            set term_from [lindex [split $from "."] 1]
            set inst_to   [lindex [split $to "."] 0]
            set term_to   [lindex [split $to "."] 1]
            set c_arr($nc) "$inst_from $term_from $inst_to $term_to"
         }
      } elseif {[lindex $txt 0] == "L"} {
         if {[llength $txt] > 1} {
            incr nl
            set btname [lindex [split [lindex $txt 2] "(:)"] 0]
            set lname [split [lindex $txt 1] "(:)"]
            if {[llength $lname] > 1} {
               set lbname [lindex $lname 0]
               set rh     [lindex $lname 1]
               set rl     [lindex $lname 2]
               if {[expr $rh - $rl] > 0} {
                  set inc -1
               } else {
                  set inc 1
               }
               set il $rh
               while {$il != $rl} {
                  set l_arr($nl) "${lbname}_${rh}_${rl}_$il ${btname}_${rh}_${rl}_$il"
                  incr nl
                  incr il $inc
               }
               set l_arr($nl) "${lbname}_${rh}_${rl}_$il ${btname}_${rh}_${rl}_$il"
            } else {
               if {([string tolower $lname] == "gnd") ||\
                   ([string tolower $lname] == "vss")} {
			set lname "vss"
               } elseif {[string tolower $lname] == "vdd"} {
			set lname "vdd"
               }
               set l_arr($nl) "$lname $btname"
            }
         }
      }
   }
   close $f_cir
#
# substitute the labels in the label_array
#
   for {set il 0} {$il < [array size l_arr]} {incr il} {
      set x 0
      if {[string match "S-*" [lindex $l_arr($il) 1]] == 1} {
         set x "S"
      }
      if {[string match "M-*" [lindex $l_arr($il) 1]] == 1} {
         set x "M"
      }
      if {$x != 0} {
         set sname [lindex [split [lindex $l_arr($il) 1] "."] 0]
         set strm  [lindex [split [lindex $l_arr($il) 1] "."] 1]
         set stlst [split $strm "_"]
         if {[llength $stlst] > 1} {
            set delta [expr abs([lindex $stlst 1] - [lindex $stlst 3])]
         }
         set s_no  [lindex [split $sname "-"] 1]
         if {$x == "S"} {
            set rh [lindex $s_arr($s_no) 1]
            set rl [lindex $s_arr($s_no) 2]
         } else {
            set rh [lindex $m_arr($s_no) 1]
            set rl [lindex $m_arr($s_no) 2]
         }
         for {set ic 0} {$ic < [array size c_arr]} {incr ic} {
            if {([lindex $c_arr($ic) 0] == $sname) &&\
                ([lindex $c_arr($ic) 1] == "out")} {
               set cell [lindex $c_arr($ic) 2]
               set tlst [split [lindex $c_arr($ic) 3] "(:)"]
               set tname [lindex $tlst 0]
               set rht   [lindex $tlst 1]
               set rlt   [lindex $tlst 2]
               if {$rh == $rl} {
                  set l_arr($il) [lreplace $l_arr($il) 1 1 "$cell.${tname}_${rht}_${rlt}_$rh"]
               } else {
                  if {$rht > $rlt} {
                     set idx [expr $rht - $delta]
                  } else {
                     set idx [expr $rht + $delta]
                  }
                  set l_arr($il) [lreplace $l_arr($il) 1 1 "$cell.${tname}_${rht}_${rlt}_$idx"]
               }
            }
         }
      }
   }

   # for {set c 0} {$c <= $nc} {incr c} {
   #    if {$c_arr($c) == ""} { continue }
   #    show_info "1: $c_arr($c)\n"
   # }

#
# substitute the splitters in c_arr
#
   for {set i 0} {$i <= $ns} {incr i} {
      if {[lindex $s_arr($i) 0] == "O"} {
         for {set c 0} {$c <= $nc} {incr c} {
            if {[string match "S-*" [lindex $c_arr($c) 0]] == 0} {
               if {[lindex $c_arr($c) 2] == "S-$i"} {
                  set btname [lindex [split [lindex $c_arr($c) 1] "(:)"] 0]
                  lappend s_arr($i) [lindex $c_arr($c) 0] $btname
                  set c_arr($c) ""
                  break
               }
            }
         }
      } else {
         for {set c 0} {$c <= $nc} {incr c} {
            if {[string match "S-*" [lindex $c_arr($c) 2]] == 0} {
               if {[lindex $c_arr($c) 0] == "S-$i"} {
                  set btname [lindex [split [lindex $c_arr($c) 3] "(:)"] 0]
                  lappend s_arr($i) [lindex $c_arr($c) 2] $btname
                  set c_arr($c) ""
                  break
               }
            }
         }
      }
   }
   for {set c 0} {$c <= $nc} {incr c} {
      if {[string match "S-*" [lindex $c_arr($c) 0]] == 1} {
         set s_nbr [string range [lindex $c_arr($c) 0] 2 end]
         set c_arr($c) [lreplace $c_arr($c) 0 0 [lindex $s_arr($s_nbr) 3]]
         set tname [lindex $s_arr($s_nbr) 4]
         set rh    [lindex $s_arr($s_nbr) 1]
         set rl    [lindex $s_arr($s_nbr) 2]
         set c_arr($c) [lreplace $c_arr($c) 1 1 "$tname\($rh:$rl\)"]
      }
      if {[string match "S-*" [lindex $c_arr($c) 2]] == 1} {
         set s_nbr [string range [lindex $c_arr($c) 2] 2 end]
         set c_arr($c) [lreplace $c_arr($c) 2 2 [lindex $s_arr($s_nbr) 3]]
         set tname [lindex $s_arr($s_nbr) 4]
         set rh    [lindex $s_arr($s_nbr) 1]
         set rl    [lindex $s_arr($s_nbr) 2]
         set c_arr($c) [lreplace $c_arr($c) 3 3 "$tname\($rh:$rl\)"]
      }
   }

#
# substitute the mergers in c_arr
#
   for {set i 0} {$i <= $nm} {incr i} {
      if {[lindex $m_arr($i) 0] == "O"} {
         for {set c 0} {$c <= $nc} {incr c} {
            if {[string match "M-*" [lindex $c_arr($c) 0]] == 0} {
               if {[lindex $c_arr($c) 2] == "M-$i"} {
                  set btname [lindex [split [lindex $c_arr($c) 1] "(:)"] 0]
                  lappend m_arr($i) [lindex $c_arr($c) 0] $btname
                  set c_arr($c) ""
                  break
               }
            }
         }
      } else {
         for {set c 0} {$c <= $nc} {incr c} {
            if {[string match "M-*" [lindex $c_arr($c) 2]] == 0} {
               if {[lindex $c_arr($c) 0] == "M-$i"} {
                  set btname [lindex [split [lindex $c_arr($c) 3] "(:)"] 0]
                  lappend m_arr($i) [lindex $c_arr($c) 2] $btname
                  set c_arr($c) ""
                  break
               }
            }
         }
      }
   }
   for {set c 0} {$c <= $nc} {incr c} {
      if {[string match "M-*" [lindex $c_arr($c) 0]] == 1} {
         set m_nbr [string range [lindex $c_arr($c) 0] 2 end]
         set p_nbr [string range [lindex $c_arr($c) 1] 1 end]
         set c_arr($c) [lreplace $c_arr($c) 0 0 [lindex $m_arr($m_nbr) 3]]
         set tname [lindex $m_arr($m_nbr) 4]
         set rh    [lindex $m_arr($m_nbr) 1]
         set rl    [lindex $m_arr($m_nbr) 2]
         set r [expr $p_nbr + $rl]
         set c_arr($c) [lreplace $c_arr($c) 1 1 "$tname\($r:$r\)"]
      }
      if {[string match "M-*" [lindex $c_arr($c) 2]] == 1} {
         set m_nbr [string range [lindex $c_arr($c) 2] 2 end]
         set p_nbr [string range [lindex $c_arr($c) 3] 1 end]
         set c_arr($c) [lreplace $c_arr($c) 2 2 [lindex $m_arr($m_nbr) 3]]
         set tname [lindex $m_arr($m_nbr) 4]
         set rh    [lindex $m_arr($m_nbr) 1]
         set rl    [lindex $m_arr($m_nbr) 2]
         set r [expr $p_nbr + $rl]
         set c_arr($c) [lreplace $c_arr($c) 3 3 "$tname\($r:$r\)"]
      }
   }

   # for {set c 0} {$c <= $nc} {incr c} {
   #    if {$c_arr($c) == ""} { continue }
   #    show_info "2: $c_arr($c)\n"
   # }

#
# make the net_array
#
   for {set c 0} {$c <= $nc} {incr c} {
      if {$c_arr($c) == ""} {
         continue
      }
      set tl1 [split [lindex $c_arr($c) 1] "(:)"]
      set tl2 [split [lindex $c_arr($c) 3] "(:)"]
      if {[llength $tl1] == 1} {
         if {[llength $tl2] == 1} {
            set from "[lindex $c_arr($c) 0].[lindex $c_arr($c) 1]"
            set to   "[lindex $c_arr($c) 2].[lindex $c_arr($c) 3]"
            add_to_netarr net_arr net_no $from $to
         } else {
            set bname2 [lindex $tl2 0]
            set rh2    [lindex $tl2 1]
            set rl2    [lindex $tl2 2]
            set from "[lindex $c_arr($c) 0].[lindex $c_arr($c) 1]"
            set bcel2 [lindex [split [lindex $c_arr($c) 2] ~] 0]
            set range_to   [find_range ct_arr $bcel2 $bname2]
            set to   "[lindex $c_arr($c) 2].${bname2}_${range_to}_$rh2"
            add_to_netarr net_arr net_no $from $to
         }
      } else {
         if {[llength $tl2] == 1} {
            set bname1 [lindex $tl1 0]
            set rh1    [lindex $tl1 1]
            set rl1    [lindex $tl1 2]
            set bcel1 [lindex [split [lindex $c_arr($c) 0] ~] 0]
            set range_from   [find_range ct_arr $bcel1 $bname1]
            set from   "[lindex $c_arr($c) 0].${bname1}_${range_from}_$rh1"
            set to "[lindex $c_arr($c) 2].[lindex $c_arr($c) 3]"
            add_to_netarr net_arr net_no $from $to
         } else {
            set bname1 [lindex $tl1 0]
            set rh1    [lindex $tl1 1]
            set rl1    [lindex $tl1 2]
            set bname2 [lindex $tl2 0]
            set rh2    [lindex $tl2 1]
            set rl2    [lindex $tl2 2]
            if {[expr $rh1-$rl1] > 0} {
               set inc1 -1
            } else {
               set inc1 1
            }
            if {[expr $rh2-$rl2] > 0} {
               set inc2 -1
            } else {
               set inc2 1
            }
            set bcel1 [lindex [split [lindex $c_arr($c) 0] ~] 0]
            set bcel2 [lindex [split [lindex $c_arr($c) 2] ~] 0]
            set range_from [find_range ct_arr $bcel1 $bname1]
            set range_to   [find_range ct_arr $bcel2 $bname2]
            set i1 $rh1
            set i2 $rh2
            while {$i1 != $rl1} {
               set from "[lindex $c_arr($c) 0].${bname1}_${range_from}_$i1"
               set to   "[lindex $c_arr($c) 2].${bname2}_${range_to}_$i2"
               add_to_netarr net_arr net_no $from $to
               incr i1 $inc1
               incr i2 $inc2
            }
            set from "[lindex $c_arr($c) 0].${bname1}_${range_from}_$i1"
            set to   "[lindex $c_arr($c) 2].${bname2}_${range_to}_$i2"
            add_to_netarr net_arr net_no $from $to
         }
      }
   }
#
# add a name to the nets
#
   for {set n 0} {$n < [array size net_arr]} {incr n} {
      set netname net_$n
      foreach i $net_arr($n) {
         if {[lindex [split $i "."] 0] == $CirName} {
            set netname [lindex [split $i "."] 1]
            break
         }
      }
      set net_arr($n) [linsert $net_arr($n) 0 $netname]
   }
#
# add the connections through the labels to the net_arr
#
   for {set il 0} {$il < [array size l_arr]} {incr il} {
      set lname [lindex $l_arr($il) 0]
      set found "False"
      for {set nn 0} {$nn < [array size net_arr]} {incr nn} {
         if {[lindex $net_arr($nn) 0] == $lname} {
            lappend net_arr($nn) [lindex $l_arr($il) 1]
            set found "True"
            break
         }
      }
      if {$found == "False"} {
         lappend net_arr($net_no) [lindex $l_arr($il) 0] [lindex $l_arr($il) 1]
         incr net_no
      }
   }

   # for {set n 0} {$n < [array size net_arr]} {incr n} {
   #     show_info "net_arr: $net_arr($n)\n"
   # }
#
# generate the extern network statements
#
   set names [array names ct_arr]
   foreach nm $names {
      if {$nm == $CirName} { continue }

      set ext_list [list "extern network $nm (terminal "]
      for {set ii 0} {$ii < [llength $ct_arr($nm)]} {incr ii} {
         set tlist [lindex $ct_arr($nm) $ii]
         if {[llength $tlist] == 1} {
            lappend ext_list "$tlist,"
         } else {
            set btname [lindex $tlist 0]
            set rh     [lindex $tlist 1]
            set rl     [lindex $tlist 2]
            for {set j $rh} {$j >= $rl} {incr j -1} {
               lappend ext_list "${btname}_${rh}_${rl}_$j,"
            }
         }
      }
      lappend ext_list "vss, vdd)"
      print_list $f_sls $ext_list
   }
#
# generate the network statement and the .$cellname.term file
#
   set fpt [open .${CirName}.term "w"]

   set netw_list [list "network $CirName (terminal "]
   for {set t 0} {$t < [array size p_arr]} {incr t} {
      set bnm [lindex $p_arr($t) 0]
      set nm [lindex $p_arr($t) 1]
      set rh [lindex $p_arr($t) 2]
      set rl [lindex $p_arr($t) 3]
      set ttype [lindex $p_arr($t) 4]
      if {$ttype == "in"} { set ttype2 "I" }
      if {$ttype == "out"} { set ttype2 "O" }
      if {$ttype == "*in"} { set ttype2 "B" }
      if {$ttype == "*out"} { set ttype2 "B" }
      if {($rh == "0") && ($rl == "0")} {
         lappend netw_list "$nm,"
         puts $fpt "$ttype2 ${nm} ${bnm}"

      } else {
         for {set j $rh} {$j >= $rl} {incr j -1} {
            lappend netw_list "${nm}_$j,"
            puts $fpt "$ttype2 ${nm}_$j ${bnm}($j)"
         }
      }
   }
   lappend netw_list "vss, vdd)"
   print_list $f_sls $netw_list
   puts $f_sls "\{"

   close $fpt
#
# generate the net statements
#
   for {set n 0} {$n < [array size net_arr]} {incr n} {
      set ntl ""
      set nname [lindex $net_arr($n) 0]
      for {set it 1} {$it < [llength $$net_arr($n)]} {incr it} {
         set cn [lindex [split [lindex $net_arr($n) $it] .] 0]
         set tn [lindex [split [lindex $net_arr($n) $it] .] 1]
         if {($cn == $CirName) && ($tn != $nname)} {
            lappend ntl $tn
         }
      }
      set len_ntl [llength $ntl]
      if {$len_ntl > 0} {
         puts -nonewline $f_sls "   net \{$nname, "
         for {set i 0} {$i < [expr $len_ntl - 1]} {incr i} {
            puts -nonewline $f_sls "[lindex $ntl $i], "
         }
         puts $f_sls "[lindex $ntl  [expr $len_ntl - 1]]\};"
      }
   }
#
# generate the instance statements
#
   for {set i 0} {$i < [array size i_arr]} {incr i} {
      set i_name [lindex $i_arr($i) 0]
      set c_name [lindex $i_arr($i) 1]
      set inst_list  [list "   \{${c_name}_$i\} $c_name ("]
      set tlist $ct_arr([lindex $i_arr($i) 1])
      for {set t 0} {$t < [llength $tlist]} {incr t} {
         set trm [lindex $tlist $t]
         if {[llength $trm] == 1} {
            set netno [find_term_in_netarr net_arr $i_name.$trm]
            if {$netno < 0} {
               lappend inst_list " ,"
            } else {
               lappend inst_list "[lindex $net_arr($netno) 0],"
            }
         } else {
            set bn [lindex $trm 0]
            set rh [lindex $trm 1]
            set rl [lindex $trm 2]
            if {$rh > $rl} {
               for {set j $rh} {$j >= $rl} {incr j -1} {
                  set netno [find_term_in_netarr net_arr "$i_name.${bn}_${rh}_${rl}_$j"]
                  if {$netno < 0} {
                     lappend inst_list " ,"
                  } else {
                     lappend inst_list "[lindex $net_arr($netno) 0],"
                  }
               }
            } else {
               for {set j $rh} {$j <= $rl} {incr j} {
                  set netno [find_term_in_netarr net_arr "$i_name.${bn}_${rh}_${rl}_$j"]
                  if {$netno < 0} {
                     lappend inst_list " ,"
                  } else {
                     lappend inst_list "[lindex $net_arr($netno) 0],"
                  }
               }
            }
        }
      }
      lappend inst_list "vss, vdd);"
      print_list $f_sls $inst_list
   }
   puts $f_sls "\}"
   close $f_sls
   show_info "File '$sls_file_name' ${re}written\n"

   if {![file exist $DbName/.dmrc]} {
      show_info "no nelsis directory '$DbName'; no compilation\n"
      return
   }
   goto_dbdir
   show_info "compiling $sls_file_name ...\n"
   # set res [catch {exec csls $sls_file_name} mess]
   set res [catch {exec csls "../$sls_file_name" >& csls.messages}]
   if {$res} {
      set fp [open "csls.messages"]
      set tt [read $fp]
      close $fp
      show_info $tt
   }
   set fp [open "circuit/$CirName/src_arch" w]
   puts $fp "circuit"
   close $fp

   restore_cwd
   show_info "compilation of $sls_file_name DONE\n"
}
