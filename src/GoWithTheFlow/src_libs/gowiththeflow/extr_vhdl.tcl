
proc extract_vhdl {cell} {
#-----------------------------------------------------------------------------#
# procedure to generate the the vhdl_code from the layout_database            #
#-----------------------------------------------------------------------------#
   set fname "VHDL/$cell\_EXTR.vhd"
   set wdw [edit_wdw $fname 1 $cell]
   $wdw.txt delete 1.0 end

   set port_len 0
   set port_list ""
   set ref_list ""

if {[catch {open .$cell.term} fp_t]} {
   $wdw.err.txt insert end "cannot read file: .$cell.term\n"
} else {
   $wdw.err.txt configure -fg blue
   $wdw.err.txt insert end "reading file: .$cell.term\n"
   while {![eof $fp_t]} {
      if {[set tstr [gets $fp_t]] == ""} continue

      set nm [lindex $tstr 2]
      if {[lindex $tstr 1] == $nm} {
	 lappend port_list "$nm [lindex $tstr 0]"
	 if {[lindex $tstr 0] == "O"} {
	    lappend ref_list "$nm $nm\_int"
	 } else {
	    lappend ref_list "$nm $nm"
	 }
	 if {[string len $nm] > $port_len} { set port_len [string len $nm] }
      } else {
	 set ix [lindex [split $nm "()"] 1]
	 set nm [lindex [split $nm "()"] 0]
	 if {[lindex $tstr 0] == "O"} {
	    lappend ref_list "[lindex $tstr 1] $nm\_int($ix)"
	 } else {
	    lappend ref_list "[lindex $tstr 1] $nm\($ix)"
	 }
	 if {[lsearch $port_list "$nm *"] == -1} {
	    set nbrs [split [lindex $tstr 1] "_"]
	    lappend port_list "$nm [lindex $tstr 0] [lindex $nbrs end-2] [lindex $nbrs end-1]"
	    if {[string len $nm] > $port_len} { set port_len [string len $nm] }
	 }
      }
   }
   close $fp_t
}
   goto_dbdir

   $wdw.err.txt insert end "extracting: space -egate -Sexpand_connectivity $cell\n"
   catch {exec space -egate -Sexpand_connectivity $cell}

   set Cell [string toupper [string index $cell 0]][string range $cell 1 end]
   $wdw.err.txt insert end "getting vhdl: xvhdl -r $Cell\n"
   set fp [open "|xvhdl -r $Cell"]
   while {![eof $fp]} {
      if {[set xxx [gets $fp]] == ""} {
	 if {[eof $fp]} break
	 $wdw.txt insert end "\n"
	 continue
      }

      if {[lindex $xxx 1] == "structural"} {
	 set xxx [lreplace $xxx 1 1 "extracted"]
      } elseif {[lindex $xxx 1] == "structural;"} {
	 set xxx [lreplace $xxx 1 1 "extracted"]
	 append xxx ";"
      }

      if {[lindex $xxx 0] == "BEGIN"} {
	 for {set i 0} {$i < [llength $port_list]} {incr i} {
	    if {[lindex [lindex $port_list $i] 1] != "O"} continue
	    set tnm [lindex [lindex $port_list $i] 0]_int
	    if {[llength [lindex $port_list $i]] == 2} {
	       $wdw.txt insert end "  SIGNAL $tnm: STD_LOGIC;\n"
	    } else {
	       set fst [lindex [lindex $port_list $i] 2]
	       set lst [lindex [lindex $port_list $i] 3]
	       if {$fst > $lst} { set to "DOWNTO" } { set to "TO" }
	       $wdw.txt insert end "  SIGNAL $tnm: STD_LOGIC_VECTOR($fst $to $lst);\n"
	    }
	 }
	 $wdw.txt insert end "$xxx\n"
	 for {set i 0} {$i < [llength $port_list]} {incr i} {
	    if {[lindex [lindex $port_list $i] 1] != "O"} continue
	    set tnm [lindex [lindex $port_list $i] 0]
	    $wdw.txt insert end "  $tnm <= $tnm\_int;\n"
	 }
      }\
      elseif {[lindex $xxx 1] == "<="} {
	 set nme1 [lindex $xxx 0]
	 if {$nme1 == "vss" || $nme1 == "vdd"} {
	    set nme2 $nme1
	    set nme1 [string trimright [lindex $xxx 2] ";"]
	 } else {
	    set nme2 [string trimright [lindex $xxx 2] ";"]
	 }
	 set f_idx1 [lsearch -regexp $ref_list "^$nme1 .*"]
	 set f_idx2 [lsearch -regexp $ref_list "^$nme2 .*"]
	 if {$f_idx1 == -1} {
	    if {$f_idx2 >= 0} {
	       set nmx2 [lindex [lindex $ref_list $f_idx2] 1]
	       # the first name could be an input, the second is an output
	       # so replace $ref_list[$f_idx2][1] (=${nme2}_int) by $nme1
	       set ref_list [lreplace $ref_list $f_idx2 $f_idx2 "$nme2 $nme1"]
	       $wdw.txt insert end "  $nmx2 <= $nme1;\n"
	    } else {
	       $wdw.txt insert end "  $nme1 <= $nme2;\n"
	    }
	 } else {
	    if {$f_idx2 >= 0} { set nme2 [lindex [lindex $ref_list $f_idx2] 1] }
	    set nmx1 [lindex [lindex $ref_list $f_idx1] 1]
	    set ref_list [lreplace $ref_list $f_idx1 $f_idx1 "$nme1 $nme2"]
	    $wdw.txt insert end "  $nmx1 <= $nme2;\n"
	 }
      }\
      elseif {[lindex $xxx 3] == "MAP"} {
	 while {[string first ";" $xxx] == -1} {
	    set xxx [string trimright $xxx]
	    append xxx " [string trimleft [gets $fp]]"
	 }
	 set yy $xxx
	 set line "  [lindex $yy 0] [lindex $yy 1] [lindex $yy 2] [lindex $yy 3] ("
	 if {[lindex $yy 4] == "("} {
	    set i_first 5
	 } else {
	    set i_first 4
	 }
	 for {set i $i_first} {$i < [llength $yy]} {incr i} {
	    set nm [string trimright [string trimleft [lindex $yy $i] (] ",);"]
	    if {$i > $i_first} { append line ", " }
	    set f_idx [lsearch -regexp $ref_list "^$nm .*"]
	    if {$f_idx == -1} {
	       append line $nm
	    } else {
	       append line [lindex [lindex $ref_list $f_idx] 1]
	    }
	 }
	 if {[string range $line end-9 end] == ", vss, vdd"} {
	    $wdw.txt insert end "[string range $line 0 end-10]);\n"
	 } else {
	    $wdw.txt insert end "$line);\n"
	 }
      }\
      else {
	 if {[lindex $xxx 0] == "PORT" && $port_len} {
	    $wdw.txt insert end "  PORT ("
	    set done 0
	    for {set i 0} {$i < [llength $port_list]} {incr i} {
	       switch [lindex [lindex $port_list $i] 1] {
		  I	{ set type "IN " }
		  O	{ set type "OUT" }
		default { set type "INOUT" }
	       }
	       append type " STD_LOGIC"
	       set nm [lindex [lindex $port_list $i] 0]
	       if {[llength [lindex $port_list $i]] > 2} {
		  set fst [lindex [lindex $port_list $i] 2]
		  set lst [lindex [lindex $port_list $i] 3]
		  if {$fst > $lst} { set to "DOWNTO" } { set to "TO" }
		  append type "_VECTOR($fst $to $lst)"
	       }
	       if {$done} { $wdw.txt insert end ";\n        " } { incr done }
	       $wdw.txt insert end [format "%-*s: $type" $port_len $nm]
	    }
	    $wdw.txt insert end ");\n"
	    while {![eof $fp]} {
	       set xxx [gets $fp]
	       if {[lindex $xxx 0] == "END"} {
		  $wdw.txt insert end "$xxx\n"
		  break
	       }
	    }
	    continue
	 }
	 if {[lindex $xxx 0] == "LIBRARY"} {
	    $wdw.txt insert end "LIBRARY [lindex $xxx 1]\n"
	    $wdw.txt insert end "USE [lindex $xxx 3]\n"
	    $wdw.txt insert end "LIBRARY CellsLib;\n"
	    $wdw.txt insert end "USE CellsLib.CellsLib_DECL_PACK.all;\n"
	 } else {
	    if {[lindex $xxx 0] == "SIGNAL"} {
	       if {[lindex $xxx 1] == "vss:"} continue
	       if {[lindex $xxx 1] == "vdd:"} continue
	    }
	    $wdw.txt insert end "$xxx\n"
	 }
      }
   }
   catch {close $fp}

   restore_cwd

# look if vdd or vss is present
# if present add a vss and/or a vdd signal and give
# this signal the value '0' or '1' respectively.
   if {[$wdw.txt search -regexp -nocase "\[ (,\]vss\[ );,\]" 1.0 end] != ""} {
     set idx [$wdw.txt search "BEGIN" 1.0 end]
     $wdw.txt insert "$idx lineend" "\n  vss <= '0';"
     $wdw.txt insert "$idx linestart" "  SIGNAL vss: STD_LOGIC;\n"
   }
   if {[$wdw.txt search -regexp -nocase "\[ (,\]vdd\[ );,\]" 1.0 end] != ""} {
     set idx [$wdw.txt search "BEGIN" 1.0 end]
     $wdw.txt insert "$idx lineend" "\n  vdd <= '1';"
     $wdw.txt insert "$idx linestart" "  SIGNAL vdd: STD_LOGIC;\n"
   }

   $wdw.txt highlight 1.0 end
}
