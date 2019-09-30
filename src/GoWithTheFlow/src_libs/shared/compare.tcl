
proc compare {} {
   open_compare_window 0
}

proc compare_offset {} {
   set offset [tf_txt_dialog "Enter offset (clock cycles)" ""]

   if {$offset != ""} {
      open_compare_window $offset
   }
}

proc open_compare_window {offset} {
#-----------------------------------------------------------------------------#
# procedure to generate the compare_window                                    #
#-----------------------------------------------------------------------------#
   global Fnt RefFileName ResFileName MeasFileName data_width
   global inp_list outp_list lb_off clk_offset
   global initialdir cmp_pos
   global StartTime PeriodTime EndTime NrCycles
   global fnt_n fnt_l fnt_s fnt_h
   global ProgName initialdir RefForCompPresent
   global res_file
   global cmp_inout

   set MaxNbrMenuItemsInColumn 40

   set ResFileName ""
   set MeasFileName ""

   set message "generating compare window ..."
   update idletasks

   toplevel   .cmpr

   frame      .cmpr.params -borderwidth 2 -relief raised -bg wheat3
   label      .cmpr.params.lbls   -text "start:" -font $Fnt -bg wheat3
   entry      .cmpr.params.ens    -width 10 -font $Fnt -bg wheat3
   label      .cmpr.params.lble   -text "stop:" -font $Fnt -bg wheat3
   entry      .cmpr.params.ene    -width 12 -font $Fnt -bg wheat3
   label      .cmpr.params.lblp   -text "period:" -font $Fnt -bg wheat3
   entry      .cmpr.params.enp    -width 12 -font $Fnt -bg wheat3 \
                 -readonlybackground wheat3 -highlightthickness 0
   label      .cmpr.params.lblc   -text "cycles:" -font $Fnt -bg wheat3
   entry      .cmpr.params.enc    -width 8 -font $Fnt -bg wheat3 \
                 -readonlybackground wheat3 -highlightthickness 0
   label      .cmpr.params.lblo   -text "shift:" -font $Fnt -bg wheat3
   entry      .cmpr.params.eno    -width 8 -font $Fnt -bg wheat3
   checkbutton .cmpr.params.inout -text "inout" -font $Fnt -bg wheat3 \
                  -variable cmp_inout -onvalue 1 -offvalue 0
   .cmpr.params.inout select

   frame      .cmpr.tfr
   label      .cmpr.tfr.trms -text "terminals" -font $Fnt -bg wheat3\
                             -bd 2 -relief raised -pady 5
   listbox    .cmpr.tfr.lb -bg wheat -height 32 -width 30 -font $Fnt
   frame      .cmpr.menubar -bg gold2 -borderwidth 2 -relief raised
#   frame      .cmpr.params -borderwidth 2 -relief raised -bg wheat3
   menubutton .cmpr.menubar.file -text "File" -menu .cmpr.menubar.file.cmds\
                                 -bg gold2 -font $Fnt

   menu       .cmpr.menubar.file.cmds -font $Fnt -bg wheat3
   if {$ProgName == "design_flow" || $ProgName == "GoWithTheFlow"} {
      global MyWd

      set initialdir $MyWd
      .cmpr.menubar.file.cmds add cascade -label "Read ref"\
                                  -menu .cmpr.menubar.file.cmds.read_ref
      menu       .cmpr.menubar.file.cmds.read_ref -font $Fnt -bg wheat3 -tearoff 0
      set ref_files [lsort [glob -directory $initialdir -nocomplain "*.ref"]]
      for {set i 0} {$i < [llength $ref_files]} {incr i} {
         set i_ref [lindex $ref_files $i]
         .cmpr.menubar.file.cmds.read_ref add command -label [file tail $i_ref]\
                           -command "set RefFileName \"$i_ref\"; read_reffile 0"
         if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
            .cmpr.menubar.file.cmds.read_ref entryconfigure [file tail $i_ref] -columnbreak 1
         }
      }
      .cmpr.menubar.file.cmds.read_ref add command -label <browse>\
                        -command "read_reffile 1"
   }
   .cmpr.menubar.file.cmds add cascade -label "Compare res"\
                        -state disabled -menu .cmpr.menubar.file.cmds.read_res
   menu       .cmpr.menubar.file.cmds.read_res -font $Fnt -bg wheat3 -tearoff 0
   set res_files [lsort [glob -directory $initialdir -nocomplain "*.res"]]
   for {set i 0} {$i < [llength $res_files]} {incr i} {
      set i_res [lindex $res_files $i]
      .cmpr.menubar.file.cmds.read_res add command -label [file tail $i_res]\
                           -command "set res_file 1; set MeasFileName \"$i_res\"; read_actfile 0"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .cmpr.menubar.file.cmds.read_res entryconfigure [file tail $i_res] -columnbreak 1
      }
   }
   .cmpr.menubar.file.cmds.read_res add command -label <browse>\
                        -command "set res_file 1; read_actfile 1"
   .cmpr.menubar.file.cmds add cascade -label "Compare csv"\
                        -state disabled -menu .cmpr.menubar.file.cmds.read_csv
   menu       .cmpr.menubar.file.cmds.read_csv -font $Fnt -bg wheat3 -tearoff 0
   set csv_files [lsort [glob -directory $initialdir -nocomplain "*.csv"]]
   for {set i 0} {$i < [llength $csv_files]} {incr i} {
      set i_csv [lindex $csv_files $i]
      .cmpr.menubar.file.cmds.read_csv add command -label [file tail $i_csv]\
                           -command "set res_file 0; set MeasFileName \"$i_csv\"; read_actfile 0"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .cmpr.menubar.file.cmds.read_csv entryconfigure [file tail $i_csv] -columnbreak 1
      }
   }
   .cmpr.menubar.file.cmds.read_csv add command -label <browse>\
                        -command "set res_file 0; read_actfile 1"

              .cmpr.menubar.file.cmds add command -label "Compare again"\
                                  -state disabled\
                                  -command "read_actfile 0"
              .cmpr.menubar.file.cmds add command -label "Write results"\
                                  -state disabled\
                                  -command "write_comparison"
              .cmpr.menubar.file.cmds add command -label "Quit"\
                                  -command "destroy .cmpr"
   menubutton .cmpr.menubar.cmd   -text "Commands"\
                                  -menu .cmpr.menubar.cmd.cmds\
                                  -bg gold2 -font $Fnt -state disabled
   menubutton .cmpr.menubar.fnts -text "Fonts"\
                                  -menu .cmpr.menubar.fnts.cmds\
                                  -bg gold2 -font $Fnt
   menu       .cmpr.menubar.cmd.cmds -font $Fnt -bg wheat3
              .cmpr.menubar.cmd.cmds add command -label "First_error"\
                                  -command "first_error"
              .cmpr.menubar.cmd.cmds add command -label "Next_error"\
                                  -command "next_error"
              .cmpr.menubar.cmd.cmds add command -label "Prev_error"\
                                  -command "prev_error"
              .cmpr.menubar.cmd.cmds add command -label "Last_error"\
                                  -command "last_error"
   label      .cmpr.menubar.indx -text "1.0" -bg gold2 -font $Fnt
   text       .cmpr.txt -width 100 -height 25  -yscrollcommand ".cmpr.sb set"\
                        -xscrollcommand ".cmpr.sbx set" -wrap none\
                        -font $Fnt -bg wheat
   text       .cmpr.itxt -width 100 -height 2 -font $Fnt -bg wheat -padx 5
   scrollbar  .cmpr.sb -command ".cmpr.txt yview" -bg wheat
   scrollbar  .cmpr.sbx -command ".cmpr.txt xview" -bg wheat -orient horizontal
   text       .cmpr.etxt -height 1 -font $Fnt -bg wheat3
