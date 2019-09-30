
proc make_ldm {cell} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the ldm file can be made          #
#-----------------------------------------------------------------------------#
   global Fnt srccell

   set srccell $cell

   toplevel   .mk_ldm
   frame      .mk_ldm.tfr        -relief raised -bd 2 -bg gold
   label      .mk_ldm.tfr.cell   -text "cell: $srccell" -font $Fnt -bg gold
   label      .mk_ldm.tfr.lbl3   -text "outfile:" -font $Fnt -bg gold
   entry      .mk_ldm.tfr.en     -width 25 -font $Fnt -bg wheat3
   frame      .mk_ldm.lfr        -relief raised -bd 2 -bg wheat
   text       .mk_ldm.txt        -width 80 -height 10 -font $Fnt -bg wheat\
                                    -yscrollcommand ".mk_ldm.sb set"
   scrollbar  .mk_ldm.sb         -command ".mk_ldm.txt yview" -bg wheat
   button     .mk_ldm.lfr.doit   -text "Make_ldm" -font $Fnt -bg wheat3 -command {do_make_ldm}
   button     .mk_ldm.lfr.cancel -text "Cancel" -font $Fnt -bg wheat3 -command {destroy .mk_ldm}

   pack .mk_ldm.lfr         -side left   -fill y
   pack .mk_ldm.tfr         -side top    -fill x
   pack .mk_ldm.tfr.cell    -side left
   pack .mk_ldm.tfr.en      -side right  -padx 10
   pack .mk_ldm.tfr.lbl3    -side right
   pack .mk_ldm.sb          -side right  -fill y
   pack .mk_ldm.txt         -side right  -fill both -expand 1
   pack .mk_ldm.lfr.cancel  -side bottom -padx 10 -pady 10 -fill x
   pack .mk_ldm.lfr.doit    -side bottom -padx 10 -pady 10 -fill x

   wm title .mk_ldm "make_ldm window"
   .mk_ldm.txt delete 1.0 end
}

proc do_make_ldm {} {
#-----------------------------------------------------------------------------#
# procedure to generate the ldm_code                                          #
#-----------------------------------------------------------------------------#
   global srccell

   set fname [string trim [.mk_ldm.tfr.en get]]
   if {$fname == ""} {
      .mk_ldm.txt insert end "no outfile specified: do this first\n"
      return
   }
   set nm [split $fname .]
   if {[llength $nm] > 2 || [llength $nm] == 2 && [lindex $nm 1] != "ldm"} {
      .mk_ldm.txt insert end "$fname has a wrong extension: it should be .ldm\n"
      return
   }
   set cell [lindex $nm 0]

   set answ "ok"
   if {[file exists $cell.ldm]} {
      set answ [df_choise_dialog\
	"File $cell.ldm already exists; can I overwrite it?\nAre you sure?"]
   }
   if {$answ != "no" && [isnewcell $cell] == 1} {
      set answ [df_choise_dialog "Cell\
	'$cell' already exists; can I overwrite it?\n\
	In that case the origional layout of cell '$cell'\n\
	will be lost !"]
   }
   if {$answ == "no"} {
	.mk_ldm.txt insert end "------ no flattened cell made -----\n"
	.mk_ldm.txt insert end "------ no ldm_file written  -----\n"
	.mk_ldm.txt insert end "choose another name for the file (is also the cell name)\n"
	return
   }

   .mk_ldm.txt delete 1.0 end
   .mk_ldm.txt insert end "\ngetldm -o $cell $srccell\n"
   update
   goto_dbdir
   if {[catch {exec getldm -o $cell $srccell} mess]} {
      .mk_ldm.txt insert end $mess
      restore_cwd
      return
   }
   file rename -force $cell.ldm ..
   restore_cwd
   .mk_ldm.txt insert end "\n***** file $cell.ldm (re)written *****\n"
}

proc isnewcell {name} {
#-------------------------------------------------------#
# procedure to see if a cell is already in the celllist #
#-------------------------------------------------------#
   global DbName MyWd

   set fp [open "|cat $MyWd/$DbName/layout/celllist"]
   while {![eof $fp]} {
      if {[gets $fp] == $name} { close $fp; return 1 }
   }
   close $fp
   return 0
}
