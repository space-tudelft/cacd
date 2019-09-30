
proc read_reffile {browse} {
   global StartTime PeriodTime EndTime NrCycles
   global data_name ref_tail inp_list outp_list ref_is_buf_updated message tmp_file_name
   global CLOCK PERIOD initialdir RefFileName RefForCompPresent
   global buf_file_name buf_file_read
   global fnt_n fnt_l fnt_s fnt_h
   global ProgName
   global n_outp outp_type

   set message ""

   if {$browse} {
      set types {
         {{REF Files} {.ref .REF}}
         {{All Files}   *        }
      }
      set tmp_file_name [tk_getOpenFile -filetypes $types -title "Open .ref file"]
      if {$tmp_file_name != ""} {
         set RefFileName $tmp_file_name
         set data_name [file rootname $tmp_file_name]
         set initialdir [file dirname $tmp_file_name]
      } else {
         set message "no ref_file specified"
         return
      }
   }\
   else {
      if {$RefFileName == ""} {
         set message "no ref_file specified"
         return
      }
      set tmp_file_name $RefFileName
      set data_name [file rootname $tmp_file_name]
   }
   set ref_tail [file tail $tmp_file_name]

   set fnt $fnt_n

   if {![winfo exists .reftxt]} {
      toplevel   .reftxt
      frame      .reftxt.menubar      -relief raised -bd 2 -bg gold2
      label      .reftxt.menubar.pos  -font $fnt -anchor w\
                                      -width 13 -text "cursor: 1.0"\
                                      -padx 10 -pady 5\
                                      -bg gold -bd 2 -relief ridge
      menubutton .reftxt.menubar.file -text "File"\
                                      -menu .reftxt.menubar.file.cmds\
                                      -bg gold2 -font $fnt
      menubutton .reftxt.menubar.fnts -text "Fonts"\
                                      -menu .reftxt.menubar.fnts.cmds\
                                      -bg gold2 -font $fnt
      text       .reftxt.txt          -width 75 -height 30 -font $fnt -bg wheat\
                                      -yscrollcommand ".reftxt.sb set"
      scrollbar  .reftxt.sb           -command ".reftxt.txt yview" -bg wheat
      menu       .reftxt.menubar.file.cmds             -font $fnt -bg gold2
                 .reftxt.menubar.file.cmds add command -label "Hide"\
                                                -command {wm withdraw .reftxt}
      menu       .reftxt.menubar.fnts.cmds             -font $fnt -bg gold2
                 .reftxt.menubar.fnts.cmds add command -label "very large"\
                                   -command {.reftxt.txt configure -font $fnt_h}
                 .reftxt.menubar.fnts.cmds add command -label "large"\
                                   -command {.reftxt.txt configure -font $fnt_l}
                 .reftxt.menubar.fnts.cmds add command -label "normal"\
                                   -command {.reftxt.txt configure -font $fnt_n}
                 .reftxt.menubar.fnts.cmds add command -label "small"\
                                   -command {.reftxt.txt configure -font $fnt_s}

      pack .reftxt.menubar       -side top   -fill x
      pack .reftxt.menubar.file  -side left
      pack .reftxt.menubar.fnts  -side left
      pack .reftxt.menubar.pos   -side right
      pack .reftxt.sb            -side left -fill y
      pack .reftxt.txt           -side left -fill both -expand 1

      set x_reftxt [winfo x .]
      set y_reftxt [expr [winfo y .] + [winfo height .] + 10]
      wm geometry .reftxt +$x_reftxt+$y_reftxt
      wm withdraw .reftxt

      if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
         .menubar.show.cmds entryconfigure ref_file -state normal
      }

      bind .reftxt.txt <ButtonRelease-1> {
        .reftxt.menubar.pos configure -text "cursor: [.reftxt.txt index insert]"
      }

      wm protocol .reftxt WM_DELETE_WINDOW {#}
   }\
   else {
      .reftxt.txt delete 1.0 end
   }

   set ref_is_buf_updated "yes"
   set buf_file_read "no"
   set data_section "n"
   wm title .reftxt "$ref_tail"
   set message "reading file $tmp_file_name ....."
   update idletasks
   if {$tmp_file_name != ""} {
      set f_tex [open $tmp_file_name "r"]
      .reftxt.txt delete 1.0 end
      if {[winfo exists .pgtxt.txt]} {
         .pgtxt.txt delete 1.0 end
      }
      set inp_list ""
      set outp_list ""
      set n_term 0
      set n_outp 0
      while {![eof $f_tex]} {
         set txt [gets $f_tex]
         .reftxt.txt insert end $txt\n
         if {[lindex $txt 1] == "Clock_column:"} {
            set clk_term [lindex $txt 2]
         }\
         elseif {[lindex $txt 0] == "INP"} {
            set t_name [string trimright [lindex $txt 1] ";"]
            if {[string index $t_name 0] != "t"} {
               set ref_is_buf_updated "no"
            }
            if {$n_term == $clk_term} {
               set CLOCK $t_name
            }
            lappend inp_list "$n_term $t_name"
            incr n_term
         }\
         elseif {[lindex $txt 0] == "OUT" || [lindex $txt 0] == "INOUT"} {
            set t_name [string trimright [lindex $txt 1] ";"]
            lappend outp_list "$n_term $t_name"
            incr n_term
            set outp_type($n_outp) [lindex $txt 0]
            incr n_outp
         }\
         elseif {[lindex $txt 0] == "PERIOD"} {
            set t [string trimright [lindex $txt 1] "ns;"]
            if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
               set PERIOD [get_period $t]
            }
            set time_step [expr 10*$t]
            set PeriodTime $time_step
         }\
         elseif {[lindex $txt 0] == "DATA_SECTION"} {
            set data_section "y"
         }\
         elseif {$data_section == "y"} {
            set tv [split [string trimleft $txt] <]
            if {[llength $tv] > 1} {
                set time [lindex $tv 0]
            }
         }
      }
      close $f_tex
   }

   set StartTime "0"
   set EndTime $time
   set NrCycles [expr $time / $time_step]

   if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
       update_term_window

      .menubar.file.cmds entryconfigure Update_with_buffile -state normal
      .menubar.file.cmds entryconfigure "Compare" -state normal
   }

   if {[winfo exists .cmpr]} {
      .cmpr.txt delete 1.0 end
      .cmpr.params.enp configure -state normal
      .cmpr.params.enc configure -state normal
      .cmpr.params.ens delete 0 end
      .cmpr.params.ene delete 0 end
      .cmpr.params.enp delete 0 end
      .cmpr.params.enc delete 0 end
      .cmpr.params.eno delete 0 end
      .cmpr.params.ens insert end $StartTime
      .cmpr.params.ene insert end $EndTime
      .cmpr.params.enp insert end $PeriodTime
      .cmpr.params.enc insert end $NrCycles
      .cmpr.params.eno insert end "0"
      .cmpr.params.enp configure -state readonly
      .cmpr.params.enc configure -state readonly
      .cmpr.menubar.file.cmds entryconfigure "Compare res" -state normal
      .cmpr.menubar.file.cmds entryconfigure "Compare csv" -state normal
      wm  title .cmpr "compare [file tail $RefFileName]"
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "file $RefFileName read"
   }

   set RefForCompPresent 1
   set message "file $RefFileName read"

   if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
      wm title . "test_flow $ref_tail"

      del_csv_win

      set buffiles [glob -directory $initialdir -nocomplain *.buf]
      if {[llength $buffiles] == 1} {
         set buf_file_name [lindex $buffiles 0]
         set reply [tk_dialog .bufq "" "Update with buf file $buf_file_name ?" \
                     questhead 0 Yes No]
         if {$reply == 0} {
            do_read_buffile
         }
      }
   }
}

