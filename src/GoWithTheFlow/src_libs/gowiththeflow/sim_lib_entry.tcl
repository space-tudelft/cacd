
proc mk_sim_lib {} {
#-----------------------------------------------------------------------------#
# procedure to make a new library for models to simulate                      #
#-----------------------------------------------------------------------------#
   global SimLibName

   set new_name [df_txt_dialog "New VHDL lib:" ""]
   if {$new_name == "CANCELLED"} return

   if {$new_name == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There was no name for the library specified.\n\
          Try again with a name filled out in the\
          'simulator library_name' entry."
      return
   }
   if {[file exists $new_name] == 0} {
      exec vlib  $new_name
   } else {
      df_mess_dialog "INSTRUCTION:\n\
          architecture '$new_name' already exists:\n\
          Try again using another name"
      return
   }
#   .ifr.lib.libs add command -label $new_name -command "set_lib $new_name"
   set SimLibName $new_name
   destroy .simlib
   read_infofile
   upd_status_line "make_simulation_library: DONE"
}
