
proc upd_status_line {status_text} {
#-----------------------------------------------------------------------------#
# procedure to update the status_line of the vhdl_master                      #
#-----------------------------------------------------------------------------#
   global ProgName

   if {$ProgName eq "schentry"} {
      show_info "$status_text\n"
   } else {
      .status_line configure -text $status_text
   }
   update idletasks
}

proc goto_dbdir {} {
#-----------------------------------------------------------------------------#
# procedure to set the CWD to a database                                      #
#-----------------------------------------------------------------------------#
   global DbName env MyWd

   set lst ""
   set lst [lappend lst CWD]
   set lst [lappend lst $MyWd/$DbName]

   array set env $lst
   cd $env(CWD)
}

proc update_font {font_name} {
#-----------------------------------------------------------------------------#
# procedure to set a new font                                                 #
#-----------------------------------------------------------------------------#
   global Fnt

   set Fnt $font_name
   .status_line configure -font $font_name
   foreach i [winfo children .ifr] {
      $i configure -font $font_name
   }
   foreach i [winfo children .menubar] {
      $i configure -font $font_name
      if {$i != ""} {
         foreach ii [winfo children $i] {
            $ii configure -font $font_name
            if {$ii != ""} {
               foreach iii [winfo children $ii] {
                  $iii configure -font $font_name
               }
            }
         }
      }
   }
   determine_cv_width
   read_infofile
}

proc determine_cv_width {} {
   global fnt_scl Fnt

   set fnt_scl ""
   for {set i 0} {$i < [string length $Fnt]} {incr i} {
      set chr [string index $Fnt $i]
      if {($chr == 0)||($chr == 1)||($chr == 2)||($chr == 3)||($chr == 4)||\
          ($chr == 5)||($chr == 6)||($chr == 7)||($chr == 8)||($chr == 9)} {
         set fnt_scl ${fnt_scl}$chr
      }
   }
   set fnt_scl [expr $fnt_scl/100.]
   .cv configure -width [expr 420*$fnt_scl]
}

proc set_lib {name} {
#-----------------------------------------------------------------------------#
# procedure to set the simulator library                                      #
#-----------------------------------------------------------------------------#
   global SimLibName

   set SimLibName $name
   read_infofile
}

proc set_db {name} {
#-----------------------------------------------------------------------------#
# procedure to set the layout database                                        #
#-----------------------------------------------------------------------------#
   global DbName

   if {[winfo exists .sim_dac]} {
      df_mess_dialog "INSTRUCTION\n\
	First close the dac_simulation window before giving this command"
      return
   }
   if {[winfo exists .mk_ldm]} {
      df_mess_dialog "INSTRUCTION\n\
	First close the make_ldm window before giving this command"
      return
   }
   set DbName $name
   read_infofile
}

proc restore_cwd {} {
#-----------------------------------------------------------------------------#
# procedure to to restore the origional directory                             #
#-----------------------------------------------------------------------------#
   global MyWd
   cd $MyWd
}

proc fix_sealib {} {
#-----------------------------------------------------------------------------#
# procedure to fix problem with sealib                                        #
#-----------------------------------------------------------------------------#
   global MyWd DbName
   cd $MyWd
   catch {exec rm -rf $DbName/seadif}
}

proc do_import_vhdl_file {fname} {
#----------------------------------------------------------------------------------#
# actual procedure to add a vhdl_file from another directory to the VHDL directory #
#----------------------------------------------------------------------------------#
   set f_to VHDL/[file tail $fname]
   if {[file exists $f_to]} {
      file stat $fname a
      file stat $f_to  b
      if {$a(ino) == $b(ino)} { return 0 }

      if {[df_choise_dialog "The file $fname
already exists in your VHDL directory:
Must I overwrite it?"] == "no"} { return 0 }
      set ename [get_ename_of_vhdl_file $f_to]
      if {$ename != ""} {
	 if {[df_choise_dialog "WARNING:
The file is already in use by an entity/architecture!
Are you sure to overwrite it?"] == "no"} { return 0 }
	 outdate_cir $ename
      }
   }
   file copy -force $fname $f_to
   return 1
}

proc import_vhdl_file {} {
#---------------------------------------------------------------------------#
# procedure to add a vhdl_file from another directory to the VHDL directory #
#---------------------------------------------------------------------------#
   global MyWd
   set files [tk_getOpenFile -title "Import file(s)" -initialdir $MyWd/..\
		-filetypes {{vhdl .vhd} {All *}} -multiple 1]
   foreach fname $files {
      if {[do_import_vhdl_file $fname]} {
	 upd_status_line "added file $fname"
      }
   }
}

proc import_vhdl_dir {} {
#---------------------------------------------------------------------------#
# procedure to add all vhdl_files from some directory to the VHDL directory #
#---------------------------------------------------------------------------#
   global MyWd
   set dir_name [tk_chooseDirectory -title "Import directory"\
		-initialdir $MyWd/.. -mustexist true]
   if {$dir_name != ""} {
      set n 0
      set files [glob -nocomplain $dir_name/*.vhd]
      foreach f $files { incr n [do_import_vhdl_file $f] }
      if {$n == 1} { set f "file" } { set f "files" }
      upd_status_line "added $n $f from directory $dir_name"
   }
}

proc create_layout {cellname} {
   make_layoutcell -c $cellname
}

proc edit_layout {cellname} {
   make_layoutcell "" $cellname
}

proc run_placer_circuit {cellname} {
   run_placer "" $cellname
}

proc run_placer_layout {cellname} {
   run_placer -r $cellname
}

proc make_layoutcell {arg1 arg2} {
#-----------------------------------------------------------------------------#
# procedure to to start the layout generator                                  #
#-----------------------------------------------------------------------------#
   global DbName
   if {$DbName == ""} {
      df_mess_dialog\
         "INSTRUCTION:\n\
          There is no database specified:\n\
          Make or select a database by clicking the button\n\
          Make_database or the button Database"
      return
   }
   goto_dbdir
   upd_status_line "layout_editor: BUSY"
   set res [catch {exec seadali $arg1 $arg2} mess]
   puts "$mess\n"
   restore_cwd
   read_infofile
   upd_status_line "layout_editor: DONE"
}

proc run_placer {arg1 arg2} {
#-----------------------------------------------------------------------------#
# procedure to to start the layout generator                                  #
#-----------------------------------------------------------------------------#
   global DbName
   if {$DbName == ""} {
      df_mess_dialog\
         "INSTRUCTION:\n\
          There is no database specified:\n\
          Make or select a database by clicking the button\n\
          Make_database or the button Database"
      return
   }
   goto_dbdir
   upd_status_line "placer: BUSY"
   set res [catch {exec placer $arg1 $arg2} mess]
   puts "$mess\n"
   restore_cwd
   read_infofile
   upd_status_line "placer: DONE"
}

proc do_schematic {{cell ""}} {
   global SchematicEntry MyWd
   exec $SchematicEntry $cell:$MyWd
   read_infofile
}
