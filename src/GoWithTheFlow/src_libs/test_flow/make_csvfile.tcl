
# proc scroller {W1 W2 v1 v2} {
# $W1 yview $v1 $v2
# $W2 yview $v1 $v2 
# } 

proc make_csvfile {} {
   global data_name inp_list outp_list csvmessage col_arr PERIOD myMaxLen i_trigsig
   global col_arr pg_pod_arr meas_pod_arr trig_pod check_trig
   global fnt_n fnt_l fnt_s fnt_h StartTime
   global pg_offset message

   set fnt $fnt_n

   if {![winfo exists .csvtxt]} {
      toplevel   .csvtxt
      frame      .csvtxt.menubar      -relief raised -bd 2 -bg gold2
      label      .csvtxt.menubar.pos  -font $fnt -anchor w\
                                     -width 13 -text "cursor: 1.0"\
                                     -padx 10 -pady 5\
                                     -bg gold2 -bd 2 -relief ridge
      menubutton .csvtxt.menubar.file -text "File"\
                                     -menu .csvtxt.menubar.file.cmds\
                                     -bg gold2 -font $fnt
      menubutton .csvtxt.menubar.fnts -text "Fonts"\
                                     -menu .csvtxt.menubar.fnts.cmds\
                                     -bg gold2 -font $fnt
      label      .csvtxt.mlbl        -textvariable csvmessage -bg wheat -bd 2\
                                     -relief ridge -anchor w -padx 5 -pady 5\
                                     -font $fnt
      text       .csvtxt.txti       -width [expr [llength $inp_list] * 2] -height 300 -font $fnt -bg wheat\
                                     -yscrollcommand ".csvtxt.sbi set"
      text       .csvtxt.txto       -width [expr [llength $outp_list] * 2] -height 300 -font $fnt -bg wheat\
                                     -yscrollcommand ".csvtxt.sbo set"
      # scrollbar  .csvtxt.sb           -command "scroller .csvtxt.txti .csvtxt.txto" -bg wheat
      scrollbar  .csvtxt.sbi           -command ".csvtxt.txti yview" -bg wheat
      scrollbar  .csvtxt.sbo           -command ".csvtxt.txto yview" -bg wheat
      menu       .csvtxt.menubar.file.cmds -font $fnt -bg gold2
                 .csvtxt.menubar.file.cmds add command -label "Save_csvfile"\
                                                      -command "write_csvfile"
                 .csvtxt.menubar.file.cmds add command -label "Hide"\
                                                 -command {wm withdraw .csvtxt}
      menu       .csvtxt.menubar.fnts.cmds -font $fnt -bg gold2
                 .csvtxt.menubar.fnts.cmds add command -label "very large"\
                                   -command {.csvtxt.txti configure -font $fnt_h; .csvtxt.txto configure -font $fnt_h}
                 .csvtxt.menubar.fnts.cmds add command -label "large"\
                                   -command {.csvtxt.txti configure -font $fnt_l; .csvtxt.txto configure -font $fnt_l}
                 .csvtxt.menubar.fnts.cmds add command -label "normal"\
                                   -command {.csvtxt.txti configure -font $fnt_n; .csvtxt.txto configure -font $fnt_n}
                 .csvtxt.menubar.fnts.cmds add command -label "small"\
                                   -command {.csvtxt.txti configure -font $fnt_s; .csvtxt.txto configure -font $fnt_s}

      pack .csvtxt.menubar       -side top   -fill x
      pack .csvtxt.menubar.file  -side left
      pack .csvtxt.menubar.fnts  -side left
      pack .csvtxt.menubar.pos   -side right
      pack .csvtxt.mlbl          -side top -fill x    -expand 1
      pack .csvtxt.sbi           -side left -fill y 
      pack .csvtxt.txti          -side left -fill both -expand 1
      pack .csvtxt.sbo           -side left -fill y 
      pack .csvtxt.txto          -side left -fill both -expand 1
      
      set x_csvtxt [winfo x .]
      set y_csvtxt [expr [winfo y .] + [winfo height .] + 60]
      wm geometry .csvtxt 600x400+$x_csvtxt+$y_csvtxt

      .csvtxt.txti tag configure extraSig -foreground blue
      .csvtxt.txto tag configure extraSig -foreground blue

      set csvmessage "generating pattern generator data ....."
      update

      # tkwait visibility .csvtxt.txti
      # tkwait visibility .csvtxt.txto
      .menubar.show.cmds entryconfigure csv_file -state normal

      bind .csvtxt.txti <ButtonRelease-1> {
        .csvtxt.menubar.pos configure -text "cursor: [.csvtxt.txti index insert]"
      }
      bind .csvtxt.txto <ButtonRelease-1> {
        .csvtxt.menubar.pos configure -text "cursor: [.csvtxt.txto index insert]"
      }

      wm protocol .csvtxt WM_DELETE_WINDOW del_csv_win
   }\
   else {
      set message "csv window already exists !"
      update
      return
   }
   wm title .csvtxt "csv"
   update

   # set debug2 [open "debug2" "w"]
   # for {set i [expr 0]} {$i < [llength $inp_list]} {incr i} {
   #    set n_pod [expr 1 + ($i / 8)]
   #    set n_con [expr $i % 8]
   #    set term [lindex [lindex $inp_list $i] 1]
   #    set t_name [lindex [lindex $inp_list $i] 2]
   #    puts $debug2 "[lindex $inp_list $i] $t_name - $pg_pod_arr($t_name)"
   # }
   # close $debug2
   # for {set i [expr 0]} {$i < [llength $inp_list]} {incr i} {
   #   .csvtxt.txti insert end "LABEL [lindex [lindex $inp_list $i] 1], [lindex [lindex $inp_list $i] 2] 1\n"
   # }
   set idx [.reftxt.txt search Clock_column: 1.0]
   set n [lindex [split $idx .] 0]
   set str [.reftxt.txt get $n.0 $n.end]
   set clk_col [lindex $str 2]
   set idx [.reftxt.txt search DATA_SECTION 1.0]
   set idx [.reftxt.txt search "<" $idx]
   set fl  [lindex [split $idx .] 0]
   set idx [.reftxt.txt search -backwards "<" end]
   set ll [lindex [split $idx .] 0]
   set ref_time $StartTime
   #  while {[string length $ref_time] < [expr 2*$myMaxLen]} {
   #     set ref_time "0$ref_time"
   #  }
   set n $fl
   set str [.reftxt.txt get $fl.0 $fl.end]
   set tv [split [string trimleft $str] <]
   set old_val [lindex $tv 1]
   set nr_out 0

   for {set i [expr 0]} {$i <[llength $inp_list]} {incr i} {
      set idx [lindex [lindex $inp_list $i] 0]
      if {[string index $old_val $idx] == "H"} { 
         set check_trig($i) "H"
      }\
      else {
         set check_trig($i) "X"
      }
   }
   set i_trigsig -1
   set old_clk_val ""

   while {($n <= $ll) && ($nr_out < 130000)} {
      set str [.reftxt.txt get $n.0 $n.end]
      set tv [split [string trimleft $str] <]
      set time [lindex $tv 0]
      # while {[string length $time] < [expr 2*$myMaxLen]} {
      #    set time "0$time"
      # }
      set val_str [lindex $tv 1]
      set clk_val [string index $val_str $clk_col]
      if {($clk_val != $old_clk_val) && ($clk_val == "H" || $clk_val == "L")} {
         incr nr_out
         insert_csv_data "" $val_str .csvtxt.txti .csvtxt.txto
      }
      incr n
      set old_clk_val $clk_val
   }

   for {set i [expr 0]} {$i <[llength $inp_list]} {incr i} {
      set idx [lindex [lindex $inp_list $i] 0]
      if {$check_trig($i) == "L"} { 
         set i_trigsig $i
         break
      }
   }
   if {$i_trigsig < 0} {
      for {set i [expr 0]} {$i <[llength $inp_list]} {incr i} {
         set idx [lindex [lindex $inp_list $i] 0]
         if {$check_trig($i) == "H"} { 
            set i_trigsig $i
            break
         }
      }
   }

   for {set i [expr 0]} {$i < [llength $outp_list]} {incr i} {
      set n_pod [expr 1 + (($i + 1) / 8)]
      set n_con [expr ($i + 1) % 8]
      set term [lindex [lindex $outp_list $i] 1]
      set t_name [lindex [lindex $outp_list $i] 2]
      set ch [expr $i + 1]
      set meas_pod_arr($t_name) "${n_pod}B($n_con) $ch $col_arr($n_con) $term"
   }

   if {$i_trigsig >= 0} {
      set pg_offset [expr 0]
   }\
   else {
      # create extra trigger generator signal
      set pg_offset [expr 1]
      set l 1
      set trigsig "1,"
      while {$l <= $nr_out} {
          .csvtxt.txti insert $l.0 $trigsig
          .csvtxt.txti tag add extraSig $l.0 $l.2
          set trigsig "0,"
          incr l
      }
   }

   # create extra trigger output signal
   set l 1
   while {$l <= $nr_out} {
      if {$i_trigsig >= 0} {
         set trigsig [.csvtxt.txti get $l.[expr $i_trigsig * 2] $l.[expr $i_trigsig * 2 + 2]]
      }\
      else {
         set trigsig [.csvtxt.txti get $l.0 $l.2]
      }
      .csvtxt.txto insert $l.0 $trigsig
      .csvtxt.txto tag add extraSig $l.0 $l.2
      incr l
   }

   for {set i [expr 0]} {$i < [llength $inp_list]} {incr i} {
      set n_pod [expr 1 + (($i + $pg_offset) / 8)]
      set n_con [expr ($i + $pg_offset) % 8]
      set term [lindex [lindex $inp_list $i] 1]
      set t_name [lindex [lindex $inp_list $i] 2]
      set ch [expr $i + $pg_offset]
      set pg_pod_arr($t_name) "${n_pod}A($n_con) $ch $col_arr($n_con) $term"
      if {$i_trigsig == [expr $i + $pg_offset]} {
         set trig_pod $pg_pod_arr($t_name)
      }
   }

   .menubar.file.cmds entryconfigure Show_pod_connections -state normal
   set cycles [expr $nr_out/2]
   if {$n <= $ll} {
      set csvmessage "WARNING: pattern data generated for only the maximum of $cycles clock cycles"
   }\
   else {
      set csvmessage "pattern data generated for $cycles clock cycles"
   }
}

proc del_csv_win {} {
   if {[winfo exists .csvtxt]} {
      destroy .csvtxt
      .menubar.show.cmds entryconfigure csv_file -state disabled
   }
}
