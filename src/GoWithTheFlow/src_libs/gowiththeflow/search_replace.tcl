
proc do_search {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to generate the search/replace window for an edit_window          #
#-----------------------------------------------------------------------------#
   global Fnt

   set name [file tail [wm title .$edt_wdw]]
   if {[winfo exists .sr_wdw]} {
      .sr_wdw.ffr.slbl configure -text "Find in $name:"
      .sr_wdw.cfr.fnd  configure -command "search $edt_wdw"
      .sr_wdw.cfr.reps configure -command "replace $edt_wdw"
      .sr_wdw.cfr.repa configure -command "replace_all $edt_wdw"
      raise .sr_wdw
   }\
   else {
      toplevel    .sr_wdw
      frame       .sr_wdw.ffr           -bg wheat -bd 2 -relief raised
      label       .sr_wdw.ffr.slbl      -bg wheat\
                                        -text "Find in $name:"\
                                        -font $Fnt
      entry       .sr_wdw.ffr.sen       -width 25 -bg wheat -font $Fnt
      frame       .sr_wdw.ofr0          -bg wheat
      frame       .sr_wdw.ofr0.ofr1     -bg wheat -bd 2 -relief raised
      frame       .sr_wdw.ofr0.ofr2     -bg wheat -bd 2 -relief raised
      frame       .sr_wdw.ofr0.ofr3     -bg wheat -bd 2 -relief raised
      label       .sr_wdw.ofr0.ofr1.lbl -bg wheat -fg red\
                                        -text "Start from" -width 12\
                                        -font $Fnt
      radiobutton .sr_wdw.ofr0.ofr1.rbs -bg wheat -text "begin"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Sposition -value Begin
      radiobutton .sr_wdw.ofr0.ofr1.rbi -bg wheat -text "insert"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Sposition -value Index
      radiobutton .sr_wdw.ofr0.ofr1.rbe -bg wheat -text "end"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Sposition -value End
      label       .sr_wdw.ofr0.ofr2.lbl -bg wheat -fg red\
                                        -text "Direction" -width 12\
                                        -font $Fnt
      radiobutton .sr_wdw.ofr0.ofr2.rbf -bg wheat -text "forward"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Sdirection -value Fwd
      radiobutton .sr_wdw.ofr0.ofr2.rbb -bg wheat -text "backward"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Sdirection -value Bck
      label       .sr_wdw.ofr0.ofr3.lbl -bg wheat -fg red\
                                        -text "Case" -width 12\
                                        -font $Fnt
      radiobutton .sr_wdw.ofr0.ofr3.rbi -bg wheat -text "ignore"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Scase -value Ignore
      radiobutton .sr_wdw.ofr0.ofr3.rbm -bg wheat -text "exact"\
                                        -font $Fnt\
                                        -anchor w -highlightthickness 0\
                                        -variable Scase -value Mind
      frame       .sr_wdw.rfr           -bg wheat -bd 2 -relief raised
      label       .sr_wdw.rfr.rlbl      -text "Replace With" -bg wheat\
                                        -font $Fnt
      entry       .sr_wdw.rfr.ren       -width 25 -bg wheat -font $Fnt
      frame       .sr_wdw.cfr           -bg wheat3 -bd 2 -relief raised
      button      .sr_wdw.cfr.fnd       -text "Find" -width 8\
                                        -font $Fnt\
                                        -bg wheat3\
                                        -padx 5\
                                        -command "search $edt_wdw"
      button      .sr_wdw.cfr.reps      -text "Replace" -width 8\
                                        -font $Fnt\
                                        -bg wheat3\
                                        -padx 5\
                                        -command "replace $edt_wdw"
      button      .sr_wdw.cfr.repa      -text "Repl All" -width 8\
                                        -font $Fnt\
                                        -bg wheat3\
                                        -padx 5\
                                        -command "replace_all $edt_wdw"
      button      .sr_wdw.cfr.cncl      -text Cancel -bg wheat3\
                                        -font $Fnt\
                                        -command "destroy .sr_wdw"

      pack .sr_wdw.ffr           -side top    -fill x
      pack .sr_wdw.ofr0          -side top    -fill x
      pack .sr_wdw.rfr           -side top    -fill x
      pack .sr_wdw.cfr           -side top    -fill x
      pack .sr_wdw.ffr.slbl      -side top    -fill x -padx 10 -pady 5
      pack .sr_wdw.ffr.sen       -side top    -fill x -padx 10 -pady 5
      pack .sr_wdw.ofr0.ofr1     -side left   -fill y
      pack .sr_wdw.ofr0.ofr2     -side left   -fill y
      pack .sr_wdw.ofr0.ofr3     -side left   -fill y
      pack .sr_wdw.cfr.cncl      -side bottom -fill x -padx 10 -pady 5
      pack .sr_wdw.cfr.fnd       -side left   -padx 5 -pady 5
      pack .sr_wdw.cfr.repa      -side right  -padx 5 -pady 5
      pack .sr_wdw.cfr.reps      -side right  -padx 5 -pady 5
      pack .sr_wdw.ofr0.ofr1.lbl -side top    -fill x
      pack .sr_wdw.ofr0.ofr1.rbs -side top    -fill x
      pack .sr_wdw.ofr0.ofr1.rbi -side top    -fill x
      pack .sr_wdw.ofr0.ofr1.rbe -side top    -fill x
      pack .sr_wdw.ofr0.ofr2.lbl -side top    -fill x
      pack .sr_wdw.ofr0.ofr2.rbf -side top    -fill x
      pack .sr_wdw.ofr0.ofr2.rbb -side top    -fill x
      pack .sr_wdw.ofr0.ofr3.lbl -side top    -fill x
      pack .sr_wdw.ofr0.ofr3.rbm -side top    -fill x
      pack .sr_wdw.ofr0.ofr3.rbi -side top    -fill x
      pack .sr_wdw.rfr.rlbl      -side top    -fill x -padx 10 -pady 5
      pack .sr_wdw.rfr.ren       -side top    -fill x -padx 10 -pady 5
   }
   .sr_wdw.ofr0.ofr1.rbs select
   .sr_wdw.ofr0.ofr2.rbf select
   .sr_wdw.ofr0.ofr3.rbm select
   wm title .sr_wdw Find/Change
   focus .sr_wdw.ffr.sen
}

