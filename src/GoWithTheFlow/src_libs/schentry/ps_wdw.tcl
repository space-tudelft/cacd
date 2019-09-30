
# interface to the make_ps procedure
#-----------------------------------
proc ps_wdw {} {
   global Fnt orient CirName

   if {$CirName == ""} { return }
   set orient "0"

   if {[winfo exists .ps_wdw]} { destroy .ps_wdw }
   toplevel    .ps_wdw
   frame       .ps_wdw.type -relief raised -bd 2 -bg wheat
   frame       .ps_wdw.cmd  -relief raised -bd 0 -bg gold
   label       .ps_wdw.type.o -text "orientation:" -bg wheat -font $Fnt
   radiobutton .ps_wdw.type.p -text "portrait " -variable orient -value "0" -bg wheat -font $Fnt
   radiobutton .ps_wdw.type.l -text "landscape" -variable orient -value "1" -bg wheat -font $Fnt
   button      .ps_wdw.cmd.ok -text "OK"     -width 8 -bg gold -font $Fnt -command "mk_psfile"
   button      .ps_wdw.cmd.ca -text "Cancel" -width 8 -bg gold -font $Fnt -command "destroy .ps_wdw"

   pack .ps_wdw.type -side top -fill x
   pack .ps_wdw.cmd -side bottom -fill x
   pack .ps_wdw.type.o -side top -side left
   pack .ps_wdw.type.p -side top
   pack .ps_wdw.type.l -side top
   pack .ps_wdw.cmd.ok -side left  -padx 5 -pady 5
   pack .ps_wdw.cmd.ca -side right -padx 5 -pady 5
   wm title .ps_wdw "make PS"
   bind .ps_wdw <KeyPress-Return> { mk_psfile }
   bind .ps_wdw <KeyPress-Escape> { destroy .ps_wdw }
}

# procedure to make a ps_description from the given circuit
#----------------------------------------------------------
proc mk_psfile {} {
   global orient CirName

   set gcol [.frm.cv itemcget grid_line -fill]
   .frm.cv itemconfigure grid_line -fill white
   update
   .frm.cv postscript -rotate $orient -file $CirName.ps -pagewidth 200m
   update
   .frm.cv itemconfigure grid_line -fill $gcol
   destroy .ps_wdw
   show_info "File './$CirName.ps' (re)written\n"
}