#   label      .cmpr.params.blbl -text time -font $Fnt -padx 10 -bg wheat3
#   entry      .cmpr.params.ben -width 15 -font $Fnt -bg wheat3
#   label      .cmpr.params.dlbl -text period -font $Fnt -padx 10 -bg wheat3
#   entry      .cmpr.params.den -width 15 -font $Fnt -bg wheat3
#   label      .cmpr.params.elbl -text stop -font $Fnt -padx 10 -bg wheat3
#   entry      .cmpr.params.een -width 15 -font $Fnt -bg wheat3
   menu       .cmpr.menubar.fnts.cmds -font $Fnt -bg gold2
              .cmpr.menubar.fnts.cmds add command -label "very large"\
                                 -command {.cmpr.txt configure -font $fnt_h}
              .cmpr.menubar.fnts.cmds add command -label "large"\
                                 -command {.cmpr.txt configure -font $fnt_l}
              .cmpr.menubar.fnts.cmds add command -label "normal"\
                                 -command {.cmpr.txt configure -font $fnt_n}
              .cmpr.menubar.fnts.cmds add command -label "small"\
                                 -command {.cmpr.txt configure -font $fnt_s}

   pack .cmpr.menubar -side top -fill x
   pack .cmpr.tfr -side right -fill y
   pack .cmpr.tfr.trms -side top -fill x
   pack .cmpr.tfr.lb -side top -fill y -expand 1
