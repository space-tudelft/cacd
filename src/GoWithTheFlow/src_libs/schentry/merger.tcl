
# procedure to make a new template for a merger
#------------------------------------------------
proc mk_merger {} {
   global Fnt
   upvar #0 bi_sp bi_sp

   if {[winfo exists .templ]} { destroy .templ }

# check if the merger is inside the circuit_area
   if {[check_area_inside] == 0} {
	show_mess "A merger must be inside the circuit area!"
	destroy_templ
	return
   }
   set bi_sp 0

   toplevel	.templ
   frame	.templ.hfr -relief raised -bd 2 -bg wheat
   frame	.templ.sfr -relief raised -bd 2 -bg wheat
   frame	.templ.tfr -relief raised -bd 2 -bg wheat
   frame	.templ.cfr -relief raised -bd 0 -bg gold
   label	.templ.hfr.lbl -text "width:" -font $Fnt -bg wheat
   tk_optionMenu .templ.hfr.om width\
	2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
   label	.templ.sfr.lbl -text "expand on:" -font $Fnt -bg wheat
   checkbutton	.templ.tfr.btn -text "bidirectional" -variable bi_sp -font $Fnt -bg wheat

   button .templ.cfr.ok  -text "OK"     -width 8 -font $Fnt -bg gold -command "do_make_merger"
   button .templ.cfr.cnl -text "Cancel" -width 8 -font $Fnt -bg gold -command "destroy_templ"
   tk_optionMenu .templ.sfr.om split_type input output
   .templ.hfr.om	configure -font $Fnt -bg wheat
   .templ.hfr.om.menu	configure -font $Fnt -bg wheat
   .templ.sfr.om	configure -font $Fnt -bg wheat
   .templ.sfr.om.menu	configure -font $Fnt -bg wheat

   pack .templ.hfr	-side top    -fill x
   pack .templ.sfr	-side top    -fill x
   pack .templ.tfr	-side top    -fill x
   pack .templ.cfr	-side bottom -fill x
   pack .templ.hfr.lbl	-side left  -padx 4
   pack .templ.hfr.om	-side left
   pack .templ.sfr.lbl	-side left  -padx 4
   pack .templ.sfr.om	-side right -padx 4
   pack .templ.tfr.btn	-side left  -padx 4 -pady 2
   pack .templ.cfr.ok	-side left  -padx 5 -pady 5
   pack .templ.cfr.cnl	-side right -padx 5 -pady 5

   update
   wm title .templ "add BUS EXPANDER"
   bind .templ <KeyPress-Return> { do_make_merger }
   bind .templ <KeyPress-Escape> { destroy_templ }
}

proc do_make_merger {} {
   upvar #0 width width split_type split_type bi_sp bi_sp

   set c [.frm.cv coords outline]
   destroy_templ
   make_merger [lindex $c 0] [lindex $c 1] $width $split_type $bi_sp
}

# procedure to make a new merger
#---------------------------------
proc make_merger {x y w stype bi} {
   global GW PortFnt NMerge MERGE_ARR InpCol OutpCol InoutCol

   incr NMerge
   set x $x
   set y $y
   set TD [expr $GW/2.]
   set Mtag M-$NMerge
   set ub [expr $w - 1]
   if {$stype == "input"} {
	set MERGE_ARR($NMerge) "$ub:0<"
   } else {
	set MERGE_ARR($NMerge) ">$ub:0"
   }

   set CompW [expr 4*$GW]
   set CompH [expr 2* $w * $GW]

# outline
   .frm.cv create rectangle $x $y [expr $x+$CompW] [expr $y+$CompH]\
                -fill gold -outline black -tag "$Mtag ${Mtag}_bb"

   set CompIn ""
   set CompOut ""
   if {$bi} { 
      set bisign "*" 
   } else { 
      set bisign ""  
   }
   if {$stype == "input"} {
      for {set i 0} {$i < $w} {incr i} {
         lappend CompIn "${bisign}i$i"
      }
      lappend CompOut "${bisign}o"
   } else {
      for {set i 0} {$i < $w} {incr i} {
         lappend CompOut "${bisign}o$i"
      }
      lappend CompIn "${bisign}i"
   }
   
   set xi [expr $x]
   set xo [expr $x+$CompW]

# input_ports
   set yt [expr $y + $GW]
   foreach port $CompIn {
        set fill $InpCol
        if {[string index $port 0] == "*"} {
            set fill $InoutCol
            set port [string trimleft $port *]
        }
        .frm.cv create polygon [expr $xi-$TD] $yt\
                $xi [expr $yt-$TD] [expr $xi+$TD] $yt $xi [expr $yt+$TD]\
                -fill $fill -outline black -tag "$Mtag $Mtag.$port"
        .frm.cv create text [expr $xi+$GW] $yt -anchor w\
                -font $PortFnt -text $port -tag "$Mtag $Mtag.$port"
        set yt [expr $yt+2*$GW]
   }

# output_ports
   set yt [expr $y + $GW]
   foreach port $CompOut {
        set fill $OutpCol
        if {[string index $port 0] == "*"} {
            set fill $InoutCol
            set port [string trimleft $port *]
        }
        .frm.cv create polygon [expr $xo-$TD] $yt\
                $xo [expr $yt-$TD] [expr $xo+$TD] $yt $xo [expr $yt+$TD]\
                -fill $fill -outline black -tag "$Mtag $Mtag.$port"
        .frm.cv create text [expr $xo-$GW] $yt -anchor e\
                -font $PortFnt -text $port -tag "$Mtag $Mtag.$port"
        set yt [expr $yt+2*$GW]
   }
}
