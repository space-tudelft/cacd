
proc show_add_bondleer {nr} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the placement of a circuit in a   #
# bondleer can be made.                                                       #
#-----------------------------------------------------------------------------#
   global Fnt

   toplevel .abl_wdw
   frame    .abl_wdw.sfr1 -bd 2 -relief raised
   frame    .abl_wdw.sfr2 -bd 2 -relief raised
   frame    .abl_wdw.sfr3 -bd 2 -relief raised -bg gold2
   label    .abl_wdw.sfr1.lbl -text "Add To Cell" -bg wheat3 -font $Fnt
   listbox  .abl_wdw.sfr1.lst -height $nr -font $Fnt -bg wheat
   label    .abl_wdw.sfr2.lbl -text "Bondleer name" -bg wheat3 -font $Fnt
   entry    .abl_wdw.sfr2.en  -width 14 -font $Fnt -bg wheat
   button   .abl_wdw.sfr3.cncl -text "Cancel" -width 6\
                                  -command {destroy .abl_wdw; upd_status_line ""}\
                                  -bg gold2 -font $Fnt
   button   .abl_wdw.sfr3.ok -text "OK" -width 6\
                                  -command {do_add_bondleer}\
                                  -bg gold2 -font $Fnt
   listbox  .abl_wdw.lbtf -width 3 -height 6 -font $Fnt -bg wheat
   listbox  .abl_wdw.lbtt -height 6 -font $Fnt -bg wheat

   pack .abl_wdw.lbtt       -side right -fill x -expand 1
   pack .abl_wdw.lbtf       -side right
   pack .abl_wdw.sfr1       -side top -fill both -expand 1
   pack .abl_wdw.sfr2       -side top -fill x
   pack .abl_wdw.sfr3       -side bottom -fill x
   pack .abl_wdw.sfr1.lbl   -side top -fill x
   pack .abl_wdw.sfr1.lst   -side left -fill both -expand 1
   pack .abl_wdw.sfr2.lbl   -side top -fill x
   pack .abl_wdw.sfr2.en    -side bottom -fill x
   pack .abl_wdw.sfr3.cncl  -side right -padx 10 -pady 5
   pack .abl_wdw.sfr3.ok    -side left -padx 10 -pady 5

   for {set i 1} {$i <= 6} {incr i} {
      .abl_wdw.lbtf insert end "bf$i"
   }

# The following bind_function shows the terminals in the right column
# of the interface (with an indication of in- or output).
# It uses the file .'cell'.term to determine the terminals
# a name for the circuit with the bondleer added is also given,
# but this name can be changed by the user.
#
   bind .abl_wdw.sfr1.lst <ButtonRelease-1> {
      .abl_wdw.sfr3.ok configure -state disabled
      .abl_wdw.sfr2.en delete 0 end
      .abl_wdw.lbtt delete 0 end
      .abl_wdw.lbtf selection set 0
      .abl_wdw.lbtf activate 0
      set idx [.abl_wdw.sfr1.lst index anchor]
      set tmpcell [.abl_wdw.sfr1.lst get $idx]
      if {$tmpcell == ""} break
      .abl_wdw.sfr2.en insert end $tmpcell\_bl
      for {set i 1} {$i <= 6} {incr i} {
         .abl_wdw.lbtt insert end ""
      }
      if {[catch {open .$tmpcell.term} fpt]} {
         df_mess_dialog "ERROR:\n\
             cannot place $tmpcell in bondleer because i cannot\n\
             read file .$tmpcell.term with terminal references"
         break
      }
      set nterm 0
      while {![eof $fpt]} {
         set tt [gets $fpt]
         if {$tt != ""} {
            .abl_wdw.lbtt insert $nterm "([lindex $tt 0])[lindex $tt 1]"
            incr nterm
         }
      }
      close $fpt
      if {$nterm > 6} {
         df_mess_dialog "ERROR:\n\
             cannot place $tmpcell in bondleer because there are\n\
             too many terminals to fit in a bondleer (> 6)"
         break
      }
      .abl_wdw.sfr3.ok configure -state normal
   }
   bind .abl_wdw.lbtf <ButtonRelease-1> {
        .abl_wdw.lbtf activate [.abl_wdw.lbtf curselection]
   }
   bind .abl_wdw.lbtt <ButtonRelease-1> {update_terms_bl}

   wm title .abl_wdw "add_bondleer"
}

