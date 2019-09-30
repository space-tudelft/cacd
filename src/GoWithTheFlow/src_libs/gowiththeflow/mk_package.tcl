
proc mk_pg_template {} {
#-----------------------------------------------------------------------------#
# procedure to make a package template                                        #
#-----------------------------------------------------------------------------#

   set nm [df_txt_dialog "package_name:" ""]
   if {$nm == "CANCELLED"} return

   set nm [string tolower [string trim $nm]]
   if {$nm == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	There was no name for the package specified:\n\
	Try again with a name filled in the package_name entry."
      return
   }
   if {![string is alnum [string map {_ u} $nm]]} {
      df_mess_dialog "ERROR:\n package_name '$nm' contains incorrect char(s)"
      return
   }
       if {[get_parr_idx $nm] >= 0} { set item "package" }\
   elseif {[get_earr_idx $nm] >= 0} { set item "entity" }\
   else { set item "" }
   if {$item != ""} {
      df_mess_dialog "INSTRUCTION:\n\
	The $item '$nm' already exists:\n\
	Specify another package_name and try again"
      return
   }

   set fname "VHDL/$nm\_pkg.vhd"
   if {[file exists $fname]} {
      df_mess_dialog "INSTRUCTION:\n\
	The file '$fname' already exists:\n\
	Specify another package_name and try again"
      return
   }
   set wdw [edit_wdw $fname 1 ""]
   $wdw.txt delete 1.0 end
   $wdw.txt insert end "library IEEE;\n"
   $wdw.txt insert end "use IEEE.std_logic_1164.ALL;\n\n"
   $wdw.txt insert end "PACKAGE $nm IS\n"
   $wdw.txt insert end "END $nm;\n\n"
   $wdw.txt insert end "PACKAGE BODY $nm IS\n"
   $wdw.txt insert end "END $nm;\n"
}
