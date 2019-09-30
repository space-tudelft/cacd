
proc simulate_ntw {sim_type cell_and_view cmdfile} {
#-----------------------------------------------------------------------------#
# procedure to generate a window in which a ntw_simulation can be carried out #
#-----------------------------------------------------------------------------#
   global DbName Fnt MaxNbrMenuItemsInColumn
   upvar #0 Extraction_kind extraction_kind
   upvar #0 Capacitor_extr capacitor_extr

   set cmds ""
   foreach i_cmd [glob -nocomplain *.cmd] {
      if {$i_cmd != "ac_shell.cmd"} { lappend cmds $i_cmd }
   }
   if {$cmds == ""} {
      df_mess_dialog "ERROR:\n\
         There are no cmd_files present:\n\
         So nothing can be simulated."
      return
   }

   toplevel   .sim_ntw
   frame      .sim_ntw.tfr        -relief raised -bd 2 -bg gold
   menubutton .sim_ntw.tfr.cmdf   -text "" -font $Fnt -bg gold -anchor w\
                                  -relief sunk -menu .sim_ntw.tfr.cmdf.cmdfs
   label .sim_ntw.tfr.lb_cmd      -text "cmd_file:" -font $Fnt -bg gold
   label .sim_ntw.tfr.lb_cell     -text "" -font $Fnt -bg gold -anchor w
   menubutton .sim_ntw.tfr.sim    -text "" -font $Fnt -bg gold -anchor w\
                                  -relief sunk -menu .sim_ntw.tfr.sim.sims
   label .sim_ntw.tfr.lb_sim      -text "simulator:" -font $Fnt -bg gold
   frame      .sim_ntw.lfr        -relief raised -bd 2 -bg wheat
   text       .sim_ntw.txt        -width 80 -height 20 -font $Fnt -bg wheat\
                                  -yscrollcommand ".sim_ntw.sb set"
   scrollbar  .sim_ntw.sb         -command ".sim_ntw.txt yview" -bg wheat
   button     .sim_ntw.lfr.doit   -text "Doit" -font $Fnt -bg wheat3\
                                  -command "do_ntw_sim"
   button     .sim_ntw.lfr.show   -text "ShowResult" -font $Fnt -bg wheat3\
                                  -command { global simWavFileName
					catch {exec simeye -bg white $simWavFileName}
					}
   button     .sim_ntw.lfr.cancel  -text "Cancel" -font $Fnt -bg wheat3\
                                   -command {destroy .sim_ntw}
   frame      .sim_ntw.lfr.efr     -relief raised -bd 2
   label      .sim_ntw.lfr.efr.lbl  -text "extraction" -font $Fnt -bg gold
   radiobutton .sim_ntw.lfr.efr.rb1 -text "auto" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "AUTO" -anchor w
   radiobutton .sim_ntw.lfr.efr.rb2 -text "yes" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "YES" -anchor w
   radiobutton .sim_ntw.lfr.efr.rb3 -text "no" -font $Fnt -bg wheat3\
                                    -variable Extraction_kind -value "NO" -anchor w
   frame      .sim_ntw.lfr.cfr     -relief raised -bd 2
   label      .sim_ntw.lfr.cfr.lbl  -text "capacitors" -font $Fnt -bg gold
   radiobutton .sim_ntw.lfr.cfr.rb1 -text "yes" -font $Fnt -bg wheat3\
                                    -variable Capacitor_extr -value "YES" -anchor w
   radiobutton .sim_ntw.lfr.cfr.rb2 -text "no" -font $Fnt -bg wheat3\
                                    -variable Capacitor_extr -value "NO" -anchor w
   menu        .sim_ntw.tfr.cmdf.cmdfs -bg wheat3 -font $Fnt -tearoff 0

   set cmds [lsort $cmds]
   for {set i 0} {$i < [llength $cmds]} {incr i} {
      set i_cmdf [lindex $cmds $i]
      .sim_ntw.tfr.cmdf.cmdfs add command -label $i_cmdf\
                        -command ".sim_ntw.tfr.cmdf configure -text $i_cmdf"
      if {[expr $i % $MaxNbrMenuItemsInColumn] == 0} {
         .sim_ntw.tfr.cmdf.cmdfs entryconfigure $i_cmdf -columnbreak 1
      }
   }
   if {[lsearch $cmds $cmdfile] >= 0} {
      .sim_ntw.tfr.cmdf configure -text $cmdfile
   }
   .sim_ntw.tfr.lb_cell configure -text $cell_and_view

   menu        .sim_ntw.tfr.sim.sims -bg wheat3 -font $Fnt -tearoff 0
   .sim_ntw.tfr.sim.sims add command -label sls\
                  -command ".sim_ntw.tfr.sim configure -text sls"
   .sim_ntw.tfr.sim.sims add command -label spice\
                  -command ".sim_ntw.tfr.sim configure -text spice"
   .sim_ntw.tfr.sim configure -text $sim_type

   pack .sim_ntw.lfr         -side left   -fill y
   pack .sim_ntw.tfr         -side top    -fill x
   pack .sim_ntw.tfr.lb_cell -side left   -padx 10 -fill x -expand 1
   pack .sim_ntw.tfr.lb_cmd  -side left
   pack .sim_ntw.tfr.cmdf    -side left   -padx 10 -fill x -expand 1
   pack .sim_ntw.tfr.sim     -side right  -padx 10 -fill x -expand 1
   pack .sim_ntw.tfr.lb_sim  -side right
   pack .sim_ntw.sb          -side right  -fill y
   pack .sim_ntw.txt         -side right  -fill both -expand 1
   pack .sim_ntw.lfr.cancel  -side bottom -padx 10 -pady 10 -fill x
   pack .sim_ntw.lfr.show    -side bottom -padx 10 -pady 10 -fill x
   pack .sim_ntw.lfr.doit    -side bottom -padx 10 -pady 10 -fill x
   set view [lindex $cell_and_view 1]
   if {$view == "(layout)"} {
	pack .sim_ntw.lfr.efr     -side top -fill x
	pack .sim_ntw.lfr.efr.lbl -side top -fill x
	pack .sim_ntw.lfr.efr.rb1 -side top -fill x
	pack .sim_ntw.lfr.efr.rb2 -side top -fill x
	pack .sim_ntw.lfr.efr.rb3 -side top -fill x
	pack .sim_ntw.lfr.cfr     -side top -fill x
	pack .sim_ntw.lfr.cfr.lbl -side top -fill x
	pack .sim_ntw.lfr.cfr.rb1 -side top -fill x
	pack .sim_ntw.lfr.cfr.rb2 -side top -fill x
   }

   set extraction_kind "AUTO"
   set capacitor_extr "NO"

   wm title .sim_ntw "simulation window"
}

