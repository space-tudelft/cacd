
proc simulate_dac {argcell} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which the simulation of a dac can be    #
# carried out                                                                 #
#-----------------------------------------------------------------------------#
   global Fnt DacCell Extraction_kind Capacitor_extr DacTerms

   set DacCell $argcell
   set DacTerms ""
   goto_dbdir
   set fpipe [open "|dbcat -c -s term $DacCell"]
   while {![eof $fpipe]} {
      set txt [gets $fpipe]
      if {[string range [lindex $txt 0] 0 4] == "term:"} {
         set term [string trim [string range [lindex $txt 0] 5 end] \"]
         if {[string tolower $term] != "vss" &&
             [string tolower $term] != "vdd"} {
            lappend DacTerms $term
         }
      }
   }
   catch {close $fpipe} msg
   if {$DacTerms == ""} {
      df_mess_dialog "ERROR: terminals of cell $DacCell not found!\n$msg"
      return
   }
   restore_cwd

   toplevel   .sim_dac
   frame      .sim_dac.tfr         -relief raised -bd 2 -bg gold
   label      .sim_dac.tfr.lbl2    -text "cell:"  -font $Fnt -bg gold
   label      .sim_dac.tfr.lb_cell -text $DacCell -font $Fnt -bg gold -anchor w
   frame      .sim_dac.tfr1        -relief raised -bd 2 -bg gold
   label      .sim_dac.tfr1.lbl1   -text "input_resistor   value:" -font $Fnt -bg gold
   entry      .sim_dac.tfr1.en     -width 10 -font $Fnt -bg wheat3
   label      .sim_dac.tfr1.lbl2   -text "from" -font $Fnt -bg gold
   tk_optionMenu .sim_dac.tfr1.from DacSimInTermFrom vdd vss
   label      .sim_dac.tfr1.lbl3   -text "to" -font $Fnt -bg gold
   frame      .sim_dac.tfr2        -relief raised -bd 2 -bg gold
   label      .sim_dac.tfr2.lbl1   -text "output_resistor  value:" -font $Fnt -bg gold
   entry      .sim_dac.tfr2.en     -width 10 -font $Fnt -bg wheat3
   label      .sim_dac.tfr2.lbl2   -text "from" -font $Fnt -bg gold
   tk_optionMenu .sim_dac.tfr2.from DacSimOutTermFrom vdd vss
   label      .sim_dac.tfr2.lbl3   -text "to" -font $Fnt -bg gold
   frame      .sim_dac.lfr         -relief raised -bd 2 -bg wheat
   text       .sim_dac.txt         -width 80 -height 20 -font $Fnt -bg wheat\
                                   -yscrollcommand ".sim_dac.sb set"
   scrollbar  .sim_dac.sb          -command ".sim_dac.txt yview" -bg wheat
   button     .sim_dac.lfr.doit    -text "Doit" -font $Fnt -bg wheat3\
                                   -command {do_dac_sim}
   button     .sim_dac.lfr.show    -text "ShowResult" -font $Fnt -bg wheat3\
                                   -command { goto_dbdir
                                     exec simeye -bg white $DacCell\_tmp.ana &
                                              restore_cwd
                                            }
   button     .sim_dac.lfr.cancel  -text "Cancel" -font $Fnt -bg wheat3\
                                   -command {unset DacTerms; destroy .sim_dac}
   frame      .sim_dac.lfr.efr     -relief raised -bd 2
   label      .sim_dac.lfr.efr.lbl  -text "extraction" -font $Fnt -bg gold
   radiobutton .sim_dac.lfr.efr.rb1 -text "auto" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "AUTO" -anchor w
   radiobutton .sim_dac.lfr.efr.rb2 -text "yes" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "ALWAYS" -anchor w
   radiobutton .sim_dac.lfr.efr.rb3 -text "no" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "NEVER" -anchor w
   frame      .sim_dac.lfr.cfr     -relief raised -bd 2
   label      .sim_dac.lfr.cfr.lbl  -text "capacitors" -font $Fnt -bg gold
   radiobutton .sim_dac.lfr.cfr.rb1 -text "yes" -font $Fnt -bg wheat3\
                                    -variable Capacitor_extr -value "YES" -anchor w
   radiobutton .sim_dac.lfr.cfr.rb2 -text "no" -font $Fnt -bg wheat3\
                                    -variable Capacitor_extr -value "NO" -anchor w

   pack .sim_dac.lfr         -side left   -fill y
   pack .sim_dac.tfr         -side top    -fill x
   pack .sim_dac.tfr.lbl2    -side left   -padx 8
   pack .sim_dac.tfr.lb_cell -side left
   pack .sim_dac.tfr1        -side top    -fill x
   pack .sim_dac.tfr1.lbl1   -side left   -padx 10
   pack .sim_dac.tfr1.en     -side left   -padx 10
   pack .sim_dac.tfr1.lbl2   -side left   -padx 10
   pack .sim_dac.tfr1.from   -side left   -padx 10
   pack .sim_dac.tfr1.lbl3   -side left   -padx 10
   pack .sim_dac.tfr2        -side top    -fill x
   pack .sim_dac.tfr2.lbl1   -side left   -padx 10
   pack .sim_dac.tfr2.en     -side left   -padx 10
   pack .sim_dac.tfr2.lbl2   -side left   -padx 10
   pack .sim_dac.tfr2.from   -side left   -padx 10
   pack .sim_dac.tfr2.lbl3   -side left   -padx 10
   pack .sim_dac.sb          -side right  -fill y
   pack .sim_dac.txt         -side right  -fill both -expand 1
   pack .sim_dac.lfr.cancel  -side bottom -padx 10 -pady 10 -fill x
   pack .sim_dac.lfr.show    -side bottom -padx 10 -pady 10 -fill x
   pack .sim_dac.lfr.doit    -side bottom -padx 10 -pady 10 -fill x
   pack .sim_dac.lfr.efr     -side top    -fill x
   pack .sim_dac.lfr.efr.lbl -side top    -fill x
   pack .sim_dac.lfr.efr.rb1 -side top    -fill x
   pack .sim_dac.lfr.efr.rb2 -side top    -fill x
   pack .sim_dac.lfr.efr.rb3 -side top    -fill x
   pack .sim_dac.lfr.cfr     -side top    -fill x
   pack .sim_dac.lfr.cfr.lbl -side top    -fill x
   pack .sim_dac.lfr.cfr.rb1 -side top    -fill x
   pack .sim_dac.lfr.cfr.rb2 -side top    -fill x

   set Extraction_kind "AUTO"
   set Capacitor_extr "NO"
   update
   wm title .sim_dac "dac_simulation window"
   .sim_dac.txt delete 1.0 end

   .sim_dac.tfr1.from configure -width 3 -font $Fnt -bg gold
   .sim_dac.tfr1.from.menu configure -font $Fnt
   .sim_dac.tfr2.from configure -width 3 -font $Fnt -bg gold
   .sim_dac.tfr2.from.menu configure -font $Fnt

   eval tk_optionMenu .sim_dac.tfr1.term DacSimInTerm $DacTerms
   pack .sim_dac.tfr1.term -side left
   .sim_dac.tfr1.term configure -width 15 -font $Fnt -bg gold
   .sim_dac.tfr1.term.menu configure -font $Fnt
   .sim_dac.tfr1.term.menu invoke 0
   eval tk_optionMenu .sim_dac.tfr2.term DacSimOutTerm $DacTerms
   pack .sim_dac.tfr2.term -side left
   .sim_dac.tfr2.term configure -width 15 -font $Fnt -bg gold
   .sim_dac.tfr2.term.menu configure -font $Fnt
   .sim_dac.tfr2.term.menu invoke 0
}

