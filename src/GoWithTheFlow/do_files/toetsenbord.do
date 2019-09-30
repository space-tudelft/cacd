
set w_btn [expr 5]
set h_btn [expr 5]
set dx    [expr 5]
set dy    [expr 5]
set drx   [expr 5]
set drxr  [expr 5]
set dry   [expr 5]
set names {1 2 3 4 5 6 7 8 9 * 0 #}
set mtx_btn_inst ""

set labels [find instances *]
foreach i $labels {
   set cell [lindex [split [describe $i] " :()\n"] 6]
   if {($cell == "tbord_cout") || ($cell == "tbord_rout")} {
      set mtx_btn_inst $i
   }
}

if {$mtx_btn_inst == ""} {
   echo "*******************************************"
   echo "** no toetsenbord component present"
   echo "** place one first and try again"
   echo "*******************************************"
}\
else {
   toplevel .mtx -bg wheat
   canvas   .mtx.cv -width [expr $drx + $drxr + 4*$dx + 3*$w_btn]m\
                    -height [expr 2*$dry + 5*$dy + 4*$h_btn]m\
                    -bg wheat
   frame    .mtx.fr0 -relief raised -bd 1 -bg wheat3
   frame    .mtx.fr1 -relief raised -bd 1 -bg wheat3
   frame    .mtx.fr2 -relief raised -bd 1 -bg wheat3
   frame    .mtx.fr3 -relief raised -bd 1 -bg wheat3
   label    .mtx.fr0.lbl -text  "dendertijd(ms):" -bg wheat3 -anchor center
   entry    .mtx.fr0.en -width 7 -bg wheat
   label    .mtx.fr1.lbl -text on_time(ms): -bg wheat3 -anchor center
   entry    .mtx.fr1.en -width 7 -bg wheat
   label    .mtx.fr2.lbl -text sim_time(ms): -bg wheat3 -anchor center
   entry    .mtx.fr2.en -width 7 -bg wheat
   button   .mtx.fr3.qt -text "Quit" -bg red -command {destroy .mtx}


   pack .mtx.cv -side top
   pack .mtx.fr3 -side bottom -fill x
   pack .mtx.fr2 -side bottom -fill x
   pack .mtx.fr1 -side bottom -fill x
   pack .mtx.fr0 -side bottom -fill x
   pack .mtx.fr0.lbl -side left -fill x -expand 1
   pack .mtx.fr0.en -side right -padx 5 -pady 2
   pack .mtx.fr1.lbl -side left -fill x -expand 1
   pack .mtx.fr1.en -side right -padx 5 -pady 2
   pack .mtx.fr2.lbl -side left -fill x -expand 1
   pack .mtx.fr2.en -side right -padx 5 -pady 2
   pack .mtx.fr3.qt -side left -padx 5 -pady 2 -fill x -expand 1

# default waarden: 3*de dendertijd voor on_time en
# 5 keer de dendertijd voor sim_time
   set mbt [examine /$mtx_btn_inst/lbl1/bouncer/mbt]
   set mbt [string trim $mbt "\{\}"]
   set pres_val [expr 3*[lindex $mbt 0]]
   set run_val [expr 5*[lindex $mbt 0]]
   .mtx.fr0.en delete 0 end
   .mtx.fr1.en delete 0 end
   .mtx.fr2.en delete 0 end
   .mtx.fr0.en insert end [lindex $mbt 0]
   .mtx.fr1.en insert end "$pres_val"
   .mtx.fr2.en insert end "$run_val"

   .mtx.cv bind btn_1_1 <ButtonPress-1> { do_sim 1 1 1}
   .mtx.cv bind btn_2_1 <ButtonPress-1> { do_sim 2 1 2}
   .mtx.cv bind btn_3_1 <ButtonPress-1> { do_sim 3 1 3}
   .mtx.cv bind btn_1_2 <ButtonPress-1> { do_sim 1 2 4}
   .mtx.cv bind btn_2_2 <ButtonPress-1> { do_sim 2 2 5}
   .mtx.cv bind btn_3_2 <ButtonPress-1> { do_sim 3 2 6}
   .mtx.cv bind btn_1_3 <ButtonPress-1> { do_sim 1 3 7}
   .mtx.cv bind btn_2_3 <ButtonPress-1> { do_sim 2 3 8}
   .mtx.cv bind btn_3_3 <ButtonPress-1> { do_sim 3 3 9}
   .mtx.cv bind btn_1_4 <ButtonPress-1> { do_sim 1 4 s}
   .mtx.cv bind btn_2_4 <ButtonPress-1> { do_sim 2 4 0}
   .mtx.cv bind btn_3_4 <ButtonPress-1> { do_sim 3 4 h}
   .mtx.cv bind btn_1_1 <ButtonRelease-1> { .mtx.cv itemconfigure outl_1_1 -fill wheat }
   .mtx.cv bind btn_2_1 <ButtonRelease-1> { .mtx.cv itemconfigure outl_2_1 -fill wheat }
   .mtx.cv bind btn_3_1 <ButtonRelease-1> { .mtx.cv itemconfigure outl_3_1 -fill wheat }
   .mtx.cv bind btn_1_2 <ButtonRelease-1> { .mtx.cv itemconfigure outl_1_2 -fill wheat }
   .mtx.cv bind btn_2_2 <ButtonRelease-1> { .mtx.cv itemconfigure outl_2_2 -fill wheat }
   .mtx.cv bind btn_3_2 <ButtonRelease-1> { .mtx.cv itemconfigure outl_3_2 -fill wheat }
   .mtx.cv bind btn_1_3 <ButtonRelease-1> { .mtx.cv itemconfigure outl_1_3 -fill wheat }
   .mtx.cv bind btn_2_3 <ButtonRelease-1> { .mtx.cv itemconfigure outl_2_3 -fill wheat }
   .mtx.cv bind btn_3_3 <ButtonRelease-1> { .mtx.cv itemconfigure outl_3_3 -fill wheat }
   .mtx.cv bind btn_1_4 <ButtonRelease-1> { .mtx.cv itemconfigure outl_1_4 -fill wheat }
   .mtx.cv bind btn_2_4 <ButtonRelease-1> { .mtx.cv itemconfigure outl_2_4 -fill wheat }
   .mtx.cv bind btn_3_4 <ButtonRelease-1> { .mtx.cv itemconfigure outl_3_4 -fill wheat }

   .mtx.cv create rectangle ${drx}m ${dry}m [expr $drx + 4*$dx + 3*$w_btn]m\
                            [expr $dry + 5*$dy + 4*$h_btn]m -width 2
   for {set i [expr 1]} {$i <= 3} {incr i} {
      set i1 [expr $i-1]
      for {set j [expr 1]} {$j <= 4} {incr j} {
         set j1 [expr $j-1]
         set tag_list ""
         lappend tag_list btn_${i}_${j}
         lappend tag_list outl_${i}_${j}
         .mtx.cv create rectangle [expr $drx + $i*$dx + $i1*$w_btn]m\
				  [expr $dry + $j*$dy + $j1*$h_btn]m\
				  [expr $drx + $i*$dx + $i*$w_btn]m\
				  [expr $dry + $j*$dy + $j*$h_btn]m\
				-fill wheat -tag $tag_list
         .mtx.cv create text [expr $drx + $i*$dx + $i1*$w_btn + $w_btn/2.]m\
                             [expr $dry + $j*$dy + $j1*$h_btn + $h_btn/2.]m\
                             -text [lindex $names [expr $j1*3+$i1]]\
                             -tag btn_${i}_${j}
      }
   }
}

proc do_sim {x y btn} {
   global mtx_btn_inst

   run 0 ns
   change /$mtx_btn_inst/lbl$btn/bouncer/mbt "[.mtx.fr0.en get] ms"
   .mtx.cv itemconfigure outl_${x}_${y} -fill red
   force /$mtx_btn_inst/btn_$btn TRUE 0 ms, FALSE [.mtx.fr1.en get] ms
   run [.mtx.fr2.en get] ms
}
