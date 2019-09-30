
proc set_synth_opt {c_idx} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which a synthesis can be carried out    #
#-----------------------------------------------------------------------------#
   global Fnt SynthLibPath Carr CEarr CElist synopsysNetidFix Flatten path_to_dc_shell username

   if [catch {toplevel .synthopt}] {
	df_mess_dialog "NOTE: Only one Synthesize window may be open."
	return
   }
   set Flatten 0 ;# Flattening does not fully work yet
   set username $::env(USER)

   upd_status_line "Synthesis: BUSY"
   set e_name [lindex $Carr($c_idx) 1]
   set a_name [lindex $Carr($c_idx) 2]
   set CElist $CEarr($c_idx)

   set synopsysNetidFix 0
   if {$synopsysNetidFix || [file exists /data/ewi-practica]} {
     # For Linux != 2.x we have a problem with the Synopsys software
     # in combination with a netid
	set synopsysNetidFix 1
	set fp_which [open "|which dc_shell"]
	set path_to_dc_shell [gets $fp_which]
	if {$path_to_dc_shell == ""} { set path_to_dc_shell "dc_shell" }
	catch {close $fp_which}
   }
   set srcLib "$SynthLibPath/g_digilib5_99.db"

   frame    .synthopt.llfr
   frame    .synthopt.llfr.lfr         -relief raised -bd 2 -bg wheat3
   frame    .synthopt.llfr.sfr         -relief raised -bd 2 -bg wheat3
   frame    .synthopt.lftfr            -bg wheat
   frame    .synthopt.lftfr.kfr        -relief raised -bd 2
   frame    .synthopt.lftfr.efr        -relief raised -bd 2
   label    .synthopt.llfr.lfr.lbl     -text "module:" -font $Fnt -bg wheat3
   label    .synthopt.llfr.lfr.mod     -text $e_name -width 25 -font $Fnt -bg wheat3
   label    .synthopt.llfr.sfr.lbl     -text "source_library:" -font $Fnt -bg wheat3
 # entry    .synthopt.llfr.sfr.en      -width 60 -font $Fnt -bg wheat3
   label    .synthopt.llfr.sfr.en      -text $srcLib -width [string len $srcLib] -font $Fnt -bg wheat3
   # checkbutton .synthopt.lftfr.kfr.cb -text "flatten" -variable Flatten\
   #                                     -width 15 -anchor w -font $Fnt -bg wheat3
   button   .synthopt.lftfr.rd         -text "ReadSynthScript"\
                                       -command "do_read_script" -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.mk         -text "MakeSynthScript"\
                                       -command "do_make_script $a_name" -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.ok         -text "Synthesize" -state disabled\
                                       -command do_synthesize -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.pl         -text "Show circuit" -state disabled\
                                       -command show_circuit -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.cp         -text "Compile" -state disabled\
                                       -command comp_synth -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.ac         -text "Parse_sls" -state disabled\
                                       -command syn_compile_sls -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.wrt        -text "Write logfile"\
                                       -command do_write_logfile -width 15 -font $Fnt -bg wheat3
   button   .synthopt.lftfr.cancel     -text "Cancel"\
                                       -command {destroy .synthopt; upd_status_line ""}\
                                       -width 15 -font $Fnt -bg wheat3
   text      .synthopt.txt             -width 80 -height 25 -font $Fnt\
                                       -yscrollcommand ".synthopt.sb set" -bg wheat
   scrollbar .synthopt.sb              -command ".synthopt.txt yview" -bg wheat

   pack .synthopt.llfr             -side top -fill x
   pack .synthopt.llfr.lfr         -side left
   pack .synthopt.llfr.sfr         -side right
   pack .synthopt.lftfr            -side left   -fill y
   pack .synthopt.lftfr.kfr        -side top -fill x -padx 5 -pady 3
   pack .synthopt.llfr.sfr         -side bottom -fill x
   pack .synthopt.llfr.lfr.lbl     -side top -fill x
   pack .synthopt.llfr.lfr.mod     -side bottom -pady 10 -padx 10
   pack .synthopt.llfr.sfr.lbl     -side top -fill x
   pack .synthopt.llfr.sfr.en      -side bottom -pady 10
 # pack .synthopt.lftfr.kfr.cb     -side top -fill x
   pack .synthopt.lftfr.cancel     -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.wrt        -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.ac         -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.cp         -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.pl         -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.ok         -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.mk         -side bottom -padx 5 -pady 3
   pack .synthopt.lftfr.rd         -side bottom -padx 5 -pady 3
   pack .synthopt.sb               -side right -fill y
   pack .synthopt.txt              -side right -fill both -expand 1

   if {[file exists ADB/$e_name.ddc]} {
      # If this file exists, the circuit has been synthesized and we can show it.
      .synthopt.lftfr.pl configure -state normal
   }
 # .synthopt.llfr.sfr.en insert end $srcLib
   wm title .synthopt "synthesizer options"
}

proc do_write_logfile {} {
#-----------------------------------------------------------------------------#
# procedure to save the log_file                                              #
#-----------------------------------------------------------------------------#
   global MyWd

   set e_name [.synthopt.llfr.lfr.mod cget -text]
   set log_file "$MyWd/$e_name.synlog"
   set fp_log [open $log_file w]
   puts $fp_log [.synthopt.txt get 1.0 end]
   close $fp_log
}