#   pack .cmpr.params -side top -fill x
#   pack .cmpr.params.blbl -side left
#   pack .cmpr.params.ben -side left
#   pack .cmpr.params.dlbl -side left
#   pack .cmpr.params.den -side left
#   pack .cmpr.params.elbl -side left
#   pack .cmpr.params.een -side left
   pack .cmpr.params           -side top    -fill x
   pack .cmpr.params.lbls      -side left   -padx 4
   pack .cmpr.params.ens       -side left   -padx 4
   pack .cmpr.params.lble      -side left   -padx 4
   pack .cmpr.params.ene       -side left   -padx 4
   pack .cmpr.params.lblp      -side left   -padx 4
   pack .cmpr.params.enp       -side left   -padx 4
   pack .cmpr.params.lblc      -side left   -padx 4
   pack .cmpr.params.enc       -side left   -padx 4
   pack .cmpr.params.lblo      -side left   -padx 4
   pack .cmpr.params.eno       -side left   -padx 4
   pack .cmpr.params.inout       -side left   -padx 4

   pack .cmpr.etxt     -side bottom -fill x
   pack .cmpr.sbx     -side bottom -fill x
   pack .cmpr.itxt     -side top -anchor w -fill x
   pack .cmpr.sb      -side right -fill y
   pack .cmpr.txt     -side left -fill both -expand 1
   pack .cmpr.menubar.file -side left
   pack .cmpr.menubar.cmd -side left
   pack .cmpr.menubar.fnts -side left
   pack .cmpr.menubar.indx -side right

   bind .cmpr.txt <ButtonRelease-1>\
             { .cmpr.menubar.indx configure -text [.cmpr.txt index insert]}
   .cmpr.txt tag bind testErr <ButtonRelease-1> { select_test }
   .cmpr.txt tag bind testOK <ButtonRelease-1> { select_test }

   if {$RefForCompPresent} {
      .cmpr.menubar.file.cmds entryconfigure "Compare res" -state normal
      .cmpr.menubar.file.cmds entryconfigure "Compare csv" -state normal
   }
   if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
      .menubar.show.cmds entryconfigure compare -state normal
   }

   wm  title .cmpr "compare [file tail $RefFileName]"
   update idletasks

   set W [winfo width .cmpr.txt]
   set dW [expr $W/[.cmpr.txt cget -width].]
   set tab1 [expr int($dW*15)]
   set tab2 [expr int($dW*17)]
   set tab3 [expr int($dW*51)]
   set tab4 [expr int($dW*85)]
   set cfl ""
   set cfl [lappend cfl $tab1 right]
   set cfl [lappend cfl $tab2]
   set cfl [lappend cfl $tab3]
   set cfl [lappend cfl $tab4]


   .cmpr.txt configure -tabs $cfl
   .cmpr.itxt configure -tabs $cfl
   .cmpr.txt tag configure testOK
   .cmpr.txt tag configure testErr -foreground red
   .cmpr.txt tag configure sel_err -background gray80
   .cmpr.itxt tag configure hlterm -background gray80
   .cmpr.itxt tag configure err_bit -foreground red
   .cmpr.itxt tag configure OK_bit
   # .cmpr.itxt tag bind err_bit <ButtonPress-1> {show_terminal %x %y}
   bind .cmpr.itxt <ButtonPress-1> {show_terminal %x %y}
   bind .cmpr.tfr.lb <ButtonRelease-1> {upd_position}

   if {$RefForCompPresent} {
      .cmpr.params.ens insert end $StartTime
      .cmpr.params.ene insert end $EndTime
      .cmpr.params.enp insert end $PeriodTime
      .cmpr.params.enc insert end $NrCycles
      .cmpr.params.eno insert end "0"
   }
   .cmpr.params.enp configure -state readonly
   .cmpr.params.enc configure -state readonly
}