proc do_ntw_sim {} {
#-----------------------------------------------------------------------------#
# procedure to do a ntw_extraction and simulation                             #
#-----------------------------------------------------------------------------#
   global simWavFileName
   upvar #0 Extraction_kind extraction_kind
   upvar #0 Capacitor_extr capacitor_extr

   .sim_ntw.txt delete 1.0 end

   set cmd_file [.sim_ntw.tfr.cmdf cget -text]
   set sim_type [.sim_ntw.tfr.sim cget -text]
   if {$cmd_file == ""} {
      df_mess_dialog "INSTRUCTION:\n\
          There is no cmd_file specified:\n\
          Specify one by clicking the cmd_file button\n\
          and try again"
      return
   }
   set cell_and_view [.sim_ntw.tfr.lb_cell cget -text]
   set cell [lindex $cell_and_view 0]
   set view [lindex $cell_and_view 1]

   if {$view == "(layout)"} {
      set tmpCell [string toupper [string index $cell 0]]
      append tmpCell [string range $cell 1 end]
   } else {
      set tmpCell $cell
   }

   goto_dbdir

   if {$view == "(layout)" && ($extraction_kind == "YES" || ($extraction_kind == "AUTO" &&
	([file exists layout/$cell/in_gln] == 1 ||
	[file exists layout/$cell/ins_gln] == 1 ||
	[file exists circuit/$tmpCell/net] == 0 ||
	[file mtime layout/$cell/box] > [file mtime circuit/$tmpCell/net])))} {

      .sim_ntw.txt insert end "makeboxl -x $cell\n"
      update
      catch {exec makeboxl -x $cell >& tmp_spc}
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_ntw.txt insert end "[gets $fp]\n"
         .sim_ntw.txt see end
         update
      }
      close $fp

      .sim_ntw.txt insert end "makegln $cell\n"
      catch {exec makegln $cell >& tmp_spc}
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_ntw.txt insert end "[gets $fp]\n"
         .sim_ntw.txt see end
         update
      }
      close $fp

      if {$capacitor_extr == "NO"} {
         .sim_ntw.txt insert end "space $cell\n"
         update
         catch {exec space $cell >& tmp_spc}
      } else {
         .sim_ntw.txt insert end "space -c $cell\n"
         update
         catch {exec space -c $cell >& tmp_spc}
      }
      set fp [open tmp_spc]
      while {![eof $fp]} {
         .sim_ntw.txt insert end "[gets $fp]\n"
         .sim_ntw.txt see end
          update
      }
      close $fp

      .sim_ntw.txt insert end "ghoti $tmpCell\n"
      update
      set fp [open "|ghoti $tmpCell"]
      while {![eof $fp]} {
         .sim_ntw.txt insert end "[gets $fp]\n"
         .sim_ntw.txt see end
         update
      }
      close $fp

      .sim_ntw.txt insert end "removing gln_files\n"
      foreach i_gln [glob -nocomplain "layout/$cell/*_gln"] {
         file delete $i_gln
      }
   }

   if {$sim_type == "sls"} {
      set simulator "sls"
      set simWavFileName [lindex [split $cmd_file .] 0].res
      set tmpWavFileName $tmpCell.res
      set simOutFileName [lindex [split $cmd_file .] 0].out
      set tmpOutFileName $tmpCell.out
   } else {
      set simulator "nspice_op"
      set simWavFileName [lindex [split $cmd_file .] 0].ana
      set tmpWavFileName $tmpCell.ana
   }
   .sim_ntw.txt insert end "$simulator $tmpCell ../$cmd_file\n"
   update
   set res [catch {exec $simulator $tmpCell ../$cmd_file >& sim.messages}]
   if {$res} {
      set fp [open "sim.messages"]
      .sim_ntw.txt insert end [read $fp]
      close $fp
      .sim_ntw.txt see end
      restore_cwd
      return
   }

   if {$sim_type == "sls"} {
      file rename -force $tmpOutFileName ../$simOutFileName
   }
   file rename -force $tmpWavFileName ../$simWavFileName
   restore_cwd
   .sim_ntw.txt insert end "simulation completed\n"
   .sim_ntw.txt insert end "results written on file: $simWavFileName\n"
   .sim_ntw.txt see end
}