proc get_sig_unit {} {
#-----------------------------------------------------------------------------#
# procedure to find the sigunit from the cmd_file                             #
#-----------------------------------------------------------------------------#
   set unit ""
   set ftmp [open tmp.cmd]
   while {![eof $ftmp]} {
      set txt [gets $ftmp]
      if {[lindex $txt 0] == "option" && [lindex $txt 1] == "sigunit"} {
	set unit [lindex $txt 3]
	break
      }
   }
   close $ftmp
   if {$unit == ""} {
      set unit "1e-7"
      df_mess_dialog "INFORMATION\n\
	sigunit cannot be determined from the command_file:\n\
	A value of $unit will be taken"
   }
   return $unit
}

proc do_dac_sim {} {
#-----------------------------------------------------------------------------#
# procedure to do a dac_extraction and simulation                             #
#-----------------------------------------------------------------------------#
   global Extraction_kind Capacitor_extr DacTerms
   upvar #0 DacCell cell
   upvar #0 DacSimInTerm in_term
   upvar #0 DacSimOutTerm out_term
   upvar #0 DacSimInTermFrom in_term_from
   upvar #0 DacSimOutTermFrom out_term_from

   set tmpCell [string toupper [string  index $cell 0]]
   append tmpCell [string range $cell 1 end]

   goto_dbdir

   if {$Extraction_kind == "ALWAYS" || ($Extraction_kind == "AUTO" &&
	([file exists layout/$cell/in_gln] == 1 ||
	[file exists layout/$cell/ins_gln] == 1 ||
	[file exists circuit/$tmpCell/net] == 0 ||
	[file mtime layout/$cell/box] > [file mtime circuit/$tmpCell/net]))} {

      .sim_dac.txt insert end "makeboxl -x $cell\n"
      update
      catch {exec makeboxl -x $cell >& tmp_spc}
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_dac.txt insert end "[gets $fp]\n"
         .sim_dac.txt see end
         update
      }
      close $fp

      .sim_dac.txt insert end "makegln $cell\n"
      catch {exec makegln $cell >& tmp_spc}
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_dac.txt insert end "[gets $fp]\n"
         .sim_dac.txt see end
         update
      }
      close $fp

      if {$Capacitor_extr == "NO"} {
         .sim_dac.txt insert end "space $cell\n"
         update
         catch {exec space $cell >& tmp_spc}
      } else {
         .sim_dac.txt insert end "space -c $cell\n"
         update
         catch {exec space -c $cell >& tmp_spc}
      }
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_dac.txt insert end "[gets $fp]\n"
         .sim_dac.txt see end
          update
      }
      close $fp

      .sim_dac.txt insert end "ghoti $tmpCell\n"
      update
      set fp [open "|ghoti $tmpCell"]
      while {![eof $fp]} {
         .sim_dac.txt insert end "[gets $fp]\n"
         .sim_dac.txt see end
         update
      }
      close $fp

      .sim_dac.txt insert end "removing gln_files\n"
      set gln_files [glob -nocomplain "layout/$cell/*_gln"];
      foreach i_gln $gln_files {
         file delete $i_gln
      }
   }
   update

   set in_res_val [.sim_dac.tfr1.en get]
   if {$in_res_val == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	There is no input resistor value given:\n\
	Specify a value in the 'input_resistor value' entry\n\
	and try again."
      restore_cwd
      return
   }
   set out_res_val [.sim_dac.tfr2.en get]
   if {$out_res_val == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	There is no output resistor value given:\n\
	Specify a value in the 'output_resistor value' entry\n\
	and try again."
      restore_cwd
      return
   }
   .sim_dac.txt insert end "generating a testcell .....\n"
   update
   .sim_dac.txt see end