proc read_actfile {browse} {

   global Fnt RefFileName ResFileName MeasFileName data_width
   global inp_list outp_list lb_off clk_offset
   global initialdir cmp_pos
   global StartTime PeriodTime EndTime NrCycles
   global res_file
   global cmp_inout
   global outp_type

# This hangs the program when done for 2nd time !!!!
   .cmpr.txt delete 1.0 end

   if {$browse == 1} {
      if {$res_file} {
         set types {
            {{RES Files} {.res .RES}}
            {{All Files}  *         }
         }
      } else {
         set types {
            {{CSV Files} {.csv .CSV}}
            {{All Files}  *         }
         }
      }
      set MeasFileName [tk_getOpenFile -initialdir $initialdir -filetypes $types]
      if {$MeasFileName == ""} return
   }\
   else {
      if {$MeasFileName == ""} {
         .cmpr.etxt delete 1.0 end
         .cmpr.etxt insert end "first read an actual results file"
         return
      }
   }

   .cmpr.tfr.lb delete 0 end

   set cmp_pos 1.end

   set type "INIT"
   set i_sig 0
   set clk_start_found "False"
   set old_clk_val ""
   set o2val_str ""

   set idx [.reftxt.txt search -backwards "<" end]
   set ll [lindex [split $idx .] 0]
   set nl 1
   set ol 0
   set ecnt 0
   set nc 0

   set f_meas [open $MeasFileName "r"]

   set d_r_t_s [expr -1.0e99]
   set res_time_str "0"
   set res_val_str "0"
   set cycle 0

   set ct 0
   set period_time [.cmpr.params.enp get]
   if {$period_time == "0"} {
      .cmpr.etxt insert end "set the value for time_step first"
      return
   }
   set steptime [expr $period_time/2.0]

   set start_time [.cmpr.params.ens get]
   set end_time [.cmpr.params.ene get]
   set offset [expr int([.cmpr.params.eno get])]
   set csv_offset [expr $offset * 2]
   set old_o2val_str ""
   set o2val_str ""
   set old_oval_str ""
   set oval_str ""
   set old_ival_str ""

   .cmpr.etxt delete 1.0 end

   while {$nl <= $ll} {
      if {[expr $nl % 1000] == 0} {
         set part_ready [expr ($nl * 100) / $ll]
         .cmpr.etxt delete 1.0 end
         .cmpr.etxt insert end "$part_ready % ready"
         update idletasks
      }
      set txt [.reftxt.txt get $nl.0 $nl.end]
      if {$txt == ""} {
         continue
      }\
      elseif {([lindex $txt 0] == "INP") || ([lindex $txt 0] == "OUT")
              || ([lindex $txt 0] == "INOUT")} {
         set term_type($i_sig) [lindex $txt 0]
         set term_name($i_sig) [string trimright [lindex $txt 1] ";"]
         incr i_sig
      }\
      elseif {[lindex $txt 0] == "TABLE_FORMAT"} {
      }\
      elseif {$txt == "DATA_SECTION"} {
         set type "DATA"
         set lb_off 1

         for {set i 0} {$i < [llength $inp_list]} {incr i} {
            if {[llength [lindex $inp_list $i]] > 2} {
               set term [lindex [lindex $inp_list $i] 1]
               set t_name [lindex [lindex $inp_list $i] 2]
            } else {
               set term ""
               set t_name [lindex [lindex $inp_list $i] 1]
            }
            .cmpr.tfr.lb insert end [format "%-20s %-10s" $t_name $term]
            incr lb_off
         }

         .cmpr.tfr.lb insert end ""

         for {set i 0} {$i < [llength $outp_list]} {incr i} {
            if {$outp_type($i) == "OUT" || ($outp_type($i) == "INOUT" && $cmp_inout)} {
               if {[llength [lindex $outp_list $i]] > 2} {
                  set term [lindex [lindex $outp_list $i] 1]
                  set t_name [lindex [lindex $outp_list $i] 2]
               }\
               else {
                  set term ""
                  set t_name [lindex [lindex $outp_list $i] 1]
               }
               .cmpr.tfr.lb insert end [format "%-20s %-10s" $t_name $term]
            }
         }
      }\
      elseif {[lindex $txt 0] == "PERIOD"} {
#  fill the time_step entry
#         .cmpr.params.den delete 0 end
#         .cmpr.params.den insert end "[string trimright [lindex $txt 1] "ns;"]0"
      }\
      elseif {([lindex $txt 0] == "#") &&\
              ([lindex $txt 1] == "Clock_column:")} {
              set clk_col [lindex $txt 2]
      }\
      elseif { $type == "DATA"} {
         set time_str [string trimright [lindex $txt 0] "<lhxLHX"]
         set val_str [string trimleft [lindex $txt 0] "0123456789<"]

         set ival_str ""
         set oval_str ""
         set oval_mask ""
         for {set i 0} {$i < [string length $val_str]} {incr i} {
            set char [string index $val_str $i]
            set usethisoval 0
            if {$term_type($i) == "OUT" || $term_type($i) == "INOUT"} {
               if {$term_type($i) == "OUT" || ($term_type($i) == "INOUT" && $cmp_inout)} {
                  set usethisoval 1
                  set oval_mask "${oval_mask}1"
               } else {
                  set oval_mask "${oval_mask}0"
               }
            }
            if {($char == "h") || ($char == "H")} {
               if {$term_type($i) == "INP"} {
                  set ival_str "${ival_str}1"
               } elseif {$usethisoval} {
                  set oval_str "${oval_str}1"
               }
            }\
            elseif {($char == "l") || ($char == "L")} {
               if {$term_type($i) == "INP"} {
                  set ival_str "${ival_str}0"
               } elseif {$usethisoval} {
                  set oval_str "${oval_str}0"
               }
            }\
            elseif {($char == "x") || ($char == "X")} {
               if {$term_type($i) == "INP"} {
                  set ival_str "${ival_str}x"
               } elseif {$usethisoval} {
                  set oval_str "${oval_str}x"
               }
            }\
            else {
               if {$term_type($i) == "INP"} {
                  set ival_str "${ival_str}${char}"
               } elseif {$usethisoval} {
                  set oval_str "${oval_str}${char}"
               }
            }
         }

         set data_width [string length $oval_str]
         if {$clk_col >= 0} {
            set clk_val [string index $val_str $clk_col]
         }\
         else {
            # no clock signal in ref file
            if {$old_ival_str != $ival_str} {
               # enforce comparsion
               set clk_val "H"
               set old_clk_val "L"
            }\
            else {
               set clk_val "U"
               set old_clk_val "U"
            }
         }

         if {($clk_start_found == "False") &&\
             ($clk_val == "H")} {
            set clk_offset $time_str
            set clk_start_found "True"
         }

         if {($clk_val != $old_clk_val) && ($clk_val == "H" || $clk_val == "L")} {
            # read new o2val_str from f_meas
            if {$res_file} {
              set d_t_s [expr double($time_str.0)]
              # d_t_s is time in reference file
              # d_r_t_s is time in res or csv file
# .cmpr.txt insert end "$d_r_t_s $d_t_s [expr double($offset.0 * $period_time.0)]\n"
	      while {($d_r_t_s < ($d_t_s - [expr double($offset.0 * $period_time.0)])) && ([eof $f_meas] == 0)} {
		 set txt [gets $f_meas]
                 if { [llength $txt] == 1} {
                    set res_time_str [string trimright [lindex $txt 0] "lhx"]
                    # unit must 1e-10 sec.
                    switch $rf {
                    0 {set res_time_str [expr $res_time_str / 100]}
                    1 {set res_time_str [expr $res_time_str / 10]}
                    2 {}
                    3 {append res_time_str "0"}
                    4 {append res_time_str "00"}
                    5 {append res_time_str "000"}
                    6 {append res_time_str "0000"}
                    7 {append res_time_str "00000"}
                    }
                    set res_val_str [string trimleft [lindex $txt 0] "0123456789"]
                    set d_r_t_s  [expr double($res_time_str.0)]
                 }\
                 elseif {[llength $txt] > 1} {
                    set exponent [lindex [split $txt "eE "] 1]
                    # change e.g. -08 to -8
                    regsub -all -- {\-0+} $exponent {-} exponent
                    regsub -all -- {\+0+} $exponent {+} exponent
                    set rf [expr 12 + ($exponent)]
                 }
                 set old_o2val_str $o2val_str
                 set o2val_str ""
                 for {set i 0} {$i < [string length $res_val_str]
                                       && $i < [array size term_type]} {incr i} {
                    if {$term_type($i) == "OUT" || $term_type($i) == "INOUT"} {
                       set char [string index $res_val_str $i]
                       if {($char == "h")} {
                          set o2val_str "${o2val_str}1"
                       }\
                       elseif {($char == "l")} {
                          set o2val_str "${o2val_str}0"
                       }\
                       elseif {($char == "x")} {
                          set o2val_str "${o2val_str}x"
                       }
                    }
                 }
# .cmpr.txt insert end "$txt [llength $txt] $d_r_t_s $d_t_s [expr double($offset.0 * $period_time.0)]\n"
              }
              if {$d_r_t_s < ($d_t_s - [expr double($offset.0 * $period_time.0)])} {
                 set old_o2val_str $o2val_str
                 set o2val_str ""
              }
            }\
            else {
               set old_o2val_str $o2val_str
               while {$csv_offset < 0} {
                  set txt [gets $f_meas]
                  while {[string index $txt 0] == "#"} {
                     set txt [gets $f_meas]
                  }
                  incr csv_offset
               }
               if {$csv_offset == 0} {
                  set txt [gets $f_meas]
                  while {[string index $txt 0] == "#"} {
                     set txt [gets $f_meas]
                  }
               }\
               else {
                  set txt ""
                  incr csv_offset -1
               }
               if { [llength $txt] == 1} {
                  set o2val_str [string trimleft [lindex $txt 0] ""]
                  regsub -all "," $o2val_str "" o2val_str
                  set data_width [string length $o2val_str]
                  set ct [expr double($ct + $steptime)]
               }\
               else {
                  set o2val_str ""
               }
            }
         }

         if {$clk_val == "H" && $old_clk_val == "L"} {
            incr cycle
         }

         if {$clk_val == "H" && $old_clk_val == "L" \
             && ($time_str > $start_time) && ($time_str <= $end_time)} {
            # compare outputs just before positive clock edge
            incr ol

            # select only outputs that need to be compared
            set tmp_str $old_o2val_str
            set old_o2val_str ""
            set j 0
            for {set i 0} {$i < [array size term_type]} {incr i} {
               if {$term_type($i) == "OUT" || $term_type($i) == "INOUT"} {
                  if {$j < [string length $tmp_str]
                      && $j < [string length $oval_mask] && [string index $oval_mask $j] == "1"} {
                     set f [string index $tmp_str $j]
                     set old_o2val_str "${old_o2val_str}$f"
                  }
                  incr j
               }
            }

            if {$old_o2val_str == "" || $old_o2val_str == [format "%${data_width}s" ""] ||\
                $old_oval_str == "" || $old_oval_str == [format "%${data_width}s" ""]} {
               if {$old_o2val_str == ""} {
                  set old_o2val_str [format "%${data_width}s" ""]
               }
               if {$old_oval_str == ""} {
                  set old_oval_str [format "%${data_width}s" ""]
               }
               .cmpr.txt insert end [format " %12s %6d %s %s %s"\
                                      $time_str $cycle $old_ival_str $old_oval_str $old_o2val_str]
               .cmpr.txt insert end "         \n"
            }\
            elseif {$old_o2val_str != $old_oval_str} {
               .cmpr.txt insert end [format " %12s %6d %s %s %s"\
                                      $time_str $cycle $old_ival_str $old_oval_str $old_o2val_str]
               .cmpr.txt insert end "  error  \n"
               set ref_pos [expr 22 + [string length $old_ival_str]]
               set res_pos [expr [string length $old_oval_str] + 1 + $ref_pos]
               .cmpr.txt tag add testOK $ol.0 $ol.$ref_pos
               for {set i 0} {$i < [string length $old_oval_str]} {incr i} {
                   if {[string index $old_o2val_str $i] != [string index $old_oval_str $i]} {
                      .cmpr.txt tag add testErr $ol.[expr $ref_pos+$i]
                      .cmpr.txt tag add testErr $ol.[expr $res_pos+$i]
                   } else {
                      .cmpr.txt tag add testOK $ol.[expr $ref_pos+$i]
                      .cmpr.txt tag add testOK $ol.[expr $res_pos+$i]
                   }
               }
               .cmpr.txt tag add testErr $ol.[expr $res_pos+$i] $ol.end
               incr ecnt
               incr nc
            }\
            else {
               .cmpr.txt insert end [format " %12s %6d %s %s %s"\
                                      $time_str $cycle $old_ival_str $old_oval_str $old_o2val_str]
               .cmpr.txt insert end "  OK     \n"
               .cmpr.txt tag add testOK $ol.0 $ol.end
               incr nc
            }
         }

         set old_clk_val $clk_val
         set old_time_str $time_str
         set old_ival_str $ival_str
         set old_oval_str $oval_str
      }
      incr nl
   }
   close $f_meas

   .cmpr.menubar.cmd configure -state normal
   .cmpr.menubar.file.cmds entryconfigure "Compare again" -state normal
   .cmpr.menubar.file.cmds entryconfigure "Write results" -state normal

   .cmpr.etxt delete 1.0 end
   .cmpr.etxt insert end "$nc comparisons done: $ecnt errors found"

   wm  title .cmpr "compare [file tail $RefFileName] [file tail $MeasFileName]"
   set message "comparison ready"
   update idletasks
}


