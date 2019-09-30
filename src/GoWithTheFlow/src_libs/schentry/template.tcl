
set new_name ""

# interface to generate a new circuit template
#---------------------------------------------
proc mk_template {} {
   global Fnt CirName new_name

   if {[winfo exists .templ]} { destroy_templ }

# first see if there is still unwritten or updated data
   if {[see_circ_change] == 1} {
      set answ [tk_messageBox -icon error -type yesno\
         -message "Editor contains changed data!\nMust it be saved?"]
      if {$answ == "yes"} { write_cir_file $CirName }
   }

   toplevel	.templ
   frame	.templ.efr	-relief raised	-bd 2 -bg wheat
   frame	.templ.cfr	-relief raised	-bd 0 -bg gold
   label	.templ.efr.lbl	-text "name:"	-font $Fnt -bg wheat
   entry	.templ.efr.en	-width 17	-font $Fnt -bg wheat
   button	.templ.cfr.ok	-width 8 -text "OK"	-font $Fnt -bg gold -command "mk_celloutline 0"
   button	.templ.cfr.cnl	-width 8 -text "Cancel"	-font $Fnt -bg gold -command "mk_celloutline 1"

   pack .templ.efr	-side top	-fill x
   pack .templ.cfr	-side bottom	-fill x
   pack .templ.efr.lbl	-side left
   pack .templ.efr.en	-side left	-padx 5 -pady 9
   pack .templ.cfr.ok	-side left	-padx 5 -pady 5
   pack .templ.cfr.cnl	-side right	-padx 5 -pady 5

   .templ.efr.en insert end $new_name
   focus .templ.efr.en
   update

   wm title .templ "New circuit"
   bind .templ <KeyPress-Return> { mk_celloutline 0 }
   bind .templ <KeyPress-Escape> { mk_celloutline 1 }
}

# procedure to make a new template for a circuit cell
#----------------------------------------------------
proc mk_celloutline {cancel} {
   global CirName CircuitPath ComponentPath new_name
   global NPort NCell NConn NLabel NSplit NMerge
   global Cmd Fnt NameFnt GW CellNh CvW CvH

   set new_name [.templ.efr.en get]
   if {$cancel} { destroy .templ; return }
   if {[test_name $new_name]} { raise .templ; return }

   set cir_files [glob -nocomplain "$CircuitPath/*.cir"];
   foreach i_cir $cir_files {
      set cir_name [lindex [split [file tail $i_cir] .] 0]
      if {$new_name == $cir_name} {
	show_mess "There is already a circuit with this name!\nUse another name!"
	raise .templ
	return
      }
   }
   set cmp_files [glob -nocomplain "$ComponentPath/*.cmp"];
   foreach i_cmp $cmp_files {
      set comp_name [lindex [split [file tail $i_cmp] .] 0]
      if {$new_name == $comp_name} {
	show_mess "There is a component with this name!\nUse another name!"
	raise .templ
	return
      }
   }
   .frm.cv delete all
   .frm.cv configure -scrollregion "0 0 $CvW $CvH"
   set GW [expr $CvW/70.]
   set NCell  -1
   set NConn  -1
   set NPort  -1
   set NLabel -1
   set NSplit -1
   set NMerge -1
   set CirName $new_name
   set x1 [expr $GW*10]
   set y1 [expr $GW*5]
   set x2 [expr $GW*60]
   set y2 [expr (round([winfo height .frm.cv]/$GW) - 5)*$GW]
   set yt [expr $y1+$CellNh*$GW]
   set nx [expr int(($x2-$x1)/$GW)]
   set x2 [expr $x1+$nx*$GW]
   set ny [expr int(($y2-$yt)/$GW)]
   set y2 [expr $yt+$ny*$GW]
   mk_grid $x1 $yt $x2 $y2
   .frm.cv create rectangle $x1 $y1 $x2 $y2 -tag "border border_line"
   .frm.cv create rectangle $x1 $y1 $x2 $yt -fill gold -tag "border header"
   .frm.cv create text [expr ($x1+$x2)/2.] [expr $y1+$CellNh*$GW/2.]\
                   -text $new_name -font $NameFnt -tag "border CirName"
   set Cmd "NONE"
   release_cmds
   destroy .templ
}
