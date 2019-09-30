
proc arch_name_exists {ename aname} {
#---------------------------------------------------------------------------#
# procedure which checks if there is already an architecture in the project #
# with the name 'aname' belonging to the entity 'ename'                     #
# returns 1 if this is the case; 0 otherwise                                #
#---------------------------------------------------------------------------#
   if {[set i [get_earr_idx $ename]] >= 0} {
      if {[get_aarr_idx $i $aname] >= 0} { return 1 }
   }
   return 0
}

proc mk_newarch {ename} {
#---------------------------------------------------------------------------#
# procedure to make a new architecture to an entity                         #
#---------------------------------------------------------------------------#
   set tmp_arch_name [df_txt_dialog "architecture_name:" "behaviour"]
   if {$tmp_arch_name == "CANCELLED"} return

   set tmp_arch_name [string tolower [string trim $tmp_arch_name]]
   if {$tmp_arch_name == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There was no name for the architecture specified.\n\
          Try again with a name filled out in the architecture_name entry."
      return
   }
   if {[arch_name_exists $ename $tmp_arch_name]} {
      df_mess_dialog "INSTRUCTION:\n\
          The architecture '$tmp_arch_name' already exists:\n\
          Try again using another name"
      return
   }
   set fname VHDL/$ename-$tmp_arch_name.vhd
   set wdw [edit_wdw $fname 1 $ename]
   $wdw.txt delete 1.0 end
   $wdw.txt insert end "library IEEE;\n"
   $wdw.txt insert end "use IEEE.std_logic_1164.ALL;\n\n"
   $wdw.txt insert end "architecture $tmp_arch_name of $ename is\n"
   $wdw.txt insert end "begin\n"
   $wdw.txt insert end "end $tmp_arch_name;\n"
   $wdw.txt highlight 1.0 end
}
