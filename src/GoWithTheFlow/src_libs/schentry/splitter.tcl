
# procedure to make a new template for a splitter
#------------------------------------------------
proc mk_splitter {} {
   global Fnt
   upvar #0 bi_sp bi_sp

   if {[winfo exists .templ]} { destroy .templ }

# check if the splitter is inside the circuit_area
   if {[check_area_inside] == 0} {
	show_mess "A splitter must be inside the circuit area!"
	destroy_templ
	return
   }
   set bi_sp 0

   toplevel	.templ
   frame	.templ.hfr -relief raised -bd 2 -bg wheat
   frame	.templ.sfr -relief raised -bd 2 -bg wheat
   frame	.templ.tfr -relief raised -bd 2 -bg wheat
   frame	.templ.cfr -relief raised -bd 0 -bg gold
   label	.templ.hfr.lbl -text "high:" -font $Fnt -bg wheat
   tk_optionMenu .templ.hfr.om high_bound\
	0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
   label	.templ.hfr.lbl2 -text "low:" -font $Fnt -bg wheat
   tk_optionMenu .templ.hfr.om2 low_bound\
	0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
   label	.templ.sfr.lbl -text "subrange on:" -font $Fnt -bg wheat
   checkbutton	.templ.tfr.btn -text "bidirectional" -variable bi_sp -font $Fnt -bg wheat

   button .templ.cfr.ok  -text "OK"     -width 8 -font $Fnt -bg gold -command "do_make_splitter"
   button .templ.cfr.cnl -text "Cancel" -width 8 -font $Fnt -bg gold -command "destroy_templ"
   tk_optionMenu .templ.sfr.om split_type input output
   .templ.hfr.om	configure -font $Fnt -bg wheat
   .templ.hfr.om.menu	configure -font $Fnt -bg wheat
   .templ.hfr.om2	configure -font $Fnt -bg wheat
   .templ.hfr.om2.menu	configure -font $Fnt -bg wheat
   .templ.sfr.om	configure -font $Fnt -bg wheat
   .templ.sfr.om.menu	configure -font $Fnt -bg wheat

   pack .templ.hfr	-side top    -fill x
   pack .templ.sfr	-side top    -fill x
   pack .templ.tfr	-side top    -fill x
   pack .templ.cfr	-side bottom -fill x
   pack .templ.hfr.lbl	-side left  -padx 4
   pack .templ.hfr.om	-side left
   pack .templ.hfr.om2	-side right -padx 4
   pack .templ.hfr.lbl2	-side right
   pack .templ.sfr.lbl	-side left  -padx 4
   pack .templ.sfr.om	-side right -padx 4
   pack .templ.tfr.btn	-side left  -padx 4 -pady 2
   pack .templ.cfr.ok	-side left  -padx 5 -pady 5
   pack .templ.cfr.cnl	-side right -padx 5 -pady 5

   update
   wm title .templ "add BUS CONNECTOR"
   bind .templ <KeyPress-Return> { do_make_splitter }
   bind .templ <KeyPress-Escape> { destroy_templ }
}

proc do_make_splitter {} {
   upvar #0 high_bound high_bound low_bound low_bound split_type split_type bi_sp bi_sp

   if {$high_bound < $low_bound} {
	show_mess "Splitter high-bound must be >= low-bound!"
	raise .templ
   } else {
	set c [.frm.cv coords outline]
	destroy_templ
	make_splitter [lindex $c 0] [lindex $c 1] $high_bound $low_bound $split_type $bi_sp
   }
}

# procedure to make a new splitter
#---------------------------------
proc make_splitter {x y hb lb stype bi} {
   global GW PortFnt NSplit SPLIT_ARR InpCol OutpCol InoutCol

   incr NSplit
   set x $x
   set y [expr $y+$GW]
   set TD [expr $GW/2.]
   set Stag S-$NSplit
   .frm.cv create polygon [expr $x-$TD] $y [expr $x+$GW] [expr $y-$GW]\
		[expr $x+4*$GW] [expr $y-$GW] [expr $x+5*$GW+$TD] $y\
		[expr $x+4*$GW] [expr $y+$GW] [expr $x+$GW] [expr $y+$GW]\
		-fill gold -outline black -tag $Stag
   if {$stype == "input"} {
	.frm.cv create text [expr $x+2.5*$GW] $y -text "$hb:$lb <"\
		-font $PortFnt -tag $Stag
	set SPLIT_ARR($NSplit) "$hb:$lb<"
   } else {
	.frm.cv create text [expr $x+2.5*$GW] $y -text "> $hb:$lb"\
		-font $PortFnt -tag $Stag
	set SPLIT_ARR($NSplit) ">$hb:$lb"
   }
   set fill $InpCol
   if {$bi} { set fill $InoutCol }
   .frm.cv create polygon [expr $x-$TD] $y $x [expr $y-$TD]\
			  [expr $x+$TD] $y $x [expr $y+$TD]\
		-fill $fill -outline black -tag "$Stag $Stag.in"
   set x [expr $x+5*$GW]
   if {!$bi} { set fill $OutpCol }
   .frm.cv create polygon [expr $x-$TD] $y $x [expr $y-$TD]\
			  [expr $x+$TD] $y $x [expr $y+$TD]\
		-fill $fill -outline black -tag "$Stag $Stag.out"
}