proc first_error {} {
#-----------------------------------------------------------------------------#
# procedure to show the first error_line                                      #
#-----------------------------------------------------------------------------#
   global cmp_pos

   set pos [.cmpr.txt search -forwards "error" 1.0 end]
   if {$pos == ""} {
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "No errors"
   }\
   else {
      # .cmpr.etxt delete 1.0 end
      .cmpr.itxt delete 1.0 end
      set line [lindex [split $pos .] 0]
      set val_r [lindex [.cmpr.txt get $line.0 $line.end] 3]
      set val_s [lindex [.cmpr.txt get $line.0 $line.end] 4]
      .cmpr.itxt insert end "$val_r\n$val_s"
      .cmpr.txt tag remove sel_err 1.0 end
      .cmpr.txt tag add sel_err $line.0 $line.end
      .cmpr.txt see $pos
      set cmp_pos $line.end
      show_error_bits
   }
}

proc next_error {} {
#-----------------------------------------------------------------------------#
# procedure to show the next error_line                                       #
#-----------------------------------------------------------------------------#
   global cmp_pos

   if {[lindex [split $cmp_pos .] 1] == 0} {
      set cmp_pos [lindex [split $cmp_pos .] 0].end
   }
   set pos [.cmpr.txt search -forwards "error" $cmp_pos end]
   if {$pos == ""} {
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "No more errors"
   }\
   else {
      # .cmpr.etxt delete 1.0 end
      .cmpr.itxt delete 1.0 end
      set line [lindex [split $pos .] 0]
      set val_r [lindex [.cmpr.txt get $line.0 $line.end] 3]
      set val_s [lindex [.cmpr.txt get $line.0 $line.end] 4]
      .cmpr.itxt insert end "$val_r\n$val_s"
      .cmpr.txt tag remove sel_err 1.0 end
      .cmpr.txt tag add sel_err $line.0 $line.end
      .cmpr.txt see $pos
      set cmp_pos $line.end
      show_error_bits
   }
}

