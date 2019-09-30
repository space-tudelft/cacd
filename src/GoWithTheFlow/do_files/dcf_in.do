
proc mk_dcf_input {widget} {
   global myglobalvar

   set fnt "-*-Courier-Bold-R-Normal--*-100-*"
   set SigWdth   [expr 1]

   set signals [find signals -in -internal /*]
   set possible_signals ""
   foreach s $signals {
      if {[string length [examine $s]] == $SigWdth} {
         set idx [string last "/" $s]
         lappend possible_signals  [string range $s $idx end]
      }
   }
   if {$possible_signals == ""} {
      echo "**********************************************"
      echo "** Cannot find a suitable signal for dcf_in"
      echo "** macro dcf_in not created"
      echo "**********************************************"
   }\
   else {
      toplevel      $widget
      frame         $widget.frt -relief raised -bg wheat -bd 2
      frame         $widget.fru -relief raised -bg wheat -bd 2
      frame         $widget.frs -relief raised -bg wheat -bd 2
      frame         $widget.frc -relief raised -bg gold -bd 2
      tk_optionMenu $widget.frt.wd myglobalvar($widget:wd)\
                 Mon Tue Wed Thu Fri Sat Sun
      tk_optionMenu $widget.frt.nd1 myglobalvar($widget:nd1) 0 1 2 3
      tk_optionMenu $widget.frt.nd2 myglobalvar($widget:nd2) 0 1 2 3 4 5\
                                                             6 7 8 9
      tk_optionMenu $widget.frt.md myglobalvar($widget:md)\
                 Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
      tk_optionMenu $widget.frt.ny1 myglobalvar($widget:ny1) 0 1 2 3 4 5\
                                                             6 7 8 9
      tk_optionMenu $widget.frt.ny2 myglobalvar($widget:ny2) 0 1 2 3 4 5\
                                                             6 7 8 9
      label         $widget.frt.tlbl -text Time -bg wheat -font $fnt
      tk_optionMenu $widget.frt.nh1 myglobalvar($widget:nh1) 0 1 2
      tk_optionMenu $widget.frt.nh2 myglobalvar($widget:nh2) 0 1 2 3 4 5\
                                                             6 7 8 9
      label         $widget.frt.mlbl -text ":" -bg wheat
      tk_optionMenu $widget.frt.nm1 myglobalvar($widget:nm1) 0 1 2 3 4 5
      tk_optionMenu $widget.frt.nm2 myglobalvar($widget:nm2) 0 1 2 3 4 5\
                                                             6 7 8 9
      label         $widget.fru.lbl1 -text "change width of pulse"\
                                     -bg wheat -font $fnt
      tk_optionMenu $widget.fru.nbr myglobalvar($widget:nbr) 0 1 2 3 4 5 6 7 8\
                                 9 10 11 12 13 14 15 17 17 18 19 20 21 22 23\
                                 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38\
                                 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53\
                                 54 55 56 57 58 59
      label         $widget.fru.lbl2 -text "to" -bg wheat -font $fnt
      tk_optionMenu $widget.fru.val myglobalvar($widget:val) 1 10 20 50 80 90\
                                 100 110 120 150 180 190 200 210 220 250 300 \
                                 400 500 600 700 800 900
      label         $widget.fru.lbl3 -text "ms" -bg wheat -font $fnt
      button        $widget.fru.do -text Doit -bg wheat3 -font $fnt\
                                   -command "update_width $widget"
      label         $widget.frs.lbl1 -text "first pulse at:" -bg wheat\
                                                             -font $fnt
      tk_optionMenu $widget.frs.st myglobalvar($widget:st) 0 1 5 10 50 100\
                                   200 500 800 1000 1500 2000
      label         $widget.frs.lbl2 -text "ms" -bg wheat -font $fnt
      label         $widget.frc.slbl -text "dcf_signal" -bg gold -font $fnt
      label         $widget.frc.tlbl -text "time_scale:" -bg gold -font $fnt
      tk_optionMenu $widget.frc.ts myglobalvar($widget:ts) 1 5 10 50 100 500\
                                                          1000
      button        $widget.frc.ap -text Apply -bg gold3 -font $fnt\
                                   -command "mk_string $widget"
      button        $widget.frc.qt -text Quit -bg gold3 -font $fnt\
                                   -command "destroy $widget"
      eval tk_optionMenu $widget.frc.sig myglobalvar($widget:dcf_in)\
                                              $possible_signals
      wm title $widget [string range $widget 4 end]

      $widget.frt.wd configure -bg wheat3 -font $fnt
      $widget.frt.wd.menu configure -bg wheat3 -font $fnt
      $widget.frt.nd1 configure -bg wheat3 -font $fnt
      $widget.frt.nd1.menu configure -bg wheat3 -font $fnt
      $widget.frt.nd2 configure -bg wheat3 -font $fnt
      $widget.frt.nd2.menu configure -bg wheat3 -font $fnt
      $widget.frt.md configure -bg wheat3 -font $fnt
      $widget.frt.md.menu configure -bg wheat3 -font $fnt
      $widget.frt.ny1 configure -bg wheat3 -font $fnt
      $widget.frt.ny1.menu configure -bg wheat3 -font $fnt
      $widget.frt.ny2 configure -bg wheat3 -font $fnt
      $widget.frt.ny2.menu configure -bg wheat3 -font $fnt
      $widget.frt.nh1 configure -bg wheat3 -font $fnt
      $widget.frt.nh1.menu configure -bg wheat3 -font $fnt
      $widget.frt.nh2 configure -bg wheat3 -font $fnt
      $widget.frt.nh2.menu configure -bg wheat3 -font $fnt
      $widget.frt.nm1 configure -bg wheat3 -font $fnt
      $widget.frt.nm1.menu configure -bg wheat3 -font $fnt
      $widget.frt.nm2 configure -bg wheat3 -font $fnt
      $widget.frt.nm2.menu configure -bg wheat3 -font $fnt
      $widget.frs.st configure -bg wheat3 -font $fnt
      $widget.frs.st.menu configure -bg wheat3 -font $fnt
      $widget.fru.nbr configure -bg wheat3 -font $fnt
      $widget.fru.nbr.menu configure -bg wheat3 -font $fnt
      $widget.fru.nbr.menu entryconfigure 30 -columnbreak 1
      $widget.fru.val configure -bg wheat3 -font $fnt
      $widget.fru.val.menu configure -bg wheat3 -font $fnt
      $widget.frc.ts configure -bg wheat3 -font $fnt -width 4
      $widget.frc.ts.menu configure -bg wheat3 -font $fnt
      $widget.frc.sig configure -bg wheat3 -font $fnt -width 4
      $widget.frc.sig.menu configure -bg wheat3 -font $fnt

      pack $widget.frt      -side top    -fill x
      pack $widget.frc      -side bottom -fill both -expand 1
      pack $widget.frs      -side left
      pack $widget.fru      -side right  -fill x -expand 1
      pack $widget.frt.wd   -side left -pady 10 -padx 10
      pack $widget.frt.nd1  -side left -pady 10
      pack $widget.frt.nd2  -side left -pady 10
      pack $widget.frt.md   -side left -pady 10 -padx 10
      pack $widget.frt.ny1  -side left -pady 10
      pack $widget.frt.ny2  -side left -pady 10
      pack $widget.frt.tlbl -side left -padx 10
      pack $widget.frt.nh1  -side left -pady 10
      pack $widget.frt.nh2  -side left -pady 10
      pack $widget.frt.mlbl -side left -padx 10
      pack $widget.frt.nm1  -side left -pady 10
      pack $widget.frt.nm2  -side left -pady 10
      pack $widget.frs.lbl1 -side left -padx 10 -pady 5
      pack $widget.frs.st   -side left -padx 1 -pady 5
      pack $widget.frs.lbl2 -side left -padx 10 -pady 5
      pack $widget.fru.do   -side right -padx 10 -pady 5
      pack $widget.fru.lbl3 -side right -padx 1 -pady 5
      pack $widget.fru.val  -side right -padx 1 -pady 5
      pack $widget.fru.lbl2 -side right -padx 1 -pady 5
      pack $widget.fru.nbr  -side right -padx 1 -pady 5
      pack $widget.fru.lbl1 -side right -padx 1 -pady 5
      pack $widget.frc.slbl -side left -padx 10 -pady 5
      pack $widget.frc.sig  -side left -padx 10 -fill x -expand 1
      pack $widget.frc.tlbl -side left -padx 10 -pady 5
      pack $widget.frc.ts   -side left -padx 10 -pady 5
      pack $widget.frc.qt   -side right -padx 10 -pady 5
      pack $widget.frc.ap   -side right -padx 10 -pady 5

      set tmptime [clock seconds]
      set myglobalvar($widget:wd)    [clock format $tmptime -format %a]
      set myglobalvar($widget:md)    [clock format $tmptime -format %b]
      set nd                         [clock format $tmptime -format %d]
      set myglobalvar($widget:nd1)   [string range $nd 0 0]
      set myglobalvar($widget:nd2)   [string range $nd 1 1]
      set ny                         [clock format $tmptime -format %y]
      set myglobalvar($widget:ny1)   [string range $ny 0 0]
      set myglobalvar($widget:ny2)   [string range $ny 1 1]
      set nh                         [clock format $tmptime -format %H]
      set myglobalvar($widget:nh1)   [string range $nh 0 0]
      set myglobalvar($widget:nh2)   [string range $nh 1 1]
      set nm                         [clock format $tmptime -format %M]
      set myglobalvar($widget:nm1)   [string range $nm 0 0]
      set myglobalvar($widget:nm2)   [string range $nm 1 1]
      set myglobalvar($widget:nbr)   [expr 0]
      set myglobalvar($widget:val)   [expr 1]
      set myglobalvar($widget:e_nbrs) ""
      set myglobalvar($widget:e_vals) ""
      set myglobalvar($widget:st)    [expr 0]
   }
}

proc update_width {widget} {
   global myglobalvar

   lappend myglobalvar($widget:e_nbrs) $myglobalvar($widget:nbr)
   lappend myglobalvar($widget:e_vals) $myglobalvar($widget:val)
}

proc dcf_in_remove {widget} {
   if {[winfo exists .signals]} {
      .signals.mBar.view delete [string range $widget 4 end]
   }
}

proc dcf_in_add_string {widget } {
    set dcf_in_str [.signals.tree.tree1 get $sel_id]
    set idx1 [string first "/" $dcf_in_str]
    set dcf_in_str [string range $dcf_in_str $idx1 end]
    $widget.frc.en configure -state normal
    $widget.frc.en delete 0 end
    $widget.frc.en insert end $dcf_in_str
    set fr [$widget.frc.en xview]
    set fract [expr 1.0 + [lindex $fr 0] - [lindex $fr 1]]
    $widget.frc.en xview moveto $fract
    $widget.frc.en configure -state disabled
}

proc nbr2string {nbr nb} {
   case $nb {
      1 { case $nbr {
             0 {return "0"}
             1 {return "1"}
          }
        }
      2 { case $nbr {
             0 {return "00"}
             1 {return "10"}
             2 {return "01"}
             3 {return "11"}
          }
        }
      3 { case $nbr {
             0 {return "000"}
             1 {return "100"}
             2 {return "010"}
             3 {return "110"}
             4 {return "001"}
             5 {return "101"}
             6 {return "011"}
             7 {return "111"}
          }
        }
      4 { case $nbr {
             0 {return "0000"}
             1 {return "1000"}
             2 {return "0100"}
             3 {return "1100"}
             4 {return "0010"}
             5 {return "1010"}
             6 {return "0110"}
             7 {return "1110"}
             8 {return "0001"}
             9 {return "1001"}
          }
        }
   }
}

proc mk_par_bit { ref_str first last} {

   set n_ones [expr 0]
   for {set i [expr $first]} {$i <= $last} {incr i} {
      if {[string index $ref_str $i] == "1"} {
         incr n_ones
      }
   }
   if { [expr $n_ones % 2] == 1 } {
      return "1"
   }\
   else {
      return "0"
   }
}

proc day2string {day} {
   case $day {
      "Mon" { return "100"}
      "Tue" { return "010"}
      "Wed" { return "110"}
      "Thu" { return "001"}
      "Fri" { return "101"}
      "Sat" { return "110"}
      "Sun" { return "111"}
   }
}

proc month2string {month} {
   case $month {
      "Jan"  { return "10000"}
      "Feb"  { return "01000"}
      "Mar"  { return "11000"}
      "Apr"  { return "00100"}
      "May"  { return "10100"}
      "Jun"  { return "01100"}
      "Jul"  { return "11100"}
      "Aug"  { return "00010"}
      "Sep"  { return "10010"}
      "Oct"  { return "00001"}
      "Nov"  { return "10001"}
      "Dec"  { return "01001"}
   }
}

proc mk_string {widget} {
   global myglobalvar

   set per [expr 1000000/$myglobalvar($widget:ts)]
   set outstr "100000000000000000001"
   set m2_str [nbr2string $myglobalvar($widget:nm2) 4]
   set m1_str [nbr2string $myglobalvar($widget:nm1) 3]
   set outstr "${outstr}${m2_str}${m1_str}"
   set p1 [mk_par_bit $outstr 0 27]
   set outstr "${outstr}${p1}"
   set h2_str [nbr2string $myglobalvar($widget:nh2) 4]
   set h1_str [nbr2string $myglobalvar($widget:nh1) 2]
   set outstr "${outstr}${h2_str}${h1_str}"
   set p2 [mk_par_bit $outstr 0 34]
   set outstr "${outstr}${p2}"
   set d2_str [nbr2string $myglobalvar($widget:nd2) 4]
   set d1_str [nbr2string $myglobalvar($widget:nd1) 2]
   set outstr "${outstr}${d2_str}${d1_str}"
   set wd_str [day2string $myglobalvar($widget:wd)]
   set outstr "${outstr}${wd_str}"
   set md_str [month2string $myglobalvar($widget:md)]
   set outstr "${outstr}${md_str}"
   set y2_str [nbr2string $myglobalvar($widget:ny2) 4]
   set y1_str [nbr2string $myglobalvar($widget:ny1) 4]
   set outstr "${outstr}${y2_str}${y1_str}"
   set p3 [mk_par_bit $outstr 0 57]
   set outstr "${outstr}${p3}"
   set st [expr 1000*$myglobalvar($widget:st)]
   force $myglobalvar($widget:dcf_in) 0
   set len [string length $outstr]
   for {set i [expr 0]} {$i <= $len} {incr i} {
      set e_idx [lsearch $myglobalvar($widget:e_nbrs) $i]
      if {$e_idx >= 0} {
         set dur [expr 1000*[lindex $myglobalvar($widget:e_vals) $e_idx]]
      }\
      elseif {($i < $len) && ([string index $outstr $i] == 1)} {
         set dur [expr $per/5]
      }\
      elseif {($i < $len) && ([string index $outstr $i] == 0)} {
         set dur [expr $per/10]
      }\
      else {
         set dur [expr 0]
      }
      if {$dur > 0} {
         force $myglobalvar($widget:dcf_in) 1 [expr $st+$i*$per] us,\
                                     0 [expr $st+$i*$per+$dur] us
      }
   }
   set myglobalvar($widget:e_nbrs) ""
   set myglobalvar($widget:e_vals) ""
   set myglobalvar($widget:st) [expr 0]
   run  "[expr $st+60*$per] us"
}

set chdren [winfo children .]
set i [expr 0]
while {[lsearch -exact $chdren .my:dcf_$i] >= 0} {
   incr i
}
mk_dcf_input .my:dcf_$i
