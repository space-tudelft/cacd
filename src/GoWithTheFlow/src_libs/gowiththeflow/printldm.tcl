
proc print_layout {arg_cell} {
#-----------------------------------------------------------------------------------#
# procedure to generate the window from which the layout of the cell can be printed #
#-----------------------------------------------------------------------------------#
   global Fnt

   toplevel .pl
   frame    .pl.tfr         -relief raised -bd 2 -bg gold
   label    .pl.tfr.lbl     -text "cell: " -font $Fnt -bg gold
   label    .pl.tfr.cell    -text $arg_cell -bg gold -font $Fnt
   frame    .pl.mfr         -relief raised -bd 2 -bg wheat
   label    .pl.mfr.lbl     -text "orientation:" -width 35 -font $Fnt -bg wheat -fg red
   radiobutton .pl.mfr.rb1  -text "portrait " -variable orient -value "P" -bg wheat -font $Fnt
   radiobutton .pl.mfr.rb2  -text "landscape" -variable orient -value "L" -bg wheat -font $Fnt
   frame    .pl.bfr         -relief raised -bd 2 -bg wheat3
   button   .pl.bfr.doit    -text "MakeEps" -font $Fnt -bg wheat3 -command do_print_layout
   button   .pl.bfr.cancel  -text "Cancel" -font $Fnt -bg wheat3 -command {destroy .pl; upd_status_line ""}

   pack .pl.tfr         -side top -fill x
   pack .pl.mfr         -side top -fill both -expand 1
   pack .pl.bfr         -side bottom -fill x
   pack .pl.tfr.lbl     -side left
   pack .pl.tfr.cell    -side left
   pack .pl.mfr.lbl     -side top -fill x
   pack .pl.mfr.rb1     -side top -fill x
   pack .pl.mfr.rb2     -side top -fill x
   pack .pl.bfr.doit    -side left  -padx 9 -pady 3
   pack .pl.bfr.cancel  -side right -padx 9

   .pl.mfr.rb1 select
   wm title .pl "print_layout options"
}

proc do_print_layout {} {
#-----------------------------------------------------------------------------#
# procedure to print the layout of a cell                                     #
#-----------------------------------------------------------------------------#
   global DbName MyWd
   upvar #0 orient orient

   upd_status_line "Print_layout: BUSY"

   set cell [.pl.tfr.cell cget -text]
   if {$cell == ""} { df_mess_dialog "ERROR: cell not found!"; return }

   goto_dbdir
   if {$orient == "P"} {
      set res [catch {exec getepslay -t $cell >& tmp_print}]
   } else {
      set res [catch {exec getepslay -t -r $cell >& tmp_print}]
   }
   if {$res} {
      set fp [open "tmp_print"]
      df_mess_dialog [read $fp]
      close $fp
   }
   restore_cwd

   upd_status_line "Layout printed to $MyWd/$DbName/$cell.eps"
}