proc last_error {} {
#-----------------------------------------------------------------------------#
# procedure to show the last error_line                                       #
#-----------------------------------------------------------------------------#
   global cmp_pos

   set pos [.cmpr.txt search -backwards "error" end 1.0]
   if {$pos == ""} {
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "No errors"
   }\
   else {
      # .cmpr.etxt delete 1.0 end
      .cmpr.itxt delete 1.0 end
      set line [lindex [split $pos .] 0]
      set val_r [lindex [.cmpr.txt get $line.0 $line.end] 3]
      set val_s [lindex [.cmpr.txt get $line.0 $line.end] 4]
      .cmpr.itxt insert end "$val_r\n$val_s"
      .cmpr.txt tag remove sel_err 1.0 end
      .cmpr.txt tag add sel_err $line.0 $line.end
      .cmpr.txt see $pos
      set cmp_pos $line.0
      show_error_bits
   }
}

proc prev_error {} {
#-----------------------------------------------------------------------------#
# procedure to show the previous error_line                                   #
#-----------------------------------------------------------------------------#
   global cmp_pos

   if {[lindex [split $cmp_pos .] 1] != 0} {
      set cmp_pos [lindex [split $cmp_pos .] 0].0
   }
   set pos [.cmpr.txt search -backwards "error" $cmp_pos 1.0]
   if {$pos == ""} {
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "No more errors"
   }\
   else {
      # .cmpr.etxt delete 1.0 end
      .cmpr.itxt delete 1.0 end
      set line [lindex [split $pos .] 0]
      set val_r [lindex [.cmpr.txt get $line.0 $line.end] 3]
      set val_s [lindex [.cmpr.txt get $line.0 $line.end] 4]
      .cmpr.itxt insert end "$val_r\n$val_s"
      .cmpr.txt tag remove sel_err 1.0 end
      .cmpr.txt tag add sel_err $line.0 $line.end
      .cmpr.txt see $pos
      set cmp_pos $line.0
      show_error_bits
   }
}