# make a sls_file tmp.sls with the extracted cell and an input_
# and output resistor

   set ftmp [open tmp.sls "w"]
   set del_term_list ""
   set repl_term_list ""
   foreach i $DacTerms {
      if {$i == $in_term} {
         lappend rep_term_list "node_in"
      } elseif {$i == $out_term} {
         lappend rep_term_list "node_out"
      } else {
         lappend del_term_list $i
         lappend rep_term_list $i
      }
   }
   puts -nonewline $ftmp "extern network $tmpCell (terminal "
   foreach i $DacTerms { puts -nonewline $ftmp "$i, " }
   puts $ftmp "vss, vdd)"
   puts -nonewline $ftmp "network $cell\_tmp (terminal "
   foreach i $del_term_list { puts -nonewline $ftmp "$i, " }
   puts $ftmp "vss, vdd)"
   puts $ftmp "{"
   puts -nonewline $ftmp "   {inst_c}  $tmpCell ("
   foreach i $rep_term_list { puts -nonewline $ftmp "$i, " }
   puts $ftmp "vss, vdd);"
   puts $ftmp "   {inst_ri} res $in_res_val (node_in, $in_term_from);"
   puts $ftmp "   {inst_ro} res $out_res_val (node_out, $out_term_from);"
   puts $ftmp "}"
   close $ftmp

