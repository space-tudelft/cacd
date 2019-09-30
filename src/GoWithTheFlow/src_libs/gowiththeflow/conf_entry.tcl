
proc mk_newconf {ename aname afname} {
#-----------------------------------------------------------------------------#
# procedure to make a new configuration to an entity_architecture pair        #
#-----------------------------------------------------------------------------#
   global Earr Aarr Carr SimLibName

   set tmp_conf_name [df_txt_dialog "configuration name:" "$ename\_$aname\_cfg"]
   if {$tmp_conf_name == "CANCELLED"} return

   set tmp_conf_name [string trim $tmp_conf_name]
   if {$tmp_conf_name == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	There was no name for the configuration specified.\n\
	Try again with a name filled in the configuration_name entry."
      return
   }
   if {[get_carr_idx $tmp_conf_name] >= 0} {
      df_mess_dialog "INSTRUCTION:\n\
	The configuration '$tmp_conf_name' already exists.\n\
	Try again using another name."
      return
   }
   set fname VHDL/$tmp_conf_name.vhd
   set wdw [edit_wdw $fname 1 ""]
   $wdw.txt delete 1.0 end
   $wdw.txt insert end "configuration $tmp_conf_name of $ename is\n"
   $wdw.txt insert end "   for $aname\n"
   update

   set child_list [find_children_in_vhdl $afname]
   for {set n 0} {$n < [llength $child_list]} {incr n} {
      set child [lindex $child_list $n]
      set c_list ""
      if {[set i [get_earr_idx $child]] >= 0} {
            for {set j 2} {$j < [llength $Earr($i)]} {incr j} {
               set a_idx [lindex $Earr($i) $j]
               for {set k 3} {$k < [llength $Aarr($a_idx)]} {incr k} {
                  set c_idx [lindex $Aarr($a_idx) $k]
                  lappend c_list [lindex $Carr($c_idx) 0]
               }
            }
      } else {
         # child is not a local entity
         continue
      }
      if {$c_list == ""} {
         df_mess_dialog "INSTRUCTION:\n\
             no configuration for $child present:\n\
             make this configuration first"
          destroy .conf
          destroy $wdw
          return
      }
      if {[llength $c_list] == 1} {
	 set cname [lindex $c_list 0]
      } else {
	 set cname [conf_choise_dialog "your choice for the configuration of entity $ename:" $wdw.txt $c_list]
      }
      $wdw.txt insert end "      for all: $child use configuration $SimLibName.$cname;\n"
      $wdw.txt insert end "      end for;\n"
   }
   $wdw.txt insert end "   end for;\n"
   $wdw.txt insert end "end $tmp_conf_name;\n"
   $wdw.txt highlight 1.0 end
}

proc find_children_in_vhdl {fname} {

   set celllist ""
   set COLON 0

   set fp [open $fname r]
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} continue
      if {[set i [string first "--" $txt]] >= 0} {
         if {$i == 0} continue
         set txt [string range $txt 0 $i-1]
      }

      if {!$COLON} {
         if {[set i [string first ":" $txt]] < 0} continue
         set txt [string range $txt $i+1 end]
         set COLON 1
      }
      foreach w [split $txt] {
         if {$w == ""} continue
         if {[incr COLON] == 2} {
            set word2 [string tolower $w]
         } elseif {$COLON == 3} {
            set word3 [string tolower $w]
            if {$word3 != "port" && $word3 != "generic"} { set COLON 0; break }
         } else {
            set word4 [string tolower $w]
            if {$word4 == "map" || [string eq -len 4 $word4 "map("]} {
               if {[lsearch $celllist $word2] < 0} { lappend celllist $word2 }
            }
            set COLON 0
            break
         }
      }
   }
   close $fp

   return $celllist
}