proc upd_position {} {
#-----------------------------------------------------------------------------#
# procedure to update the hilite position                                     #
#-----------------------------------------------------------------------------#
   global lb_off

   set pos [.cmpr.tfr.lb curselection]
   set pos [expr $pos-$lb_off]
   .cmpr.itxt tag remove hlterm 1.0 end
   .cmpr.itxt tag add hlterm 1.$pos 1.[expr $pos+1] \
                             2.$pos 2.[expr $pos+1]
}

proc select_test {} {
   global cmp_pos

   .cmpr.itxt delete 1.0 end
   set line [lindex [split [.cmpr.txt index insert] .] 0]
   set val_r [lindex [.cmpr.txt get $line.0 $line.end] 3]
   set val_s [lindex [.cmpr.txt get $line.0 $line.end] 4]
   .cmpr.itxt insert end "$val_r\n$val_s"
   .cmpr.txt tag remove sel_err 1.0 end
   .cmpr.txt tag add sel_err $line.0 $line.end
   set cmp_pos $line.end
   show_error_bits
}

proc show_error_bits {} {
#-----------------------------------------------------------------------------#
# procedure to hilite the bits that are wrong in the itxt_window              #
#-----------------------------------------------------------------------------#
   global data_width

   for {set i 0} {$i < $data_width} {incr i} {
      if {[.cmpr.itxt get 1.$i] != [.cmpr.itxt get 2.$i]} {
         .cmpr.itxt tag add err_bit 1.$i
         .cmpr.itxt tag add err_bit 2.$i
      } else {
         .cmpr.itxt tag add OK_bit 1.$i
         .cmpr.itxt tag add OK_bit 2.$i
      }
   }
}

proc show_terminal {xpic ypic} {
#-----------------------------------------------------------------------------#
# procedure to hilite the terminal that belongs to a bit in the itxt window   #
#-----------------------------------------------------------------------------#
   global lb_off

   set idx [.cmpr.itxt index @$xpic,$ypic]
   set pos [lindex [split $idx .] 1]
   .cmpr.tfr.lb selection clear 0 end
   .cmpr.tfr.lb selection set [expr $pos+$lb_off] [expr $pos+$lb_off]
   .cmpr.tfr.lb see [expr $pos+$lb_off]
   .cmpr.itxt tag remove hlterm 1.0 end
   .cmpr.itxt tag add hlterm 1.$pos 1.[expr $pos+1] \
                             2.$pos 2.[expr $pos+1]
}

proc write_comparison {} {
   global data_name message

   set message "writing comparison data to file $data_name.cmp"
   update idletasks
   set f_cmp [open $data_name.cmp "w"]
   puts $f_cmp [.cmpr.txt get 1.0 end]
   close $f_cmp
   set message "comparison data written to file $data_name.cmp"
   update idletasks
}

