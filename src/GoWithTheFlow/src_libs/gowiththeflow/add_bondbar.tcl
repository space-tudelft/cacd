
proc show_add_bondbar {nr} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the placement of a circuit in a   #
# bondbar can be made.                                                        #
#-----------------------------------------------------------------------------#
   global Fnt

   toplevel .abb_wdw
   frame    .abb_wdw.sfr1 -bd 2 -relief raised
   frame    .abb_wdw.sfr2 -bd 2 -relief raised
   frame    .abb_wdw.sfr3 -bd 2 -relief raised -bg gold2
   label    .abb_wdw.sfr1.lbl -text "Add To Cell" -bg wheat3 -font $Fnt
   listbox  .abb_wdw.sfr1.lst -height $nr -font $Fnt -bg wheat
   label    .abb_wdw.sfr2.lbl -text "Bondbar name" -bg wheat3 -font $Fnt
   entry    .abb_wdw.sfr2.en  -width 14 -font $Fnt -bg wheat
   button   .abb_wdw.sfr3.cncl -text "Cancel" -width 6\
                                  -command {destroy .abb_wdw; upd_status_line ""}\
                                  -bg gold2 -font $Fnt
   button   .abb_wdw.sfr3.ok -text "OK" -width 6\
                                  -command {do_add_bondbar}\
                                  -bg gold2 -font $Fnt
   listbox  .abb_wdw.lbtf -width 3 -height 32 -font $Fnt -bg wheat
   listbox  .abb_wdw.lbtt -height 32 -font $Fnt -bg wheat

   pack .abb_wdw.lbtt       -side right -fill x -expand 1
   pack .abb_wdw.lbtf       -side right
   pack .abb_wdw.sfr1       -side top -fill both -expand 1
   pack .abb_wdw.sfr2       -side top -fill x
   pack .abb_wdw.sfr3       -side bottom -fill x
   pack .abb_wdw.sfr1.lbl   -side top -fill x
   pack .abb_wdw.sfr1.lst   -side left -fill both -expand 1
   pack .abb_wdw.sfr2.lbl   -side top -fill x
   pack .abb_wdw.sfr2.en    -side bottom -fill x
   pack .abb_wdw.sfr3.cncl  -side right -padx 10 -pady 5
   pack .abb_wdw.sfr3.ok    -side left -padx 10 -pady 5

   for {set i 1} {$i <= 32} {incr i} {
      .abb_wdw.lbtf insert end "t$i"
   }

# The following bind_function shows the terminals in the right column
# of the interface (with an indication of in- or output).
# It uses the file .'cell'.term to determine the terminals
# a name for the circuit with the bondbar added is also given,
# but this name can be changed by the user.
#
   bind .abb_wdw.sfr1.lst <ButtonRelease-1> {
      .abb_wdw.sfr3.ok configure -state disabled
      .abb_wdw.sfr2.en delete 0 end
      .abb_wdw.lbtt delete 0 end
      .abb_wdw.lbtf selection set 0
      .abb_wdw.lbtf activate 0
      set idx [.abb_wdw.sfr1.lst index anchor]
      set tmpcell [.abb_wdw.sfr1.lst get $idx]
      if {$tmpcell == ""} break
      .abb_wdw.sfr2.en insert end $tmpcell\_bb
      for {set i 1} {$i <= 32} {incr i} {
         .abb_wdw.lbtt insert end ""
      }
      if {[catch {open .$tmpcell.term} fpt]} {
         df_mess_dialog "ERROR:\n\
             cannot place $tmpcell in bondbar because i cannot\n\
             read file .$tmpcell.term with terminal references"
         break
      }
      set nterm 0
      while {![eof $fpt]} {
         set tt [gets $fpt]
         if {$tt != ""} {
            .abb_wdw.lbtt insert $nterm "([lindex $tt 0])[lindex $tt 1]"
            incr nterm
         }
      }
      close $fpt
      if {$nterm > 32} {
         df_mess_dialog "ERROR:\n\
             cannot place $tmpcell in bondbar because there are\n\
             too many terminals to fit in a bondbar (> 32)"
         break
      }
      .abb_wdw.sfr3.ok configure -state normal
   }
   bind .abb_wdw.lbtf <ButtonRelease-1> {
        .abb_wdw.lbtf activate [.abb_wdw.lbtf curselection]
   }
   bind .abb_wdw.lbtt <ButtonRelease-1> {update_terms_bb}

   wm title .abb_wdw "add_bondbar"
}

