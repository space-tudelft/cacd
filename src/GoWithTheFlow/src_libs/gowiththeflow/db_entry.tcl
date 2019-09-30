
#-----------------------------------------------------------------------------#
# procedure to make a new database to make the layout in                      #
#-----------------------------------------------------------------------------#
proc make_db {} {
   global DbName

   set tmp_db_name [df_txt_dialog "New backend lib:" ""]
   if {$tmp_db_name == "CANCELLED"} return

   set tmp_db_name [string trim $tmp_db_name]
   if {$tmp_db_name == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There was no name for the database specified:\n\
          Try again with a name filled out in the database_name entry."
      return
   }
   if {![file exists $tmp_db_name/.dmrc]} {
      exec mkvopr $tmp_db_name
      set DbName $tmp_db_name
#      .ifr.db.dbs add command -label $DbName -command "set_db $DbName"
   } else {
      df_mess_dialog "INSTRUCTION:\n\
          database '$tmp_db_name' already exists:\n\
          Try again using another name"
      return
   }
   destroy .db
   upd_status_line "Make_database: DONE"
}
