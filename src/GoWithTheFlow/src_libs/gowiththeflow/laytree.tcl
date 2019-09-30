
proc export_layout_tree {cell} {
   global MyWd

   restore_cwd
   run_in_window "exptree" $cell "creating file $MyWd/$cell.lt.tar"
}

proc import_layout_tree {} {
   global MyWd

   set types {
      {{TAR Files} {.lt.tar .lt.TAR}}
      {{All Files} *}
   }
   set file_name [tk_getOpenFile -initialdir $MyWd -filetypes $types -title "Open .lt.tar file"]
   if {$file_name == ""} {
      set message "No tar file specified"
      return
   }
   restore_cwd
   run_in_window "imptree" $file_name "importing data from $file_name"

   read_infofile
}

proc run_in_window {command arg1 finalmess} {
   global MyWd DbName Fnt

   if {![file isdir $DbName]} {
      df_mess_dialog "INSTRUCTION:\n\
	The database '$DbName' has not yet been made or chosen:\n\
	Do this first and try again"
      return
   }

   if {![winfo exists .window_to_run_in]} {
      toplevel  .window_to_run_in
      text      .window_to_run_in.txt -width 80 -height 10 -font $Fnt -bg wheat\
				-yscrollcommand ".window_to_run_in.sb set"
      scrollbar .window_to_run_in.sb   -command ".window_to_run_in.txt yview" -bg wheat

      pack .window_to_run_in.sb  -side right -fill y
      pack .window_to_run_in.txt -side right -fill both -expand 1

      wm title .window_to_run_in "run window"
   }

   .window_to_run_in.txt insert end "\nrunning $command $arg1 ...\n"
   update
   .window_to_run_in.txt see end

   if {[catch {exec $command $arg1} mess]} { .window_to_run_in.txt insert end $mess }
   .window_to_run_in.txt insert end "\nDONE $finalmess\n"
   restore_cwd
   update
   .window_to_run_in.txt see end
}