proc do_add_bondbar {} {
#-----------------------------------------------------------------------------#
# procedure to do the placement of a circuit in a bondbar                     #
#-----------------------------------------------------------------------------#

   upd_status_line "generating cell with bondbar: BUSY"
#
# see if names for the cell and the bondbar cell are specified
#
   set cell [.abb_wdw.sfr1.lst get [.abb_wdw.sfr1.lst index anchor]]
   if {$cell == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             first chose the cell to be placed from the list of cells"
      return
   }
   set bb_cell [string trim [.abb_wdw.sfr2.en get]]
   if {$bb_cell == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             give the name of the circuit in the bondbar\
             in the Bondbar name entry"
      return
   }
#
# generate tlist/blist
#
   set tlist ""
   set blist ""
   set fpt [open .$cell.term]
   while {![eof $fpt]} {
      set tt [gets $fpt]
      if {$tt != ""} {
	 set term [lindex $tt 1]
	 append tlist "$term, "
	 for {set i 0} {$i < 32} {} {
	    set tt [.abb_wdw.lbtt get $i]
	    incr i
	    if {$tt != "" && [string range $tt 3 end] == $term} {
	       append blist "t$i, "
	       break
	    }
	 }
      }
   }
   close $fpt
#
# generate a tempory sls_file
#
   set fp_tot [open SLS/$bb_cell.sls w]
   puts $fp_tot "extern network $cell (terminal ${tlist}vss, vdd)"
   set tlist ""
   for {set i 1} {$i < 32} {incr i} { append tlist "t$i, " }
   append tlist "t32"
   puts $fp_tot "extern network bond_bar (terminal $tlist)\n"
   puts $fp_tot "network $bb_cell (terminal $tlist)"
   puts $fp_tot "{"
   puts $fp_tot "   $cell (${blist}vss, vdd);"
   puts $fp_tot "   bond_bar ($tlist);"
   puts $fp_tot "}"
   close $fp_tot
#
# compile the generated sls_file into the network_database
#
   if {[catch {exec csls SLS/$bb_cell.sls 2>.slslog}]} {
      set fp [open .slslog]
      df_mess_dialog "csls ERROR:\n\n[read $fp]"
      close $fp
   }
#
# generate a buf_file with the terminal connections
#
   set f_buf [open $bb_cell.buf w]
   set fpt [open .$bb_cell.term w]
   for {set i 0} {$i < 32} {} {
      set tt [.abb_wdw.lbtt get $i]
      incr i
      if {$i < 10} { set tm "t$i " } { set tm "t$i" }
      set type "B"
      if {$tt == ""} {
	 puts $f_buf "$tm NC"
      } elseif {[string index $tt 1] == "I"} {
	 puts $f_buf "$tm INPUT  [string range $tt 3 end]"
	 set type "I"
      } elseif {[string index $tt 1] == "O"} {
	 puts $f_buf "$tm OUTPUT [string range $tt 3 end]"
	 set type "O"
      } else {
	 puts $f_buf "$tm DIRECT [string range $tt 3 end]"
      }
      puts $fpt "$type t$i t$i"
   }
   close $fpt
   close $f_buf

   destroy .abb_wdw
   read_infofile
   upd_status_line "generating cell with bondbar: DONE"
}

proc update_terms_bb {} {
#-----------------------------------------------------------------------------#
# procedure to specify the connections with the bondbar                       #
#-----------------------------------------------------------------------------#
   set idx [.abb_wdw.lbtt curselection]
   if {$idx == ""} return
   set idx_f [.abb_wdw.lbtf index active]
   if {$idx_f == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             first chose a terminal from the first row"
      return
   }
if {$idx_f != $idx} {
   set tmp1 [.abb_wdw.lbtt get $idx_f]
   set tmp2 [.abb_wdw.lbtt get $idx]
   .abb_wdw.lbtt delete $idx_f
   .abb_wdw.lbtt insert $idx_f $tmp2
   .abb_wdw.lbtt delete $idx
   .abb_wdw.lbtt insert $idx $tmp1
}
   if {[incr idx_f] >= 32} { set idx_f 0 }
   .abb_wdw.lbtf selection set $idx_f
   .abb_wdw.lbtf activate $idx_f
}

proc add_bondbar {} {
#-----------------------------------------------------------------------------#
# procedure to add a bond_bar cell as the father of a given cell              #
#-----------------------------------------------------------------------------#
   global DbName

   upd_status_line "add_bondbar: BUSY"
   if {$DbName == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There is no database specified:\n\
          Make or chose a database by clicking the button\n\
          Make_database or the button Database"
      upd_status_line ""
      return
   }

   goto_dbdir
   set fpipe [open "|dblist -c"]
   restore_cwd

   set nr 0
   while {![eof $fpipe]} {
      set tmptxt [gets $fpipe]
      if {$tmptxt == "imported:"} break
      if {[string match "\[a-z\]" [string index $tmptxt 0]]} {
         lappend clist $tmptxt
	 incr nr
      }
   }
   close $fpipe

   lappend clist ""; incr nr
   show_add_bondbar $nr
   foreach cell $clist { .abb_wdw.sfr1.lst insert end $cell }
}