proc check_cellname_to_synthesize {} {
#-----------------------------------------------------------------------------#
# procedure to check synthesize script on-screen for the correct cell         #
#-----------------------------------------------------------------------------#
   foreach tt [split [.synthopt.txt get 1.0 end] \n] {
      if {[string first "cell:" $tt] > 0} {
         set idx [lsearch $tt "cell:"]
         return [lindex $tt $idx+1]
      }
   }
   return ""
}

proc do_read_script {} {
#-----------------------------------------------------------------------------#
# procedure to read the last used synthesize script                           #
#-----------------------------------------------------------------------------#
   .synthopt.txt delete 1.0 end
   .synthopt.lftfr.ok configure -state disabled

   set fn "synthesize.tcl"
   if {[catch {set fp [open $fn]}]} {
      df_mess_dialog "INSTRUCTION:\n\
	cannot read $fn script: use MakeSynthScript"
      return
   }
   while {1} {
      set tt [gets $fp]
      if {[eof $fp]} break
      .synthopt.txt insert end "$tt\n"
   }

   set cname [check_cellname_to_synthesize]
   if {$cname == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	cannot find a cellname: use MakeSynthScript"
      return
   } elseif {$cname ne [.synthopt.llfr.lfr.mod cget -text]} {
      df_mess_dialog "WARNING:\n\
	Wrong synthesize script:\n\
	it is for the cell $cname\n\
	Edit or use MakeSynthScript"
   }
   .synthopt.lftfr.ok configure -state normal
   .synthopt.lftfr.cp configure -state disabled
   .synthopt.lftfr.ac configure -state disabled
}

