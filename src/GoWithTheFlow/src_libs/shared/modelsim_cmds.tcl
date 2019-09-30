
package require comm
package require snit

proc do_compile_new {type fname} {
   global SimLibName
   upd_status_line "compiling new$type $fname ..."
   set just $type
   return [do_vcom $SimLibName $fname]
}

proc compile_new {} {
   compile_new_files [glob -nocomplain VHDL/*.vhd] 0
}

proc compile_new_files {files {nocheck 1}} {
#-----------------------------------------------------------------------------#
# procedure to compile all not compiled VHDL files to the modeltech library   #
#-----------------------------------------------------------------------------#
   global Carr Parr

   set e_list ""
   set a_list ""
   set c_list ""
   set p_list ""

   foreach i_vhd $files {
     set fp [open $i_vhd]
     set ulist ""
     set nochk $nocheck
     while {![eof $fp]} {
	if {[set tt [gets $fp]] == ""} continue
	set tt [string tolower $tt]
	regsub -all {[;""{}]} $tt " " tt
	set t0 [lindex $tt 0]
	if {$t0 == "entity"} {
	   if {$nochk || [get_earr_idx [lindex $tt 1]] < 0} { lappend e_list $i_vhd; incr nochk }
           continue
	}
	if {$t0 == "architecture"} {
	   set aname [lindex $tt 1]
	   set ename [lindex $tt 3]
	   if {$nochk || ![arch_name_exists $ename $aname]} { lappend a_list $i_vhd; incr nochk }
           continue
	}
	if {$t0 == "configuration"} {
	   set cname [lindex $tt 1]
	   if {$nochk || [get_carr_idx $cname] < 0} {
	      set ulist ""
	      while {![eof $fp]} {
		 if {[set tt [gets $fp]] == ""} continue
		 regsub -all {[;""{}]} $tt " " tt
		 if {[string match -nocase "work.*" [lindex $tt end]]} {
		    lappend ulist [string tolower [string range [lindex $tt end] 5 end]]
		 }
	      }
	      lappend cname $i_vhd
	      lappend cname $ulist
	      lappend c_list $cname
	   }
	   break
	}
	if {$t0 == "package"} {
	   set pname [lindex $tt 1]
	   if {$nochk || [get_parr_idx $pname] < 0} {
	      lappend pname $i_vhd
	      lappend pname $ulist
	      lappend p_list $pname
	   }
	   break
	}
	if {[set i [lsearch $tt "use"]] >= 0} {
	   set tt [split [lindex $tt $i+1] "."]
	   if {[lindex $tt 0] == "work"} { lappend ulist [lindex $tt 1] }
	}
     }
     close $fp
   }

   set ok 0
   set nochk 0
   while {1} {
      set old $ok
      set left ""
      foreach p $p_list {
	 if {!$nochk} {
	    set unresolved 0
	    foreach pkg [lindex $p 2] {
	       if {[lsearch -index 0 $p_list $pkg] >= 0
	        || [get_parr_idx $pkg] < 0} { incr unresolved; break }
	    }
	    if {$unresolved} { lappend left $p; continue }
	 }
	 if {[do_compile_new P [lindex $p 1]] == -1} { if {$ok} read_infofile; return }
	 incr ok
	 #read_infofile needs to be done!
	 #but we only add a new Parr entry for unresolved check!
	 set Parr([array size Parr]) [lindex $p 0]
      }
      if {$left == ""} break
      if {$ok == $old} { incr nochk }
      set p_list $left
   }

   foreach f $e_list {
      if {[do_compile_new E $f] == -1} { if {$ok} read_infofile; return }
      incr ok
   }
   foreach f $a_list {
      if {[do_compile_new A $f] == -1} { if {$ok} read_infofile; return }
      incr ok
   }

   set nochk 0
   while {1} {
      set old $ok
      set left ""
      foreach c $c_list {
	 if {!$nochk} {
	    set unresolved 0
	    foreach cfg [lindex $c 2] {
	       if {[lsearch -index 0 $c_list $cfg] >= 0
		|| [get_carr_idx $cfg] < 0} { incr unresolved; break }
	    }
	    if {$unresolved} { lappend left $c; continue }
	 }
	 if {[do_compile_new C [lindex $c 1]] == -1} { if {$ok} read_infofile; return }
	 incr ok
	 #read_infofile needs to be done!
	 #but we only add a new Carr entry for unresolved check!
	 set Carr([array size Carr]) [lindex $c 0]
      }
      if {$left == ""} break
      if {$ok == $old} { incr nochk }
      set c_list $left
   }

   if {$ok} read_infofile
   upd_status_line "compiling new_files DONE"
}

proc recompile_lib {} {
#-----------------------------------------------------------------------------#
# procedure to recompile the modeltech library                                #
#-----------------------------------------------------------------------------#
   global Aarr Carr Earr SimLibName

   upd_status_line "recompiling library $SimLibName: busy"

   set f $SimLibName/_lib.qdb
   if {[catch {file mtime $f} MT]} {
      df_mess_dialog "ERROR: $MT"
      upd_status_line "recompiling library $SimLibName: FAILED"
      return
   }

   set files ""
   set found 0
   for {set i 0} {$i < [array size Earr]} {incr i} {
      set f [lindex $Earr($i) 1]
      if {[lsearch $files $f] >= 0} continue
      if {[catch {file mtime $f} mt]} { df_mess_dialog "SKIPPING: $f (cannot read)"; continue }
      if {$mt > $MT} { incr found }
      lappend files $f
   }
   for {set i 0} {$i < [array size Aarr]} {incr i} {
      set f [lindex $Aarr($i) 2]
      if {[lsearch $files $f] >= 0} continue
      if {[catch {file mtime $f} mt]} { df_mess_dialog "SKIPPING: $f (cannot read)"; continue }
      if {$mt > $MT} { incr found }
      lappend files $f
   }
   for {set i 0} {$i < [array size Carr]} {incr i} {
      set f [lindex $Carr($i) 3]
      if {[lsearch $files $f] >= 0} continue
      if {[catch {file mtime $f} mt]} { df_mess_dialog "SKIPPING: $f (cannot read)"; continue }
      if {$mt > $MT} { incr found }
      lappend files $f
   }

   if {$found} { compile_new_files $files }

   upd_status_line "recompiling library $SimLibName: DONE"
}

proc recompile {type idx} {
   global Aarr Carr Earr Parr SimLibName

   switch $type {
	P { set fname [lindex $Parr($idx) 1] }
	E { set fname [lindex $Earr($idx) 1] }
	A { set fname [lindex $Aarr($idx) 2] }
	C { set fname [lindex $Carr($idx) 3] }
   }
   set ae 0
   if {$type == "A"} {
      set ae 1
      set sz [array size Earr]
      for {set i 0} {$i < $sz} {incr i} {
	 if {[lindex $Earr($i) 1] == $fname} { incr ae; break }
      }
   }
   if {$type == "E"} {
      set ae 1
      set sz [array size Aarr]
      for {set i 0} {$i < $sz} {incr i} {
	 if {[lindex $Aarr($i) 2] == $fname} { incr ae; break }
      }
   }
## upd_status_line "compiling $type$ae $fname ..."
   do_vcom $SimLibName $fname $ae
}

proc do_vcom {library file_name {ae 0}} {
#-----------------------------------------------------------------------------#
# procedure to compile a file into a library                                  #
#-----------------------------------------------------------------------------#
   upvar just just

   set ie 0
   set ia 0
   set ic 0
   set ip 0
   set E  0

   restore_cwd

   # make sure to use no directory (except VHDL) in file name when compiling
   set file_name VHDL/[file tail $file_name]
   set e_wdw [get_edit_wdw $file_name]

   set fp [open $file_name "r"]
   while {![eof $fp]} {
      if {[set line [gets $fp]] == ""} continue

      regsub -all {[""{}]} $line " " line

      set t0 [string tolower [lindex $line 0]]
      if {$t0 == "entity"} {
	 if {$ie} { set E 1; break }
	 if {$ip || $ia || $ic} { set E 6; break }
	 incr ie
	 set en [string tolower [lindex $line 1]]
      }\
      elseif {$t0 == "architecture"} {
	 if {$ia} { set E 1; break }
	 if {$ip || $ic} { set E 6; break }
	 incr ia
	 set an [string tolower [lindex $line 1]]
	 set t3 [string tolower [lindex $line 3]]
	 if {!$ie} { set en $t3 }\
	 elseif {$t3 != $en} { set E 2; break }
      }\
      elseif {$t0 == "configuration"} {
	 if {$ic} { set E 1; break }
	 if {$ip || ($ie && !$ia)} { set E 6; break }
	 incr ic
	 set t3 [string tolower [lindex $line 3]]
	 if {$ia} {
	    if {$t3 != $en} { set E 2; break }
	    while {[set line [gets $fp]] == "" && ![eof $fp]} {}
	    set t1 [string tolower [lindex $line 1]]
	    if {$t1 != $an} { set E 3; break }
	 }
      }\
      elseif {$t0 == "package"} {
	 if {$ie || $ia || $ic} { set E 6; break }
	 set t1 [string tolower [lindex $line 1]]
	 if {$t1 == "body"} {
	    if {!$ip} { set E 5; break }
	    if {[string tolower [lindex $line 2]] != $pn} { set E 4; break }
	    if {[incr ip] > 2} { set t0 $t1; set E 1; break }
	    continue
	 }
	 if {[incr ip] > 1} { set E 1; break }
	 set pn $t1
      }
   }
   close $fp

   if {$E || ($ie + $ia + $ic + $ip) == 0} {
      switch $E {
	0 { set E "no description of package, entity, architecture or configuration found" }
	1 { set E "only one description of $t0 is allowed" }
	2 { set E "$t0 has incorrect entity name" }
	3 { set E "$t0 has incorrect architecture name" }
	4 { set E "body has incorrect $t0 name" }
	5 { set E "body has no description of $t0" }
	6 { set E "other description found; no description of $t0 allowed" }
      }
      if {$e_wdw == ""} {
         set e_wdw [edit_wdw $file_name 2 ""]
      } else {
         $e_wdw.err.txt delete 1.0 end
         $e_wdw.err.txt configure -fg red
      }
      $e_wdw.err.txt insert end "ERROR in $file_name: $E\n"
      return -1
   }

if {[info exist just]} {
   set fp_comp [open "|vcom -2008 -just $just -lower -explicit -work $library $file_name -quiet -nologo" "r+"]
} else {
   set fp_comp [open "|vcom -2008 -lower -explicit -work $library $file_name -quiet -nologo" "r+"]
}

   set ERR_FND "False"
   if {$e_wdw != ""} {
      $e_wdw.err.txt delete 1.0 end
      $e_wdw.err.txt configure -fg red
   }
   while {![eof $fp_comp]} {
      set tt [gets $fp_comp]
      if {$tt != ""} {
         if {$ERR_FND == "False"} {
            if {$e_wdw == ""} { set e_wdw [edit_wdw $file_name 2 ""] }
            set ERR_FND "True"
         }
         $e_wdw.err.txt insert end "$tt\n"
      }
   }
   catch {close $fp_comp}
   if {$ERR_FND == "False"} {
      if {$e_wdw != ""} {
         $e_wdw.err.txt configure -fg blue
         $e_wdw.err.txt insert end "compilation DONE with no errors"
      }
      if {$ae && ($ie + $ia) != $ae} read_infofile
      upd_status_line "compilation of $file_name DONE"
      return 1
   } else {
      upd_status_line "ERRORS during the compilation of $file_name"
      return -1
   }
}

proc do_vsim {library config} {
#-----------------------------------------------------------------------------#
# procedure to start the modelsim simulator                                   #
#-----------------------------------------------------------------------------#
   global VsimAppName DoPath MyWd

   set vsim_id ""
   if {[file exist "$MyWd/vsim_id"]} {
      set fp [open "$MyWd/vsim_id"]
      set vsim_id [gets $fp]
      close $fp
   }

   set send_poss 1
   if {$vsim_id != ""} {
      set send_poss [catch {comm::comm send $vsim_id ""}]
   }

   if {$send_poss == 0} {
      upd_status_line "Simulator loading $config ..."
      comm::comm send $vsim_id quit -sim
      comm::comm send $vsim_id cd $MyWd
      comm::comm send $vsim_id vsim -gui
      comm::comm send $vsim_id quit -sim
      comm::comm send $vsim_id vsim -lib $library $config
      set do_op_init 1
      #set vsim_procs [comm::comm send $vsim_id info procs]
      #set do_op_init [lsearch $vsim_procs do_op_init]
      if {$do_op_init < 0} {
         comm::comm send $vsim_id do $DoPath/init_simulator.do
      } else {
         comm::comm send $vsim_id do_op_init
      }
      upd_status_line "Simulator continued with $config"
   } else {
      upd_status_line "Simulator starting up with $config ..."
      file delete "$MyWd/vsim_id"
      .menuc entryconfigure [.menuc index Simulate] -state disabled
      exec $VsimAppName -lib $library $config -do $DoPath/init_simulator.do &
      set i 0
      while {[incr i] < 100} {
         after 1000
	 if {[file exist "$MyWd/vsim_id"]} break
      }
      after 1000
      .menuc entryconfigure [.menuc index Simulate] -state normal
      upd_status_line "Simulator started with $config"
   }
}

proc do_vdel {type idx} {
#----------------------------------------------------------------#
# procedure to delete an item type from the vhdl working library #
#----------------------------------------------------------------#
   switch $type {
	A { delete_design A $idx }
	E { delete_design E $idx }
	C { delete_design C $idx }
	P { delete_design P $idx }
   }
}
