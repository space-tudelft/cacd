
proc show_cons {} {
   global fnt pg_pod_arr meas_pod_arr bondbar tmp_file_name buf_file_name
   global CLOCK trig_pod i_trigsig message

   if {$bondbar == "-"} {
      set message "first set the bondbar number !"
      update
      return
   }

   if {[winfo exists .cons]} {
      .cons.cv delete all
   }\
   else {
      toplevel   .cons
      frame      .cons.menubar       -relief raised -bd 2 -bg gold2
      menubutton .cons.menubar.file -text "File" -menu .cons.menubar.file.cmds\
                                    -bg gold2 -font $fnt
      menu       .cons.menubar.file.cmds -font $fnt -bg gold2
                 .cons.menubar.file.cmds add command -label "Write_postscript"\
                                                     -command {write_ps}
                 .cons.menubar.file.cmds add command -label "Write_channels"\
                                                     -command {write_ch}
                 .cons.menubar.file.cmds add command -label "Hide"\
                                                  -command {wm withdraw .cons}
      canvas     .cons.cv -background gray90 -width 600 -height 800

      pack .cons.menubar       -side top   -fill x
      pack .cons.menubar.file  -side left
      pack .cons.cv -side bottom -fill both -expand 1

      set x_trftxt [expr [winfo x .] + [winfo width .] + 20]
      set y_trftxt [winfo y .]
      wm geometry .cons +$x_trftxt+$y_trftxt

      tkwait visibility .cons.cv
      .menubar.show.cmds entryconfigure pod_connections -state normal
   }
   set xl_textb [expr 50]
   set xr_textb [expr 550]
   set yt_textb [expr 20]
   set dy_text  [expr 20]
   set yb_textb [expr $yt_textb+4*$dy_text]
   .cons.cv create rectangle $xl_textb  $yt_textb $xr_textb $yb_textb
   .cons.cv create line $xl_textb  [expr $yt_textb+$dy_text]\
                        $xr_textb [expr $yt_textb+$dy_text]
   .cons.cv create text [expr $xl_textb + 5] [expr $yt_textb+0.5*$dy_text]\
                        -text "connections to the\
                               logic analyzer generated from:"\
                        -anchor w -font $fnt
   .cons.cv create text [expr $xl_textb + 5] [expr $yt_textb+1.5*$dy_text]\
                        -text "reffile: $tmp_file_name"\
                        -anchor w -font $fnt
   .cons.cv create text [expr $xl_textb + 5] [expr $yt_textb+2.5*$dy_text]\
                        -text "buffile: $buf_file_name"\
                        -anchor w -font $fnt
   .cons.cv create text [expr $xl_textb + 5] [expr $yt_textb+3.5*$dy_text]\
                        -text "bondbar: $bondbar"\
                        -anchor w -font $fnt
   set dtx [expr 20]
   set dty [expr 20]
   set ddy [expr 10]
   set dx  [expr 200]
   set ddx [expr 5]
   set dy  [expr 20*$dty + 21*$ddy]
   set xc  [expr [winfo width .cons.cv]/2]
   set yc  [expr ([winfo height .cons.cv]+$yb_textb)/2]
   set xl  [expr $xc - $dx/2]
   set xr  [expr $xc + $dx/2]
   set yt  [expr $yc - $dy/2]
   set yb  [expr $yc + $dy/2]
   set r   [expr 25]
   set dxp [expr 100]
   set nt [expr 40]
   .cons.cv create rectangle $xl $yt $xr $yb -fill wheat -outline black
   .cons.cv create arc [expr $xc-$r] [expr $yt-$r] [expr $xc+$r] [expr $yt+$r]\
                  -start 180 -extent 180
   set xtl [expr $xl-$dtx]
   set xtr [expr $xr+$dtx]

   # set debug3 [open "debug3" "w"]

   for {set i [expr 1]} {$i <= $nt} {incr i} {
      if {$i <= [expr $nt/2]} {
         set ytt [expr $yt+$i*$ddy + ($i-1)*$dty]
         set ytb [expr $yt+$i*($ddy+$dty)]
         .cons.cv create rectangle $xtl $ytt $xl $ytb -fill wheat3\
                                                      -outline black 
         set xtc [expr ($xl+$xtl)/2]
         set ytc [expr ($ytt+$ytb)/2]
         .cons.cv create text $xtc $ytc -text $i -font $fnt
         set ttext [get_term_text $bondbar $i]
   # puts $debug3 "$bondbar $i $ttext"
         .cons.cv create text [expr $xl+$ddx] [expr $ytc - 6]\
                         -text $ttext\
                         -font $fnt -anchor w
         switch $ttext {
            NC      {}
            VDD     { show_pod LIN [expr $xl-$dty] [expr $xl-$dxp] $ytc\
                               "VDD" "-1" red black white
                    }
            VSS     { show_pod LIN [expr $xl-$dty] [expr $xl-$dxp] $ytc\
                               "VSS" "-1" black black white
                    }
            default { if {[lsearch [array names pg_pod_arr] $ttext] >= 0} {
                         show_pod LIN [expr $xl-$dty] [expr $xl-$dxp] $ytc\
                                      [lindex $pg_pod_arr($ttext) 0]\
                                      [lindex $pg_pod_arr($ttext) 1]\
                                      white\
                                      [lindex $pg_pod_arr($ttext) 2]\
                                      black
                      }\
                      elseif {\
                      [lsearch [array names meas_pod_arr] $ttext] >= 0} {
                         show_pod LOUT [expr $xl-$dty] [expr $xl-$dxp] $ytc\
                                       [lindex $meas_pod_arr($ttext) 0]\
                                       [lindex $meas_pod_arr($ttext) 1]\
                                       white\
                                       [lindex $meas_pod_arr($ttext) 2]\
                                       black
                      }\
                      elseif {$ttext == [lindex $CLOCK 1]} {
                         show_pod LIN [expr $xl-$dty] [expr $xl-$dxp] $ytc\
                          "CLOCK" "-1" blue black white
                      }
                    }
         }
      }\
      else {
         set ytt [expr $yt+($nt-$i+1)*($ddy+$dty)]
         set ytb [expr $yt+($nt-$i+1)*$ddy+($nt-$i)*$dty]
         .cons.cv create rectangle $xr $ytt $xtr $ytb -fill wheat3\
                                                      -outline black 
         set xtc [expr ($xr+$xtr)/2]
         set ytc [expr ($ytt+$ytb)/2]
         .cons.cv create text $xtc $ytc -text $i -font $fnt
         .cons.cv create text $xtc $ytc -text $i -font $fnt
         set ttext [get_term_text $bondbar $i]
         .cons.cv create text [expr $xr-$ddx] [expr $ytc + 6]\
                         -text $ttext\
                         -font $fnt -anchor e
         switch $ttext {
            NC      {}
            VDD     { show_pod RIN [expr $xr+$dty] [expr $xr+$dxp] $ytc\
                               "VDD" "-1" red black white
                    }
            VSS     { show_pod RIN [expr $xr+$dty] [expr $xr+$dxp] $ytc\
                               "VSS" "-1" black black white
                    }
            default { if {[lsearch [array names pg_pod_arr] $ttext] >= 0} {
                          show_pod RIN [expr $xr+$dty] [expr $xr+$dxp] $ytc\
                                       [lindex $pg_pod_arr($ttext) 0]\
                                       [lindex $pg_pod_arr($ttext) 1]\
                                       white\
                                       [lindex $pg_pod_arr($ttext) 2]\
                                       black
                      }\
                      elseif {\
                      [lsearch [array names meas_pod_arr] $ttext] >= 0} {
                         show_pod ROUT [expr $xr+$dty] [expr $xr+$dxp] $ytc\
                                       [lindex $meas_pod_arr($ttext) 0]\
                                       [lindex $meas_pod_arr($ttext) 1]\
                                       white\
                                       [lindex $meas_pod_arr($ttext) 2]\
                                       black
                      }\
                      elseif {$ttext == [lindex $CLOCK 1]} {
                         show_pod RIN [expr $xr+$dty] [expr $xr+$dxp] $ytc\
                          "CLOCK" "-1" blue black white
                      }
                    }
         }
      }
   }

   if {$i_trigsig >= 0} {
      .cons.cv create text [expr $xl_textb ] [expr $yb + 25]\
                        -text "directly connect \"pod 1B(0) ch 0\" to \"pod [lindex $trig_pod 0] ch [lindex $trig_pod 1]\" for use as trigger signal"\
                        -anchor w -font $fnt
   }\
   else {
      .cons.cv create text [expr $xl_textb ] [expr $yb + 25]\
                        -text "directly connect \"pod 1B(0) ch 0\" to \"pod 1A(0) ch 0\" for use as trigger signal"\
                        -anchor w -font $fnt
   }
   wm title .cons "pod connections"

   # close $debug3
}

proc write_ch {} {
   global pg_pod_arr meas_pod_arr bondbar 
   global outp_list inp_list data_name

   set nt [expr 40]

   set f_ch [open $data_name.chs "w"]

   for {set i [expr 0]} {$i < [llength $outp_list]} {incr i} {
      set t_name [lindex [lindex $outp_list $i] 2]
      puts $f_ch "[lindex $meas_pod_arr($t_name) 3] [lindex $meas_pod_arr($t_name) 1] O"
   }

   for {set i [expr 0]} {$i < [llength $inp_list]} {incr i} {
      set t_name [lindex [lindex $inp_list $i] 2]
      puts $f_ch "[lindex $pg_pod_arr($t_name) 3] [lindex $pg_pod_arr($t_name) 1] I"
   }

   close $f_ch
}