proc show_circuit {} {
#-----------------------------------------------------------------------------#
# procedure to show the synthesized circuit.                                  #
#-----------------------------------------------------------------------------#
   global PsViewer1 PsViewer2 MyWd synopsysNetidFix path_to_dc_shell username

   set e_name [.synthopt.llfr.lfr.mod cget -text]

   restore_cwd

   if ($synopsysNetidFix) {
      foreach i [glob -nocomplain /tmp/$username/*] {
         file delete -force $i
      }
      set MyWd_effect /tmp/$username/$MyWd
      file mkdir $MyWd_effect/ADB
      file copy ADB/$e_name.ddc $MyWd_effect/ADB

      exec xhost local:ewi-practica
      set dc_show_cmd "sudo -u ewi-practica sh -c \"umask 0000; cd $MyWd_effect; xterm -e $path_to_dc_shell -f plot.tcl\""
   } else {
      set dc_show_cmd "xterm -e dc_shell -f plot.tcl"
   }

   set fn "plot.tcl"
   set fpc [open $fn "w"]
   puts $fpc "read_file -f ddc ADB/$e_name.ddc"
   puts $fpc "set designer $username"
   puts $fpc "set company TU-Delft"
   puts $fpc "start_gui"
   puts $fpc "gui_create_schematic"
   puts $fpc "# Use 'Schematic -> Move Down' to see contents of a block"
   puts $fpc "# To quit: click File -> Exit in Design Vision window"
   puts $fpc "# or type quit in dc_shell window"
   close $fpc

   if ($synopsysNetidFix) {
      file copy $fn $MyWd_effect
      exec chmod -R go+wrx /tmp/$username
   }

   set fp_show [open "|$dc_show_cmd"]
   while {![eof $fp_show]} {
      set dummy [gets $fp_show]
      .synthopt.txt insert end "$dummy\n"
      .synthopt.txt see end
      update idletasks
   }
   catch {close $fp_show}
   restore_cwd

   if ($synopsysNetidFix) {
      # file copy -force $MyWd_effect/$e_name.ps $e_name.ps
      foreach i [glob -nocomplain /tmp/$username/*] {
         file delete -force $i
      }
   }

   # catch {exec dos2unix $e_name.ps }
   # if {[auto_execok $PsViewer1] != ""} {
   #    exec $PsViewer1 $e_name.ps &
   # } elseif {[auto_execok $PsViewer2] != ""} {
   #    exec $PsViewer2 $e_name.ps &
   # } else {
   #    df_mess_dialog "Cannot run $PsViewer1 or $PsViewer2 to display circuit"
   # }
}

proc do_make_script {a_name} {
#-----------------------------------------------------------------------------#
# procedure to make a synthesize script                                       #
#-----------------------------------------------------------------------------#
   global SynthLibPath Flatten Parr CElist Earr Aarr username

   .synthopt.txt delete 1.0 end
   .synthopt.lftfr.ok configure -state disabled
#
# check to see if a target_library is present
#
#  set source_lib_name [.synthopt.llfr.sfr.en get]
#  if {$source_lib_name != "$SynthLibPath/g_digilib5_99.db"} {
#     df_mess_dialog "WARNING:\n\
#	There is another source library_name given:\n\
#	The source library_name can't be changed."
#  }

   set e_name [.synthopt.llfr.lfr.mod cget -text]
   set e_idx [get_earr_idx $e_name]
   if {$e_idx < 0} { df_mess_dialog "ERROR:\n No index for $e_name found"; return }
   set a_idx [get_aarr_idx $e_idx $a_name]
   if {$a_idx < 0} { df_mess_dialog "ERROR:\n No index for $a_name found"; return }
#
# make a header
#
   set hdrstr "#*********************************************************\n"
   .synthopt.txt insert end $hdrstr
   .synthopt.txt insert end "# synthesize script for cell: $e_name\n"
   .synthopt.txt insert end $hdrstr
#
# set some global items and specify the synthesis libraries
#
   .synthopt.txt insert end "set hdlin_vhdl_93 false\n"
# synopsys manual says the default for the following is true,
# but not specifying these variables gives a different result.
# if the following variables are not specified, ff's with reset
# are not inferred and ModelSim simulations may give X-es.
   .synthopt.txt insert end "set hdlin_ff_always_async_set_reset true\n"
   .synthopt.txt insert end "set hdlin_ff_always_sync_set_reset true\n"
   .synthopt.txt insert end "set vhdlout_architecture_name \"synthesised\"\n"
   .synthopt.txt insert end "set vhdlout_use_packages {\"ieee.std_logic_1164\" \"CellsLib.CellsLib_DECL_PACK\"}\n"
   .synthopt.txt insert end "set company \"ontwerp_practicum\"\n"
   .synthopt.txt insert end "set designer \"$username\"\n"
   .synthopt.txt insert end "set target_library {\"$SynthLibPath/g_digilib5_99.db\"}\n"

   .synthopt.txt insert end "set link_library \[list \"*\" \\\n"
   if {$CElist != ""} {
      for {set i 0} {$i < [llength $CElist]} {incr i} {
         set subent [lindex $CElist $i]
         set adb_file ADB/$subent.ddc
         if {![file exists $adb_file]} {
            df_mess_dialog "INSTRUCTION:\n\
                subcell $subent has not yet been synthesized:\n\
                synthesize this cell first"
            .synthopt.txt delete 1.0 end
            return
         }
         .synthopt.txt insert end "\"$adb_file\" \\\n"
      }
   }
   .synthopt.txt insert end "\"$SynthLibPath/g_digilib5_99.db\" \\\n"
   .synthopt.txt insert end "\"$SynthLibPath/g_analib8_00.db\" \\\n"
   .synthopt.txt insert end "\"$SynthLibPath/buffers.db\"\]\n"

   .synthopt.txt insert end "define_design_lib MY_LIB -path syn_work\n"
   .synthopt.txt insert end "define_design_lib CELLSLIB -path $SynthLibPath/CellsLib\n"

# determine which files to use for the synthesis of the cell
# the subcells must have been synthesized first.
# if this is not the case a message will be generated and the synthesis
# will not be caried out
#
   set e_file [lindex $Earr($e_idx) 1]
   set a_file [lindex $Aarr($a_idx) 2]
   if {$a_file eq $e_file} { set a_file "" }

   set sz [array size Parr]
   for {set i 0} {$i < $sz} {incr i} {
      set p_file [lindex $Parr($i) 1]
      .synthopt.txt insert end "read_file -format vhdl -work MY_LIB $p_file\n"
   }
   .synthopt.txt insert end "read_file -format vhdl -work MY_LIB {$e_file $a_file}\n"

   if {!$Flatten && $CElist != ""} {
      .synthopt.txt insert end "set_dont_touch \{$CElist\}\n"
      .synthopt.txt insert end "# following prevents the insertion of buffers.\n"
      .synthopt.txt insert end "set structcells \[filter_collection \[get_cells *\] \"is_hierarchical == true\"\]\n"
      .synthopt.txt insert end "set_dont_touch \[get_nets -of_objects \$structcells\]\n"
   }
   .synthopt.txt insert end "set_dont_touch \{g_analib8_00/*\}\n"
   .synthopt.txt insert end "# set_max_fanout 1.8 all_inputs()\n"
   .synthopt.txt insert end "# set_max_area 1000\n"

   if {!$Flatten} {
      .synthopt.txt insert end "set compile_preserve_subdesign_interfaces true\n"
   }
   .synthopt.txt insert end "compile\n"
   # To flatten Design Ware blocks we have to ungroup after compile.
   .synthopt.txt insert end "ungroup -all -flat\n"
   .synthopt.txt insert end "report_area\n"
   .synthopt.txt insert end "report_fsm\n"
   .synthopt.txt insert end "write_file -f ddc  $e_name -output ADB/$e_name.ddc\n"
   .synthopt.txt insert end "write_file -f vhdl $e_name -output VHDL/$e_name\_SYNTH.vhd\n"
   .synthopt.txt insert end "quit\n"

   .synthopt.lftfr.ok configure -state normal
   .synthopt.lftfr.cp configure -state disabled
   .synthopt.lftfr.ac configure -state disabled
}

proc do_synthesize {} {
#-----------------------------------------------------------------------------#
# procedure to perform a synthesis                                            #
#-----------------------------------------------------------------------------#
   global MyWd synopsysNetidFix path_to_dc_shell username

   set e_name [.synthopt.llfr.lfr.mod cget -text]

   if {[check_cellname_to_synthesize] ne $e_name} {
      df_mess_dialog "ERROR:\n\
	Wrong synthesize script:\n\
	cannot find the cell '$e_name'\n\
	Edit or generate a new one first"
      return
   }
   restore_cwd

# save the synthesize script shown to the file synthesize.tcl
   set fn "synthesize.tcl"
   if {[catch {set fpc [open $fn "w"]}]} {
      df_mess_dialog "ERROR:\n\
	cannot write $fn script\n\
	this should not happen (maybe readonly)"
      return
   }
   puts -nonewline $fpc [.synthopt.txt get 1.0 end]
   close $fpc

#
# then call the synthesizer with the tcl_file made as the input
#
   .synthopt.lftfr.ok configure -state disabled
   .synthopt.txt insert end "\n----- invoking synthesizer .....\n\n"
   .synthopt.txt see end
   .synthopt.txt tag configure ERR_TAG -foreground "red" -background "white"
   update

   if ($synopsysNetidFix) {
      foreach i [glob -nocomplain /tmp/$username/*] {
         file delete -force $i
      }
      set MyWd_effect /tmp/$username/$MyWd
      file mkdir $MyWd_effect
      file copy $fn ADB VHDL syn_work $MyWd_effect
      #----------------------------------
      exec chmod -R go+wrx /tmp/$username
      #----------------------------------
      set dc_shell_cmd "sudo -u ewi-practica sh -c \"umask 0000; cd $MyWd_effect; $path_to_dc_shell -f $fn\""
   } else {
      set dc_shell_cmd "dc_shell -f $fn"
   }

   set fp_cmd [open "dc_shell_cmd" "w"]
   puts  $fp_cmd $dc_shell_cmd
   close $fp_cmd

   set nr_errs 0
   set fp_syn [open "|$dc_shell_cmd" "r+"]
   while {1} {
      set dummy [gets $fp_syn]
      if {[eof $fp_syn]} break
      .synthopt.txt insert end "$dummy\n"
      .synthopt.txt see end
      update
      if {([string first "error" [string tolower $dummy]] >= 0) || \
          ([string first "fatal" [string tolower $dummy]] >= 0)} {
         set ln [lindex [split [.synthopt.txt index "end-2line"] .] 0]
         .synthopt.txt tag add ERR_TAG $ln.0 $ln.end
         incr nr_errs
         if {[string first "(UI-21)" $dummy] >= 0} {
             puts $fp_syn "quit\n"
             flush $fp_syn
         }
      } elseif {[string first "(TRANS-1)" $dummy] >= 0} {
         set ln [lindex [split [.synthopt.txt index "end-2line"] .] 0]
         .synthopt.txt tag add ERR_TAG $ln.0 $ln.end
         incr nr_errs
      } elseif {[string first "(TRANS-4)" $dummy] >= 0} {
         set ln [lindex [split [.synthopt.txt index "end-2line"] .] 0]
         .synthopt.txt tag add ERR_TAG $ln.0 $ln.end
         incr nr_errs
      }
   }
   catch {close $fp_syn}
   restore_cwd

   if {$nr_errs == 0} {
      .synthopt.txt insert end "******* SYNTHESIS DONE ******\n"
   } else {
      .synthopt.txt insert end "****** ERRORS found during synthsis *****\n"
   }
   .synthopt.txt see end
   update
   if {$nr_errs != 0} { return }

   if ($synopsysNetidFix) {
      file delete -force   syn_work_bak
      file rename syn_work syn_work_bak
      file copy -force $MyWd_effect/syn_work .
      file copy -force $MyWd_effect/ADB/$e_name.ddc ADB
      file copy -force $MyWd_effect/VHDL/$e_name\_SYNTH.vhd VHDL

      foreach i [glob -nocomplain /tmp/$username/*] {
         file delete -force $i
      }
   }
#
# Show the db_files that have been (re)written
#
   .synthopt.txt insert end "\n|=============================================================\n"
   .synthopt.txt insert end "| db_file (re)written:\n"
   .synthopt.txt insert end "|-------------------------------------------------------------\n"
   .synthopt.txt insert end "| ADB/$e_name.ddc\n"
   .synthopt.txt insert end "|=============================================================\n\n"
   .synthopt.txt see end
   update
#
# make the sls_files from the vhdl_descriptions
#
   set vhd_file "VHDL/$e_name\_SYNTH.vhd"
   .synthopt.txt insert end "\n|=============================================================\n"
   .synthopt.txt insert end "| sls_file (re)written:\n"
   .synthopt.txt insert end "|-------------------------------------------------------------\n"
   generate_sls $e_name "" $vhd_file
   .synthopt.txt insert end "| SLS/$e_name.sls\n"
   .synthopt.txt insert end "|=============================================================\n\n"
   .synthopt.txt see end
   update
#
# remove the entities of cells that exist in the database
#
   .synthopt.txt insert end "\n|=============================================================\n"
   .synthopt.txt insert end "| vhdl_file (re)written:\n"
   .synthopt.txt insert end "|-------------------------------------------------------------\n"
   update_vhdl_file $e_name
   .synthopt.txt insert end "| $vhd_file\n"
   .synthopt.txt insert end "|=============================================================\n\n"
   .synthopt.txt see end
   update
   .synthopt.lftfr.cp configure -state normal
   .synthopt.lftfr.ac configure -state normal
   .synthopt.lftfr.pl configure -state normal
}

proc comp_synth {} {
#-----------------------------------------------------------------------------#
# procedure to compile the latest synthesized files                           #
#-----------------------------------------------------------------------------#
   global SimLibName

   set e_name [.synthopt.llfr.lfr.mod cget -text]
   set vhd_file "VHDL/$e_name\_SYNTH.vhd"
  .synthopt.txt insert end "\n-- compiling $vhd_file --\n"
  .synthopt.txt see end
   update
   if {[do_vcom $SimLibName $vhd_file] > 0} {
     .synthopt.txt insert end "-- compiling done --\n"
     .synthopt.txt see end
      read_infofile
   } else {
     .synthopt.txt insert end "-- errors in the compilation of $vhd_file --\n"
     .synthopt.txt see end
   }
}

proc update_vhdl_file {e_name} {
#-----------------------------------------------------------------------------#
# procedure to update a generated vhdl_file                                   #
#-----------------------------------------------------------------------------#

   outdate_cir $e_name

   # I don't understand what is done below here (AvG 19 july 2012)

   .scratch delete 1.0 end
   set vhd_file "VHDL/$e_name\_SYNTH.vhd"
   set fp [open $vhd_file r]
   set do_copy 1
   while {![eof $fp]} {
      set txt [gets $fp]
      set t0 [string tolower [lindex $txt 0]]
      if {[string equal $t0 "package"] || [string equal $t0 "entity"]} {
         set do_copy 0
      } elseif {[string equal $t0 "architecture"]} {
         set do_copy 1
      }
      if {$do_copy} { .scratch insert end "$txt\n" }
   }
   close $fp
   set fp [open $vhd_file w]
   puts $fp [.scratch get 1.0 end]
   close $fp
}

proc remove_empty {in_list} {
#-----------------------------------------------------------------------------#
# procedure to remove 'empty' items from a list                               #
#-----------------------------------------------------------------------------#
   for {set i [expr [llength $in_list]-1]} {$i >= 0} {incr i -1} {
      if {[lindex $in_list $i] == ""} {
         set in_list [lreplace $in_list $i $i]
      }
   }
   return $in_list
}

proc print_stmnt {position token_list line_len} {
#-----------------------------------------------------------------------------#
# procedure to print a statement in a pretty format                           #
#-----------------------------------------------------------------------------#
   set filled_to 0
   set print_str ""
   for {set i 0} {$i < [llength $token_list]} {incr i} {
      set len_token [string length [lindex $token_list $i]]
      if {$i == 0} {
         set init_len $len_token
      }
      if {$filled_to + $len_token <= $line_len} {
         append print_str [lindex $token_list $i]
         set filled_to [expr $filled_to + $len_token]
      } else {
         append print_str "\n"
         for {set ii 0} {$ii < $init_len} {incr ii} {
         append print_str " "
         }
         append print_str "[lindex $token_list $i]"
         set filled_to [expr $init_len + $len_token]
      }
   }
   .scratch insert $position "$print_str\n"
}

proc mk_external_str {token_list} {
#-----------------------------------------------------------------------------#
# procedure to make a sls_external statement                                  #
#-----------------------------------------------------------------------------#
   set first_term 3
   set out_str ""
   lappend out_str "extern network [lindex $token_list 1] (terminal "
   for {set i 0} {$i < [llength $token_list]-1} {incr i} {
      set token [string tolower [lindex $token_list $i]]
      if {[string equal $token "in"] || [string equal $token "out"]
           || [string equal $token "inout"]} {
        set next_token [string tolower [lindex $token_list $i+1]]
        for {set j $first_term} {$j < $i} {incr j} {
            set tnm [lindex $token_list $j]
            if {[string equal $next_token "std_logic"]} {
               lappend out_str "$tnm, "
            }\
            else {
               if {[string equal -nocase [lindex $token_list $i+3] "downto"]} {
                  set lr [lindex $token_list $i+4]
                  set hr [lindex $token_list $i+2]
                  for {set idx $hr} {$idx >= $lr} {incr idx -1} {
                     lappend out_str "${tnm}_${hr}_${lr}_$idx, "
                  }
               } else {
                  set lr [lindex $token_list $i+2]
                  set hr [lindex $token_list $i+4]
                  for {set idx $hr} {$idx >= $lr} {incr idx -1} {
                     lappend out_str "${tnm}_${hr}_${lr}_$idx, "
                  }
               }
            }
         }
         if {[string equal $next_token "std_logic"]} {
            set first_term [expr $i+2]
         } else {
            set first_term [expr $i+5]
         }
      }
   }
   lappend out_str "vss, "
   lappend out_str "vdd)"
   return $out_str
}

proc mk_netw_str {cell_name token_list} {
#-----------------------------------------------------------------------------#
# procedure to make a sls_network statement                                   #
# also produces the .term statement for the circuit                           #
#-----------------------------------------------------------------------------#
   upvar idx1_arr idx1_arr
   upvar idx2_arr idx2_arr
   set fpt [open .$cell_name.term "w"]
   set first_term [expr [llength $token_list] + 1]
   set out_str ""
   lappend out_str "network [lindex $token_list 1] (terminal "
   for {set i 0} {$i < [llength $token_list]-1} {incr i} {
      set token [string tolower [lindex $token_list $i]]
      if {[string equal $token "port"]} {
          set first_term [expr $i+1]
      }
      if {[string equal $token "in"] || [string equal $token "out"]
           || [string equal $token "inout"]} {
         set next_token [string tolower [lindex $token_list $i+1]]
         switch $token {
            in      {set type "I"}
            out     {set type "O"}
            default {set type "B"}
         }
         for {set j $first_term} {$j < $i} {incr j} {
            set tnm [lindex $token_list $j]
            if {[string equal $next_token "std_logic"]} {
               lappend out_str "$tnm, "
               puts $fpt "$type $tnm $tnm"
            }\
            else {
               if {[string equal -nocase [lindex $token_list $i+3] "downto"]} {
                  set lr [lindex $token_list $i+4]
                  set hr [lindex $token_list $i+2]
                  set idx1_arr($tnm) $hr
                  set idx2_arr($tnm) $lr
                  for {set idx $hr} {$idx >= $lr} {incr idx -1} {
                     lappend out_str "${tnm}_${hr}_${lr}_$idx, "
                     puts $fpt "$type ${tnm}_${hr}_${lr}_$idx ${tnm}($idx)"
                  }
               } else {
                  set lr [lindex $token_list $i+2]
                  set hr [lindex $token_list $i+4]
                  set idx1_arr($tnm) $lr
                  set idx2_arr($tnm) $hr
                  for {set idx $lr} {$idx <= $hr} {incr idx} {
                     lappend out_str "${tnm}_${hr}_${lr}_$idx, "
                     puts $fpt "$type ${tnm}_${hr}_${lr}_$idx ${tnm}($idx)"
                  }
               }
            }
         }
         if {[string equal $next_token "std_logic"]} {
            set first_term [expr $i+2]
         } else {
            set first_term [expr $i+5]
         }
      }
   }
   lappend out_str "vss, "
   lappend out_str "vdd)"
   close $fpt
   return $out_str
}

proc mk_instance_str {token_list} {
#-----------------------------------------------------------------------------#
# procedure to make a sls_instance statement                                  #
#-----------------------------------------------------------------------------#
   upvar idx1_arr idx1_arr
   upvar idx2_arr idx2_arr
   lappend token_list "dummy"
   lappend out_str "   \{[lindex $token_list 0]\} [lindex $token_list 1] ("

   for {set i 0} {$i < [llength $token_list]-1} {incr i} {
     if {[lindex $token_list $i] == "=>"} {
      set nm_token [lindex $token_list $i+1]
      if {[array names idx1_arr $nm_token] == ""} {
         lappend out_str "$nm_token, "
      } else {
         set idx_1 $idx1_arr($nm_token)
         set idx_2 $idx2_arr($nm_token)
         if {[regexp ^\[0-9\]*\$ [lindex $token_list $i+2]]} {
            set name "${nm_token}_${idx_1}_${idx_2}_[lindex $token_list $i+2]"
            lappend out_str "$name, "
         }\
         elseif {$idx_1 > $idx_2} {
            for {set j $idx_1} {$j >= $idx_2} {incr j -1} {
               set name "${nm_token}_${idx_1}_${idx_2}_$j"
               lappend out_str "$name, "
            }
         } else {
            for {set j $idx_1} {$j <= $idx_2} {incr j} {
               set name "${nm_token}_${idx_1}_${idx_2}_$j"
               lappend out_str "$name, "
            }
         }
      }
     }
   }
   lappend out_str "vss, "
   lappend out_str "vdd);"
   return $out_str
}

proc replace_1_0 {rep_list} {
#-----------------------------------------------------------------------------#
# procedure to replce '0' and '1' by vss and vdd in a list                    #
#-----------------------------------------------------------------------------#
   for {set t 0} {$t < [llength $rep_list]} {incr t} {
      if {[lindex $rep_list $t] == "'0'"} {
         set rep_list [lreplace $rep_list $t $t "vss"]
      }\
      elseif {[lindex $rep_list $t] == "'1'"} {
         set rep_list [lreplace $rep_list $t $t "vdd"]
      }
   }
   return $rep_list
}

proc generate_sls {cell_name e_name vhdl_file_name} {
#-----------------------------------------------------------------------------#
# procedure to generate sls_code from a generated vhdl_file                   #
#-----------------------------------------------------------------------------#
   set has_instances 0
   set LINELEN 80
   set t_text ""
   .scratch delete 1.0 end
   if {$e_name != ""} {
      set fp [open "VHDL/$e_name.vhd" "r"]
      set morestuff 1
   } else {
      set fp [open $vhdl_file_name "r"]
      set morestuff 0
   }
   set state "INITIAL"
   while {![eof $fp]} {
      set txt [gets $fp]

      regsub -- "--.*" $txt "" txt
      regsub -all ":" $txt " & " txt
      regsub -all ";" $txt " & " txt
      regsub -all "=>" $txt " & " txt
      regsub -all "<=" $txt " & " txt
      regsub -all "\\(" $txt " & " txt
      regsub -all "\\)" $txt " & " txt
      regsub -all "\t" $txt "   " txt

      if {[string equal -nocase [lindex $txt 0] "entity"]} {
         set state "ENT_FND"
         set e_tokens ""
      }\
      elseif {($state == "ENT_FND") && ([string equal -nocase [lindex $txt 0] "end"])} {
         set state "ENT_DONE"
      }\
      elseif {[string equal -nocase [lindex $txt 0] "component"]} {
         set state "COMP_FND"
         set c_tokens ""
      }\
      elseif {[string equal -nocase [string trimright [lindex $txt 1] ";"] "component"]} {
         set state "COMP_DONE"
      }\
      elseif {[string equal -nocase [lindex $txt 0] "begin"]} {
         set state "BGN_FND"
      }\
      elseif {($state == "BGN_FND") && ([lindex $txt 1] == ":")} {
         set t_text $txt
         while {[string first ";" $t_text] < 0} {
            append t_text [gets $fp]
            regsub -- "--.*" $t_text "" t_text
            regsub -all ":" $t_text " & " t_text
            regsub -all ";" $t_text " & " t_text
            regsub -all "=>" $t_text " & " t_text
            regsub -all "<=" $t_text " & " t_text
            regsub -all "\\(" $t_text " & " t_text
            regsub -all "\\)" $t_text " & " t_text
            regsub -all "\t" $t_text "   " t_text
         }
         set state "INST_DONE"
      }\
      elseif {($state == "BGN_FND") && ([lindex $txt 1] == "<=")} {
         set state MK_NET
      }\
      elseif {($state == "BGN_FND") && ([string equal -nocase [lindex $txt 0] "end"])} {
         set state "READY"
      }\
      elseif {[string equal -nocase [lindex $txt 0] "signal"]} {
          set idx_s [string first "(" $txt]
          if {$idx_s > 0} {
             set idx_e [string first ")" $txt]
             set rng [string range $txt $idx_s+1 $idx_e-1]
             set i1 [lindex $rng 0]
             set i2 [lindex $rng 2]

	     set i [string first ":" $txt]
	     set subtxt [string range $txt 0 $i-1]
	     regsub -all "," $subtxt " " subtxt
	     set i 0
	     foreach t_name $subtxt {
		if {[incr i] > 1} {
		   set idx1_arr($t_name) [expr $i1]
		   set idx2_arr($t_name) [expr $i2]
		}
	     }
         }
      }
      switch $state {
         "COMP_FND"  { lappend c_tokens $txt
                     }
         "COMP_DONE" { set token_list [split $c_tokens " \t(){},;:"]
                       set token_list [remove_empty $token_list]
                       set ext_list [mk_external_str $token_list]
                       print_stmnt 1.0 $ext_list $LINELEN
                       set c_tokens ""
                       set state "INITIAL"
                       set has_instances 1
                     }
         "ENT_FND"   { lappend e_tokens $txt
                     }
         "ENT_DONE"  { set token_list [split $e_tokens " \t(){},;:"]
                       set token_list [remove_empty $token_list]
                       set ext_list [mk_netw_str $cell_name $token_list]
                       print_stmnt end $ext_list $LINELEN
                       set e_tokens ""
                       .scratch insert end "\{\n"
                       set state "INITIAL"
                     }
         "INST_DONE" { set token_list [split $t_text " \t(){},;:\n"]
                       set token_list [remove_empty $token_list]
                       set ext_list [mk_instance_str $token_list]
                       print_stmnt end $ext_list $LINELEN
                       set state "BGN_FND"
                    }
         "MK_NET"   { set net_str $txt
                      while {[string first ";" $net_str] < 0} {
                         append net_str [gets $fp]
                         regsub -- "--.*" $net_str "" net_str
                         regsub -all ":" $net_str " : " net_str
                         regsub -all ";" $net_str " ; " net_str
                         regsub -all "=>" $net_str " => " net_str
                         regsub -all "<=" $net_str " <= " net_str
                         regsub -all "\t" $net_str "   " net_str
                      }
                      set idx [string first "<" $net_str]
                      set from_str [string range $net_str 0 $idx-1]
                      set to_str   [string range $net_str $idx+2 end]
                      set from_tokens [split $from_str " \t(){},;:"]
                      set from_tokens [remove_empty $from_tokens]
                      set to_tokens [split $to_str " \t(){},;:"]
                      set to_tokens [remove_empty $to_tokens]
                      set to_tokens [replace_1_0 $to_tokens]
                      for {set it 1} {$it < [llength $to_tokens]} {incr it} {
                         if {[string is integer [lindex $to_tokens $it]] == 1} {
                            set itn [expr $it-1]
                            set tkn [lindex $to_tokens $itn]
                            set tki [lindex $to_tokens $it]
                            set idx1 $idx1_arr($tkn)
                            set idx2 $idx2_arr($tkn)
                            set ntkn "${tkn}_${idx1}_${idx2}_$tki"
                            set to_tokens [lreplace $to_tokens $itn $it $ntkn]
                         }
                      }
                      set nm_from_tokens [lindex $from_tokens 0]
                      set nm_to_tokens [lindex $to_tokens 0]

                      if {[llength $from_tokens] == 1} {
                         if {[llength $to_tokens] == 1} {
                            # case a <= b
                            if {[info exists idx1_arr($nm_from_tokens)]} {
                               set idx1_f $idx1_arr($nm_from_tokens)
                               set idx2_f $idx2_arr($nm_from_tokens)
                               set idx1_t $idx1_arr($nm_to_tokens)
                               set idx2_t $idx2_arr($nm_to_tokens)
                               set i_f $idx1_f
                               set i_t $idx1_t
                               while (1) {
                                  set from "${nm_from_tokens}_${idx1_f}_${idx2_f}_$i_f"
                                  set to "${nm_to_tokens}_${idx1_t}_${idx2_t}_$i_t"
                                  .scratch insert end "   net \{$from, $to\};\n"
                                  if {$i_f == $idx2_f} { break }
                                  if {$idx1_f < $idx2_f} { incr i_f }\
                                  else { incr i_f -1 }
                                  if {$idx1_t < $idx2_t} { incr i_t }\
                                  else { incr i_t -1 }
                               }
                            }\
                            else {
                               .scratch insert end "   net \{$from_tokens, $to_tokens\};\n"
                            }
                         }\
                         else {
                            # case a <= {b, c, d} where a is a vector
                            set idx1_f $idx1_arr($nm_from_tokens)
                            set idx2_f $idx2_arr($nm_from_tokens)
                            set i 0
                            if {$idx1_f > $idx2_f} {
                               for {set j $idx1_f} {$j >= $idx2_f} {incr j -1} {
                                  set from "${nm_from_tokens}_${idx1_f}_${idx2_f}_$j"
                                  set to   "[lindex $to_tokens $i]"
                                  .scratch insert end "   net \{$from, $to\};\n"
                                  incr i
                               }
                            }\
                            else {
                               for {set j $idx1_f} {$j >= $idx2_f} {incr j} {
                                  set from "${nm_from_tokens}_${idx2_f}_${idx1_f}_$j"
                                  set to   "[lindex $to_tokens $i]"
                                  .scratch insert end "   net \{$from, $to\};\n"
                                  incr i
                               }
                            }
                         }
                      }\
                      else {
                         # case a(2) <= b
                         set idx1_f $idx1_arr($nm_from_tokens)
                         set idx2_f $idx2_arr($nm_from_tokens)
                         set ii [lindex $from_tokens 1]
                         if {$idx1_f > $idx2_f} {
                            set from "${nm_from_tokens}_${idx2_f}_${idx1_f}_$ii"
                         }\
                         else {
                            set from "${nm_from_tokens}_${idx2_f}_${idx1_f}_$ii"
                         }
                         .scratch insert end "   net \{$from, $to_tokens\};\n"
                      }
                      set state BGN_FND
                    }
      "READY"       {  if {$has_instances == 0} {
                          set token_list {entity empty_fbcell in vss in vdd}
                          set ext_list [mk_external_str $token_list]
                          print_stmnt 1.0 $ext_list $LINELEN
                          set token_list {emp_fbcell_1 empty_fbcell vss vss vdd vdd}
                          set ext_list [mk_instance_str $token_list]
                          print_stmnt end $ext_list $LINELEN
                       }
                       .scratch insert end  "\}\n"
                       close $fp
                       set fp_sls [open SLS/$cell_name.sls "w"]
                       puts $fp_sls "[.scratch get 1.0 end]\n"
                       close $fp_sls
                       return
                      }
      }
      if {[eof $fp] && $morestuff == 1} {
         close $fp
         set fp [open $vhdl_file_name "r"]
         set morestuff 0
      }
   }
}

proc syn_compile_sls {} {
   global MyWd

   set cell [.synthopt.llfr.lfr.mod cget -text]
   set file_name "SLS/$cell.sls"
   .synthopt.txt insert end "\n-- parsing $file_name --\n"
   compile_sls $cell "synthesised" $MyWd/$file_name
   .synthopt.txt insert end "-- parsing done --\n"
   .synthopt.txt see end
   update
}

proc compile_sls {cell_name arch_name sls_file_name} {
#-----------------------------------------------------------------------------#
# procedure to compile the latest synthesized sls_files                       #
#-----------------------------------------------------------------------------#
   global DbName

   if {$DbName == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There is no database specified:\n\
          Make or set one by clicking the Make_database or\n\
          Database button"
      return
   }
   goto_dbdir
   upd_status_line "compiling $sls_file_name ..."
   # set res [catch {exec csls $sls_file_name} mess]
   set res [catch {exec csls $sls_file_name >& csls.messages}]
   if {$res} {
      set fp [open "csls.messages" r]
      set tt ""
      while {![eof $fp]} {
         set tt "$tt[gets $fp]\n"
      }
      close $fp
      df_mess_dialog "$tt "
   }
   set fp [open "circuit/$cell_name/src_arch" w]
   puts $fp $arch_name
   close $fp

   restore_cwd
   read_infofile
   upd_status_line "compilation of $sls_file_name DONE"
}
