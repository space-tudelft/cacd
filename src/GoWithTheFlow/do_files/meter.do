
proc draw_meter {widget} {
   global myglobalvar

   set mw [winfo width $widget.cv]
   set mh [winfo heigh $widget.cv]
   set fnt "-*-Courier-Bold-R-Normal--*-100-*"
   set x0  [expr $myglobalvar($widget:f_x0)*$mw]
   set y0  [expr $myglobalvar($widget:f_y0)*$mh]
   set r0  [expr $myglobalvar($widget:f_r0)*($mw+$mh)]
   set r1  [expr $myglobalvar($widget:f_r1)*$mh]
   set r1a [expr $myglobalvar($widget:f_r1a)*$mh]
   set r1b [expr $myglobalvar($widget:f_r1b)*$mh]
   set r2  [expr $myglobalvar($widget:f_r2)*$mh]
   set max_val  $myglobalvar($widget:maxval)
   set pi  [expr 3.14159]

   $widget.cv delete all
   $widget.cv create oval [expr $x0-$r0] [expr $y0-$r0]\
                          [expr $x0+$r0] [expr $y0+$r0] -fill black
   set alfa0 [expr 120*$pi/180]
   set alfa1 [expr 60*$pi/180]

   $widget.cv create arc [expr $x0-$r1] [expr $y0-$r1]\
                         [expr $x0+$r1] [expr $y0+$r1]\
                         -extent -60 -outline white -start 120 -style arc
   $widget.cv create text [expr 0.75*$mw] $y0\
                          -text "0" -font $fnt -fill red -tag value
   for {set i [expr 120]} {$i >= 60} {incr i -15} {
      set angle [expr $i*$pi/180]
      $widget.cv create line [expr $x0 + $r1a*cos($angle)]\
                             [expr $y0 - $r1a*sin($angle)]\
                             [expr $x0 + $r1b*cos($angle)]\
                             [expr $y0 - $r1b*sin($angle)]\
                             -fill white
      set val [expr int((120-$i)*$max_val/60)]
      $widget.cv create text [expr $x0 + $r1b*cos($angle)]\
                             [expr $y0 - $r1b*sin($angle)]\
                             -fill white -anchor s -text $val\
                             -font $fnt
   }
   set alfa [expr 120*$pi/180]
   $widget.cv create line $x0 $y0 \
                          [expr $x0 + $r2*cos($alfa)]\
                          [expr $y0 - $r2*sin($alfa)]\
                          -width 2 -tag hand
   upd_hand $widget
}

proc change_signal {widget} {
   global myglobalvar

   set nbr_bits [string length [examine $myglobalvar($widget:meter_in)]]
   set myglobalvar($widget:maxval) [expr pow(2,$nbr_bits)]
   draw_meter $widget
   upd_hand $widget
}

proc upd_hand {widget} {
   global myglobalvar

   set mw [winfo width $widget.cv]
   set mh [winfo heigh $widget.cv]
   set x0  [expr $myglobalvar($widget:f_x0)*$mw]
   set y0  [expr $myglobalvar($widget:f_y0)*$mh]
   set r2  [expr $myglobalvar($widget:f_r2)*$mh]
   set pi  [expr 3.14159]
   set val [examine -unsigned $myglobalvar($widget:meter_in)]
   set max_val $myglobalvar($widget:maxval)
   if {([string first "X" $val] < 0) &&
       ([string first "U" $val] < 0)} {
      set alfa [expr (120-60*$val/$max_val)*$pi/180]
      $widget.cv coords hand  $x0 $y0 \
                              [expr $x0 + $r2*cos($alfa)]\
                              [expr $y0 - $r2*sin($alfa)]
      $widget.cv itemconfigure value -text $val
      update
   }
}

proc mk_meter {widget} {
   global myglobalvar

   set fnt "-*-Courier-Bold-R-Normal--*-100-*"

   set signals [find signals -out -internal /*]
   set possible_signals ""
   foreach s $signals {
      if {[string length [examine $s]] > 1} {
         set idx [string last "/" $s]
         lappend possible_signals  [string range $s $idx end]
      }
   }
   if {$possible_signals == ""} {
      echo "**********************************************"
      echo "** Cannot find a suitable signal for meter"
      echo "** macro meter not created"
      echo "**********************************************"
   }\
   else {
      toplevel $widget
      canvas   $widget.cv -width 150 -height 150 -bg wheat3
      eval tk_optionMenu $widget.sig myglobalvar($widget:meter_in)\
                                              $possible_signals

      pack $widget.cv -side top -fill both -expand 1
      pack $widget.sig -side bottom -fill x

      wm title $widget [string range $widget 4 end]

      bind $widget.sig.menu <ButtonRelease> "change_signal $widget"
      bindtags $widget.sig.menu [list Menu all $widget.sig.menu]

      set myglobalvar($widget:f_x0)  [expr 0.5]
      set myglobalvar($widget:f_y0)  [expr 0.9]
      set myglobalvar($widget:f_r0)  [expr 0.01]
      set myglobalvar($widget:f_r1)  [expr 0.75]
      set myglobalvar($widget:f_r1a) [expr 0.73]
      set myglobalvar($widget:f_r1b) [expr 0.77]
      set myglobalvar($widget:f_r2)  [expr 0.8]
      set nbr_bits [string length [examine $myglobalvar($widget:meter_in)]]
      set myglobalvar($widget:maxval) [expr pow(2,$nbr_bits)]

      bind $widget <Expose> "draw_meter $widget"

      bind $widget.cv <Destroy> "mtr_remove $widget"

      tkwait visibility $widget.cv
      draw_meter $widget
      set lbl [string range $widget 1 end]
      when -label $lbl $myglobalvar($widget:meter_in) "upd_hand $widget"
   }

}

proc mtr_remove {widget} {
   nowhen [string range $widget 1 end]
}

set chdren [winfo children .]
set i [expr 0]
while {[lsearch -exact $chdren .my:mtr_$i] >= 0} {
   incr i
}
mk_meter .my:mtr_$i
