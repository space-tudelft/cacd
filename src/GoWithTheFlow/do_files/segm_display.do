
proc draw_segm_display {widget} {

   set sdw [winfo width $widget.cv]
   set sdh [winfo heigh $widget.cv]
   set dx [expr $sdw/10.]
   set dy [expr $sdh/10.]
   set x1 [expr $dx]
   set x2 [expr 2*$dx]
   set x3 [expr 3*$dx]
   set x4 [expr $sdw - $x3]
   set x5 [expr $sdw - $x2]
   set x6 [expr $sdw - $x1]
   set y1 [expr $dy]
   set y2 [expr $sdh/2]
   set y3 [expr $sdh - $dy]

   $widget.cv delete all
   $widget.cv create line $x3 $y1 $x6 $y1 -fill wheat2 -width 3 -tag seg0
   $widget.cv create line $x6 $y1 $x5 $y2 -fill wheat2 -width 3 -tag seg1
   $widget.cv create line $x5 $y2 $x4 $y3 -fill wheat2 -width 3 -tag seg2
   $widget.cv create line $x4 $y3 $x1 $y3 -fill wheat2 -width 3 -tag seg3
   $widget.cv create line $x1 $y3 $x2 $y2 -fill wheat2 -width 3 -tag seg4
   $widget.cv create line $x2 $y2 $x3 $y1 -fill wheat2 -width 3 -tag seg5
   $widget.cv create line $x2 $y2 $x5 $y2 -fill wheat2 -width 3 -tag seg6

   upd_sd_val $widget
}

proc upd_sd_val {widget} {
   global myglobalvar
   global SD_BITLEN

   set value [examine $myglobalvar($widget:sd_in)]
   for {set i [expr 0]} {$i < $SD_BITLEN} {incr i} {
      if {[string index $value [expr $SD_BITLEN-$i-1]]== "1"} {
         $widget.cv itemconfigure seg$i -fill red
      }\
      else {
         $widget.cv itemconfigure seg$i -fill wheat2
      }
   }
   update
}

proc mk_segm_display {widget} {
   global myglobalvar
   global SD_BITLEN

   set fnt "-*-Courier-Bold-R-Normal--*-100-*"

   set signals [find signals -out -internal /*]
   set possible_signals ""
   foreach s $signals {
      if {[string length [examine $s]] == $SD_BITLEN} {
         set idx [string last "/" $s]
         lappend possible_signals  [string range $s $idx end]
      }
   }
   if {$possible_signals == ""} {
      echo "*****************************************************"
      echo "** Cannot find a suitable signal for segment_display"
      echo "** macro segment_display not created"
      echo "*****************************************************"
   }\
   else {
      toplevel $widget
      canvas   $widget.cv -width 50 -height 80 -bg wheat
      eval tk_optionMenu $widget.sig myglobalvar($widget:sd_in)\
                                              $possible_signals

      pack $widget.cv  -side top -fill both -expand 1
      pack $widget.sig -side bottom -fill x

      wm title $widget [string range $widget 4 end]

      bind $widget.sig.menu <ButtonRelease> "upd_sd_val $widget"
      bindtags $widget.sig.menu [list Menu all $widget.sig.menu]

      bind $widget <Expose> "draw_segm_display $widget"

      bind $widget.cv <Destroy> "sd_remove $widget"

      tkwait visibility $widget.cv
      draw_segm_display $widget

      set lbl [string range $widget 1 end]
      when -label $lbl $myglobalvar($widget:sd_in) "upd_sd_val $widget"
   }
}

proc sd_remove {widget} {
   nowhen [string range $widget 1 end]
}

set SD_BITLEN 7
set chdren [winfo children .]
set i [expr 0]
while {[lsearch -exact $chdren .my:sd_$i] >= 0} {
   incr i
}
mk_segm_display .my:sd_$i