proc read_buffile {} {
   global message
   global buf_file_name
   global initialdir

   # set debug [open "debug" "w"]
   set message ""
   set types {
      {{BUF Files} {.buf .BUF}}
      {{All Files}    *  }
   }
   set buf_file_name [tk_getOpenFile -initialdir $initialdir -filetypes $types]

   do_read_buffile
}

proc do_read_buffile {} {
   global data_name inp_list outp_list ref_is_buf_updated message buf_arr
   global buf_file_name buf_file_read CLOCK
   global chs_file_name
   global initialdir
   global ProgName

   if {$buf_file_name != ""} {
      set f_buf [open $buf_file_name "r"]
   }
   while {![eof $f_buf]} {
      set txt [gets $f_buf]
      if {[lindex $txt 1] == "NC"} {
         set buf_arr([lindex $txt 0]) "NC"
      }\
      else {
         set buf_arr([lindex $txt 0]) "[lindex $txt 2]"
      }
      # puts $debug "[lindex $txt 0] [lindex $txt 2]"
      if {[lindex $txt 1] == "INPUT"} {
         if {$ref_is_buf_updated == "yes"} {
            if {[lindex $txt 0] == $CLOCK} {
                set CLOCK "$CLOCK [lindex $txt 2]"
            }
            set idx [lsearch $inp_list "* [lindex $txt 0]"]
            if {$idx >= 0} {
               set nw_item \
                 [linsert [lindex $inp_list $idx] end [lindex $txt 2]]
               #puts $debug "x $idx $nw_item"
               set inp_list [lreplace $inp_list $idx $idx $nw_item]
            }
            #puts $debug "x $inp_list"
         }\
         else {
            if {[lindex $txt 2] == $CLOCK} {
                set CLOCK "[lindex $txt 0] $CLOCK"
            }
            set idx [lsearch $inp_list "* [lindex $txt 2]"]
            if {$idx >= 0} {
               set nw_item \
                     [linsert [lindex $inp_list $idx] 1 [lindex $txt 0]]
               # puts $debug "y $idx $nw_item"
               set inp_list [lreplace $inp_list $idx $idx $nw_item]
            }
            # puts $debug "y $inp_list"
         }\
      }\
      elseif {[lindex $txt 1] == "OUTPUT"} {
         if {$ref_is_buf_updated == "yes"} {
            set idx [lsearch $outp_list "* [lindex $txt 0]"]
            if {$idx >= 0} {
               set nw_item \
                 [linsert [lindex $outp_list $idx] end [lindex $txt 2]]
               set outp_list [lreplace $outp_list $idx $idx $nw_item]
            }
         }\
         else {
            set idx [lsearch $outp_list "* [lindex $txt 2]"]
            if {$idx >= 0} {
               set nw_item \
                    [linsert [lindex $outp_list $idx] 1 [lindex $txt 0]]
               set outp_list [lreplace $outp_list $idx $idx $nw_item]
            }
         }\
      }\
      elseif {[lindex $txt 1] == "DIRECT"} {
         if {$ref_is_buf_updated == "yes"} {
            set idx [lsearch $outp_list "* [lindex $txt 0]"]
            if {$idx >= 0} {
               set nw_item \
                 [linsert [lindex $outp_list $idx] end [lindex $txt 2]]
               set outp_list [lreplace $outp_list $idx $idx $nw_item]
            }
         }\
         else {
            set idx [lsearch $outp_list "* [lindex $txt 2]"]
            if {$idx >= 0} {
               set nw_item \
                    [linsert [lindex $outp_list $idx] 1 [lindex $txt 0]]
               set outp_list [lreplace $outp_list $idx $idx $nw_item]
            }
         }
      }
   }
   close $f_buf

   # close $debug

   if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
      update_term_window

      # .menubar.file.cmds entryconfigure Show_pod_connections -state normal
      .menubar.file.cmds entryconfigure Update_with_buffile -state disabled
      .menubar.file.cmds entryconfigure Make_csvfile -state normal
   }
   set buf_file_read "yes"

   if {[winfo exists .cmpr]} {
      .cmpr.etxt delete 1.0 end
      .cmpr.etxt insert end "data updated with buf_file $buf_file_name"
   }
   set message "data updated with buf_file $buf_file_name"
   update idletasks

   if {$ProgName == "test_flow" || $ProgName == "TestFlow"} {
      set chsfiles [glob -directory $initialdir -nocomplain *.chs]
      if {[llength $chsfiles] == 1} {
         set chs_file_name [lindex $chsfiles 0]
         set reply [tk_dialog .bufq "" "Update with chs file $chs_file_name ?" \
                     questhead 0 Yes No]
         if {$reply == 0} {
            do_read_chs_file
         }
      }
      del_csv_win
   }
}