# place the cell generated in the nelsis database
   goto_dbdir
   .sim_dac.txt insert end "placing the cell in the database .....\n"
   update
   .sim_dac.txt see end
   set res [catch {exec csls tmp.sls 2>sls.diag}]
   set fp [open "sls.diag"]
   .sim_dac.txt insert end [read $fp]
   close $fp
   if {$res} {
      .sim_dac.txt insert end "**** simulation aborted ****\n"
      .sim_dac.txt see end
      restore_cwd
      return
   }
   .sim_dac.txt insert end "placing the cell in the database done\n"

# generate a cmd_file
   .sim_dac.txt insert end "generating a cmd_file .....\n"
   update
   .sim_dac.txt see end
   set ftmp [open tmp.cmd "w"]
   puts $ftmp "set vdd = h*~"
   puts $ftmp "set vss = l*~"
   set n_bit [llength $del_term_list]
   set val1 1
   for {set i 0} {$i < $n_bit} {incr i} {
      set val1 [expr $val1*2]
   }
   set val2 1
   for {set i 0} {$i < $n_bit} {incr i} {
      puts $ftmp "set [lindex $del_term_list [expr $n_bit-$i-1]] = (l*$val1 h*$val1)*$val2"
      set val1 [expr $val1/2]
      set val2 [expr $val2*2]
   }
   puts $ftmp "\noption level   = 3"
   puts $ftmp "option sigunit = 1e-7"
   puts $ftmp "option outacc  = 10n\n"
   if {$n_bit < 1} {
      puts $ftmp "option simperiod = 25\n"
   }
   puts -nonewline $ftmp "print node_in, node_out"
   for {set i 0} {$i < $n_bit} {incr i} {
      puts -nonewline $ftmp ", [lindex $del_term_list $i]"
   }
   puts -nonewline $ftmp "\nplot  node_in, node_out"
   for {set i 0} {$i < $n_bit} {incr i} {
      puts -nonewline $ftmp ", [lindex $del_term_list $i]"
   }
   puts $ftmp "\n\n/*"
   puts $ftmp "*%"
   puts $ftmp "tfall 2.0n"
   puts $ftmp "trise 2.0n"
   puts $ftmp "tstep 1n"
   puts $ftmp "*%"
   puts $ftmp ".options cptime = 500"
   puts $ftmp "*%"
   puts $ftmp "*/"
   close $ftmp
   .sim_dac.txt insert end "cmd_file generated\n"
   update
   .sim_dac.txt see end
   set answ [df_choise_dialog "Do you want to see or change the command file made ?"]
   if {$answ == "yes"} {
      change_cmd_file
      tkwait window .chgcmdf
      update
   }