proc search {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to search for a string in an edit_window                          #
#-----------------------------------------------------------------------------#
   upvar #0 Sposition spos
   upvar #0 Sdirection sdirect
   upvar #0 Scase scase
   if {[.sr_wdw.ffr.sen get] == ""} {
      df_mess_dialog "INSTRUCTION:\n first supply a string to search for."
   }
   if {$spos == "Begin"} {
      set s_idx "1.0"
   } elseif {$spos == "End"} {
      set s_idx [.$edt_wdw.txt index "end-1c"]
   } else {
      set s_idx [.$edt_wdw.txt index insert]
   }
   if {$scase == "Ignore"} {
      set ctype "-nocase"
   } else {
      set ctype ""
   }
   if {$sdirect == "Fwd"} {
      set fidx [eval .$edt_wdw.txt search -forwards $ctype -count nc --\
					{[.sr_wdw.ffr.sen get]} $s_idx end]
      if {$fidx != ""} {
         .$edt_wdw.txt see $fidx
         set tmpidx [split $fidx .]
         set tmpnl [lindex $tmpidx 0]
         set tmpnc [lindex $tmpidx 1]
         incr tmpnc
         .$edt_wdw.txt mark set insert $tmpnl.$tmpnc
         .sr_wdw.ofr0.ofr1.rbi select
      } else {
         .$edt_wdw.txt tag delete FOUND
         df_mess_dialog "INFORMATION:\n no (new) match found"
         return
      }
   } else {
      set fidx [eval .$edt_wdw.txt search -backwards $ctype -count nc --\
					{[.sr_wdw.ffr.sen get]} $s_idx 1.0]
      if {$fidx != ""} {
         .$edt_wdw.txt see $fidx
         set tmpidx [split $fidx .]
         set tmpnl [lindex $tmpidx 0]
         set tmpnc [lindex $tmpidx 1]
         incr tmpnc -1
         .$edt_wdw.txt mark set insert $tmpnl.$tmpnc
         .sr_wdw.ofr0.ofr1.rbi select
      } else {
         .$edt_wdw.txt tag delete FOUND
         df_mess_dialog "INFORMATION:\n no (new) match found"
         return
      }
   }
   .$edt_wdw.txt tag delete FOUND
   .$edt_wdw.txt tag add FOUND $fidx "$fidx + $nc chars"
   .$edt_wdw.txt tag configure FOUND -background "green"
}

proc replace {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to replace a found string in an edit_window  by another one       #
#-----------------------------------------------------------------------------#
   set tr [.$edt_wdw.txt tag ranges FOUND]
   if {$tr != ""} {
      .$edt_wdw.txt tag delete FOUND
      .$edt_wdw.txt delete [lindex $tr 0] [lindex $tr 1]
      .$edt_wdw.txt insert [lindex $tr 0] [.sr_wdw.rfr.ren get]
   }
}

proc replace_all {edt_wdw} {
#-----------------------------------------------------------------------------#
# procedure to replace all found strings in an edit_window by another string  #
#-----------------------------------------------------------------------------#
   upvar #0 Scase scase
   if {$scase == "Ignore"} {
      set ctype "-nocase"
   } else {
      set ctype ""
   }
   set fidx [eval .$edt_wdw.txt search -backwards $ctype -count nc --\
					{[.sr_wdw.ffr.sen get]} end 1.0]
   while {$fidx != ""} {
      .$edt_wdw.txt delete $fidx "$fidx + $nc chars"
      .$edt_wdw.txt insert $fidx [.sr_wdw.rfr.ren get]
      set fidx [eval .$edt_wdw.txt search -backwards $ctype -count nc --\
					{[.sr_wdw.ffr.sen get]} $fidx 1.0]
   }
}