proc do_add_bondleer {} {
#-----------------------------------------------------------------------------#
# procedure to do the placement of a circuit in a bondleer                    #
#-----------------------------------------------------------------------------#

   upd_status_line "generating cell with bondleer: BUSY"
#
# see if names for the cell and the bondleer cell are specified
#
   set cell [.abl_wdw.sfr1.lst get [.abl_wdw.sfr1.lst index anchor]]
   if {$cell == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             first chose the cell to be placed from the list of cells"
      return
   }
   set bl_cell [string trim [.abl_wdw.sfr2.en get]]
   if {$bl_cell == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             give the name of the circuit in the bondleer\
             in the Bondleer name entry"
      return
   }
#
# get the terminal_names from the lbtt_listbox
#
   set tlist ""
   set blist ""
   set fpt [open .$cell.term]
   while {![eof $fpt]} {
      set tt [gets $fpt]
      if {$tt != ""} {
	 set term [lindex $tt 1]
	 append tlist "$term, "
	 for {set i 0} {$i < 6} {} {
	    set tt [.abl_wdw.lbtt get $i]
	    incr i
	    if {$tt != "" && [string range $tt 3 end] == $term} {
	       append blist "bf$i, "
	       break
	    }
	 }
      }
   }
   close $fpt
#
# generate a tempory sls_file
#
   set fp_tot [open SLS/$bl_cell.sls w]
   puts $fp_tot "extern network $cell (terminal ${tlist}vss, vdd)"
   puts $fp_tot "extern network bond_leer (terminal bf1, bf2, bf3, bf4, bf5, bf6, vdd, vss)\n"
   puts $fp_tot "network $bl_cell (terminal bf1, bf2, bf3, bf4, bf5, bf6)"
   puts $fp_tot "{"
   puts $fp_tot "   $cell (${blist}vss, vdd);"
   puts $fp_tot "   bond_leer (bf1, bf2, bf3, bf4, bf5, bf6, vdd, vss);"
   puts $fp_tot "}"
   close $fp_tot
#
# compile the generated sls_file into the network_database
#
   if {[catch {exec csls SLS/$bl_cell.sls 2>.slslog}]} {
      set fp [open .slslog]
      df_mess_dialog "csls ERROR:\n\n[read $fp]"
      close $fp
   }
#
# generate a buf_file with the terminal connections
#
   set f_buf [open $bl_cell.buf w]
   set fpt [open .$bl_cell.term w]
   for {set i 0} {$i < 6} {} {
      set tt [.abl_wdw.lbtt get $i]
      incr i
      set type "B"
      if {$tt == ""} {
	 puts $f_buf "t$i  NC"
      } elseif {[string index $tt 1] == "I"} {
	 puts $f_buf "t$i  INPUT  [string range $tt 3 end]"
	 set type "I"
      } elseif {[string index $tt 1] == "O"} {
	 puts $f_buf "t$i  OUTPUT [string range $tt 3 end]"
	 set type "O"
      } else {
	 puts $f_buf "t$i  DIRECT [string range $tt 3 end]"
      }
      puts $fpt "$type bf$i bf$i"
   }
   close $fpt
   close $f_buf

   destroy .abl_wdw
   read_infofile
   upd_status_line "generating cell with bondleer: DONE"
}

proc update_terms_bl {} {
#-----------------------------------------------------------------------------#
# procedure to specify the connections with the bondleer                      #
#-----------------------------------------------------------------------------#
   set idx [.abl_wdw.lbtt curselection]
   if {$idx == ""} return
   set idx_f [.abl_wdw.lbtf index active]
   if {$idx_f == ""} {
      df_mess_dialog "INSTRUCTION:\n\
             first chose a terminal from the first row"
      return
   }
if {$idx_f != $idx} {
   set tmp1 [.abl_wdw.lbtt get $idx_f]
   set tmp2 [.abl_wdw.lbtt get $idx]
   .abl_wdw.lbtt delete $idx_f
   .abl_wdw.lbtt insert $idx_f $tmp2
   .abl_wdw.lbtt delete $idx
   .abl_wdw.lbtt insert $idx $tmp1
}
   if {[incr idx_f] >= 6} { set idx_f 0 }
   .abl_wdw.lbtf selection set $idx_f
   .abl_wdw.lbtf activate $idx_f
}

proc add_bondleer {} {
#-----------------------------------------------------------------------------#
# procedure to add a bond_leer cell as the father of a given cell             #
#-----------------------------------------------------------------------------#
   global DbName

   upd_status_line "add_bondleer: BUSY"
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
   show_add_bondleer $nr
   foreach cell $clist { .abl_wdw.sfr1.lst insert end $cell }
}