#call the simulator
   update
   .sim_dac.txt insert end "running the simulator .....\n"
   update
   .sim_dac.txt see end
   set res [catch {exec nspice $cell\_tmp tmp.cmd 2>sim.diag}]
   set fp [open "sim.diag"]
   .sim_dac.txt insert end [read $fp]
   close $fp
   if {$res} {
      .sim_dac.txt insert end "**** simulation aborted ****\n"
      .sim_dac.txt see end
      restore_cwd
      return
   }
   .sim_dac.txt insert end "simulation done\n\n"
   .sim_dac.txt insert end "global results:\n"
   .sim_dac.txt insert end "-------------------------------------------------------------------------\n"
   .sim_dac.txt insert end "    time           U_in          U_out       I_in(mA)   I_out(mA) D_Iout\n"
   .sim_dac.txt insert end "-------------------------------------------------------------------------\n"
   update
   .sim_dac.txt see end
   set sig_unit [get_sig_unit]
   set time_ref [expr 1.8*$sig_unit]
   set step_ref -1
   set prev_val 0.0

   set fp [open $cell\_tmp.ana]
   set VALUES_FOUND 0
   while {![eof $fp]} {
      set txt [gets $fp]
      if {[lindex $txt 0] == ""} continue
      if {[lindex $txt 0] == "Index"} { incr VALUES_FOUND; continue }
      if {$VALUES_FOUND && [string trimleft [lindex $txt 0] "0123456789"] == ""} {
         if {[lindex $txt 0] < $step_ref} break
         if {[lindex $txt 1] > $time_ref} {
            set t [lindex $txt 1]
            set U_in [lindex $txt 2]
            set U_out [lindex $txt 3]
            if {$in_term_from == "vdd"} {
               set I_in [format "%f" [expr 1.0e3*(5.0 - $U_in)/$in_res_val]]
            } else {
               set I_in [format "%f" [expr 1.0e3*$U_in/$in_res_val]]
            }
            if {$out_term_from == "vdd"} {
               set I_out [format "%f" [expr 1.0e3*(5.0 - $U_out)/$out_res_val]]
            } else {
               set I_out [format "%f" [expr 1.0e3*$U_out/$out_res_val]]
            }
            set D_Iout [format "%f" [expr $I_out - $prev_val]]
            .sim_dac.txt insert end "$t   $U_in   $U_out   $I_in   $I_out   $D_Iout\n"
            set time_ref [expr $time_ref + 2.0*$sig_unit]
            set prev_val $I_out
            incr step_ref
         }
      }
   }
   close $fp
   restore_cwd
   .sim_dac.txt insert end "--------------------------------------------------------------------------\n"
   .sim_dac.txt see end
}

proc change_cmd_file {} {
#-----------------------------------------------------------------------------#
# procedure to generate a window to change the generated cmd_file             #
#-----------------------------------------------------------------------------#
   global Fnt

   toplevel  .chgcmdf
   frame     .chgcmdf.fr        -bg wheat3 -bd 2 -relief raised
   button    .chgcmdf.fr.write  -bg wheat3 -text Write  -width 20\
		-command {
			set fp_cmd [open tmp.cmd "w"]
			puts $fp_cmd [.chgcmdf.txt get 1.0 end]
			close $fp_cmd
			destroy .chgcmdf
			}
   button    .chgcmdf.fr.cancel -bg wheat3 -text Cancel -width 20\
		-command {
			destroy .chgcmdf
			}
   text      .chgcmdf.txt  -bg wheat -width 80 -height 25\
                           -yscrollcommand ".chgcmdf.sb set" -font $Fnt
   scrollbar .chgcmdf.sb   -command ".chgcmdf.txt yview" -bg wheat

   pack .chgcmdf.fr        -side bottom -fill x
   pack .chgcmdf.fr.write  -side left   -padx 50 -pady 5
   pack .chgcmdf.fr.cancel -side right  -padx 50 -pady 5
   pack .chgcmdf.txt       -side left   -fill both -expand 1
   pack .chgcmdf.sb        -side right  -fill y

   set fp_cmd [open tmp.cmd]
   .chgcmdf.txt insert end [read $fp_cmd]
   close $fp_cmd
}
