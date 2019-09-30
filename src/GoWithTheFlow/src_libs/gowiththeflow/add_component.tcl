
proc mk_component {cmp_name fname} {
#-----------------------------------------------------------------------------#
# procedure to make a component from an index in the Earr                     #
#-----------------------------------------------------------------------------#

   set fp_vhd [open $fname r]
   set fp_cmp [open components/$cmp_name.cmp w]
#
# first read the vhdl_file with the entity end make a long string from it.
# start recording after the keyword 'port' and stop after the keyword 'end'
#
   set n_par 0
   set do_add 0
   set tot_str ""
   while {![eof $fp_vhd]} {
      set txt [gets $fp_vhd]
      set cmt_idx [string first "--" $txt]
      if {$cmt_idx >= 0} {
         set txt [string range $txt 0 $cmt_idx-1]
      }
      if {$txt == ""} continue

      regsub -all {[():;,]} $txt " & " txt

      foreach t $txt {
	 if {$do_add} {
	    if {$t == "("} { incr n_par; continue }
	    if {$t == ","} { continue }
	    if {$t == ":"} { continue }
	    if {$t == ")"} { incr n_par -1
	       if {$n_par <= 0} { incr do_add; break }
	       continue
	    }
	    append tot_str " $t"
	 }\
	 elseif {[string equal -nocase "port" $t]} { incr do_add }
      }
      if {$do_add > 1} break
   }
   close $fp_vhd

#
# decode the tokens from tot_str
#
   set new_names ""
   set n_in 0
   set n_out 0
   set np 0
   foreach token $tot_str {
      if {$token == ";"} { set new_names "" }\
      elseif {[string tolower $token] == "in"} {
         foreach i $new_names {
            lappend Parr($i) "in"
            incr n_in
         }
      }\
      elseif {[string tolower $token] == "out"} {
         foreach i $new_names {
            lappend Parr($i) "out"
            incr n_out
         }
      }\
      elseif {[string tolower $token] == "inout"} {
         foreach i $new_names {
            lappend Parr($i) "*out"
            incr n_out
         }
      }\
      elseif {[string tolower $token] == "std_logic"} { }\
      elseif {[string tolower $token] == "std_logic_vector"} { }\
      elseif {[string tolower $token] == "boolean"} { }\
      elseif {[string tolower $token] == "downto"} { }\
      elseif {[string tolower $token] == "to"} { }\
      elseif {[string is integer $token]} {
         foreach i $new_names {
            lappend Parr($i) $token
         }
      }\
      else {
         set Parr($np) $token
         lappend new_names $np
         incr np
      }
   }
#
# make an array with the lines of text for the ports of the component
#
   set inlen 0
   set outlen 0
   for {set i 0} {$i < $np} {incr i} {
      set name [lindex $Parr($i) 0]
      set type [lindex $Parr($i) 1]
      if {[llength $Parr($i)] > 2} {
         set hi [lindex $Parr($i) 2]
         set lw [lindex $Parr($i) 3]
         append name "($hi:$lw)"
      }
      set print_arr($i) "$type $name"
      set tmplen [string length $name]
      if {$type == "in"} {
         if {$tmplen > $inlen} { set inlen $tmplen }
      } else {
         if {$tmplen > $outlen} { set outlen $tmplen }
      }
   }
#
# determine the size of the component
#
   if {$n_in < $n_out} { set n_in $n_out }
   set nx [expr int(($inlen + $outlen + 1)*0.75)]
   set ny [expr 2*$n_in + 2]
#
# print the data to the file
#
   puts $fp_cmp "bbox $nx $ny"
   for {set i 0} {$i < $np} {incr i} {
      puts $fp_cmp "port $print_arr($i)"
   }
   close $fp_cmp
}
