
proc export_project {} {
   global DbName MyWd CompLibPath

   upd_status_line "Creating project archive file (it may take a while) ..."

   restore_cwd

   file delete -force proj_archive
   file mkdir proj_archive
   file mkdir proj_archive/VHDL
   file mkdir proj_archive/SLS
   file mkdir proj_archive/LDM

   set tmp_file_list ""
   set fp [open "work/_info" "r"]
   while {![eof $fp]} {
      set txt [gets $fp]
      switch [string index $txt 0] {
        F { set f [string range $txt 1 end]
	    if {[lsearch $tmp_file_list $f] < 0} { lappend tmp_file_list $f }
	  }
        Z { set i [string first " " $txt]
	    set c [string index $txt $i+1]
	    if {$c == "F"} {
	       set f [string range $txt $i+2 end]
	       if {[lsearch $tmp_file_list $f] < 0} { lappend tmp_file_list $f }
	    } elseif {$c == "!"} {
	       if {[string range $txt $i+2 $i+5] != "s107"} continue
	       set f [string range $txt $i+7 end-1]
	       if {[lsearch $tmp_file_list $f] < 0} { lappend tmp_file_list $f }
	    }
	  }
      }
   }
   close $fp

   foreach f $tmp_file_list {
     if {[file exist $f]} {
	file copy $f "proj_archive/VHDL"
     } else {
	df_mess_dialog "Error in project, file '$f' does not exist"
	export_project_cleanup 1
	return
     }
   }

if {[file isdir $DbName]} {
   goto_dbdir
   set fpipe [open "|dblist -c -h" "r"]
   set tmp_cell_list ""
   while {![eof $fpipe]} {
      set tmptxt [gets $fpipe]
      set newtxt [join $tmptxt " "]
      set token_list [split $newtxt " \t()-"]
      if {[llength $token_list] > 0 && [lindex $token_list end-1] != "imported"} {
          lappend tmp_cell_list "[lindex $token_list 3]"
      }
   }
   close $fpipe
   set cell_list ""
   for {set i [expr [llength $tmp_cell_list]-1]} {$i >= 0} {incr i -1} {
      if {[lsearch $cell_list [lindex $tmp_cell_list $i]] < 0} {
          lappend cell_list [lindex $tmp_cell_list $i]
      }
   }
   foreach i $cell_list {
      if {![file isdir circuit/$i]} continue
      set fsls [open $MyWd/proj_archive/SLS/$i.sls "w"]
      set fpipe [open "|xsls $i" "r"]
      while {![eof $fpipe]} {
         set tmptxt [gets $fpipe]
         # In extracted sls descriptions, instance names (between {}) may have a '.',
         # which is a syntax error for csls, so we replace it by a '_'
         if {([string match *{*.*}* $tmptxt])} {
            puts $fsls [regsub -all "\\." $tmptxt "_"]
         } else {
            puts $fsls $tmptxt
         }
      }
      close $fpipe
      close $fsls
      if {[file exists circuit/$i/src_arch]} {
         file mkdir $MyWd/proj_archive/circuit/$i
         file copy circuit/$i/src_arch $MyWd/proj_archive/circuit/$i/src_arch
      }
   }
   restore_cwd

   goto_dbdir
   set fpipe [open "|dblist -l -h" "r"]
   set tmp_cell_list ""
   while {![eof $fpipe]} {
      set tmptxt [gets $fpipe]
      set newtxt [join $tmptxt " "]
      set token_list [split $newtxt " \t()-"]
      if {[llength $token_list] > 0 && [lindex $token_list end-1] != "imported"} {
          lappend tmp_cell_list "[lindex $token_list 3]"
      }
   }
   close $fpipe
   set cell_list ""
   for {set i [expr [llength $tmp_cell_list]-1]} {$i >= 0} {incr i -1} {
      if {[lsearch $cell_list [lindex $tmp_cell_list $i]] < 0} {
          lappend cell_list [lindex $tmp_cell_list $i]
      }
   }
   file delete xldm.messages
   foreach i $cell_list {
      if {![file isdir layout/$i]} continue
      if {[file exists $i.ldm]} {file delete $i.ldm}
      if {[catch {exec xldm -r $i >>& xldm.messages} err_str]} {
         df_mess_dialog "Error when executing xldm -r $i: $err_str"
         export_project_cleanup 1
         return
      }
      file rename $i.ldm $MyWd/proj_archive/LDM/$i.ldm
   }
   restore_cwd
}

   set tail [file tail $MyWd]
   if {[catch {exec tar cf $MyWd/$tail.pa.tar proj_archive} err_str]} {
      df_mess_dialog "Error when executing tar cf $MyWd/$tail.pa.tar proj_archive: $err_str"
      export_project_cleanup 1
      return
   }

   set file_list [glob -nocomplain components/*.cmp]
   foreach f $file_list {
     if {[file exist $CompLibPath/[file tail $f]]} continue
     if {[catch {exec tar rf $MyWd/$tail.pa.tar $f} err_str]} {
	df_mess_dialog "Error when executing tar rf $MyWd/$tail.pa.tar $f: $err_str"
	export_project_cleanup 1
	return
     }
   }
   set file_list [glob -nocomplain circuits/* ADB/*.ddc .*.term *.cmd *.ref *.lst]
   foreach f $file_list {
     if {[catch {exec tar rf $MyWd/$tail.pa.tar $f} err_str]} {
	df_mess_dialog "Error when executing tar rf $MyWd/$tail.pa.tar $f: $err_str"
	export_project_cleanup 1
	return
     }
   }

   export_project_cleanup 0

   upd_status_line "Created file $MyWd/$tail.pa.tar"
}

proc export_project_cleanup {error} {
   restore_cwd
   file delete -force proj_archive
   if {$error} {
      upd_status_line "No successful export of project archive"
   }
}

proc import_project {} {
   global DbName MyWd posXY

   set types {
      {{TAR Files} {.pa.tar .PA.TAR}}
      {{All Files}        *         }
   }
   set file_name [tk_getOpenFile -initialdir $MyWd -filetypes $types -title "Open .pa.tar file"]
   if {$file_name == ""} {
      set message "No tar file specified"
      return
   }

   upd_status_line "Importing project data from archive file (it may take a while) ... "

   restore_cwd

   if {[catch {exec tar xf $file_name} err_str]} {
      df_mess_dialog "Error when executing tar xf $file_name: $err_str"
      import_project_cleanup 1
      return
   }
   if {![file isdir proj_archive]} {
      df_mess_dialog "Error: missing directory 'proj_archive'"
      import_project_cleanup 1
      return
   }

if {[file isdir $DbName]} {
   set file_list [glob -nocomplain $MyWd/proj_archive/SLS/*.sls]
   goto_dbdir
   file rename -force .cslsrc .cslsrc.save
   file delete csls.messages
   set frc [open ".cslsrc" "w"]
   puts $frc "EXTERN_OBLIGATORY_OFF"
   puts $frc "FORBID_FIRST_CAPITAL_OFF"
   close $frc
   foreach f $file_list {
      if {[catch {exec csls $f >>& csls.messages} err_str]} {
         df_large_mess_dialog "Error when executing csls $f:\n$err_str"
         file rename -force .cslsrc.save .cslsrc
         import_project_cleanup 1
         return
      }
   }
   file rename -force .cslsrc.save .cslsrc

   set file_list [glob -nocomplain $MyWd/proj_archive/circuit/*/src_arch]
   foreach f $file_list {
      file rename $f circuit/[file tail [file dirname $f]]
   }

   file delete cldm.messages
   set file_list [glob -nocomplain $MyWd/proj_archive/LDM/*.ldm]
   foreach f $file_list {
      if {[catch {exec cldm -f $f >>& cldm.messages} err_str]} {
         df_large_mess_dialog "Error when executing cldm -f $f:\n$err_str"
         import_project_cleanup 1
         return
      }
   }
   restore_cwd
}

   if {![file isdir VHDL]} {
      df_mess_dialog "Error: missing directory 'VHDL'"
      import_project_cleanup 1
      return
   }
   set files ""

   set posXY 0
   foreach f [glob -nocomplain proj_archive/VHDL/*] {
      set new_fn VHDL/[file tail $f]
      if {[file exists $new_fn]} {
	 if {![catch {exec cmp -s $f $new_fn}]} continue
	 set msg "File $new_fn already exist!\n"
	 append msg "Diffs between OLD and NEW are:\n"
	 set fp [open "|diff $new_fn $f"]
	 while {![eof $fp]} {
	    set tt [gets $fp]
	    if {$tt != ""} { append msg "$tt\n" }
	 }
	 catch {close $fp}
         check_use_of_file $new_fn
	 append msg "\nAre you sure? Do you want to overwrite?"
	 if {[df_choise_dialog $msg] == "no"} continue
	 file rename -force $new_fn $new_fn\_
      }
      file rename -force $f $new_fn
      lappend files $new_fn
   }
   unset posXY

   if {$files != ""} { compile_new_files $files }

   import_project_cleanup 0

   upd_status_line "Imported project data from archive file $file_name"
}

proc import_project_cleanup {error} {
   restore_cwd
   file delete -force proj_archive
   if {$error} {
      upd_status_line "No successful import of project data"
   }
}

proc get_e_of_file {fname} {
   global Earr
   for {set i 0} {$i < [array size Earr]} {incr i} {
      if {[lindex $Earr($i) 1] == $fname} { return [lindex $Earr($i) 0] }
   }
   return ""
}

proc get_a_of_file {fname} {
   global Aarr Earr
   for {set i 0} {$i < [array size Aarr]} {incr i} {
      if {[lindex $Aarr($i) 2] == $fname} { return "[lindex $Aarr($i) 0] [lindex $Earr([lindex $Aarr($i) 1]) 0]" }
   }
   return ""
}

proc get_c_of_file {fname} {
   global Carr
   for {set i 0} {$i < [array size Carr]} {incr i} {
      if {[lindex $Carr($i) 3] == $fname} { return [lindex $Carr($i) 0] }
   }
   return ""
}

proc get_p_of_file {fname} {
   global Parr
   for {set i 0} {$i < [array size Parr]} {incr i} {
      if {[lindex $Parr($i) 1] == $fname} { return [lindex $Parr($i) 0] }
   }
   return ""
}

proc find_archs {ename aname} {
   global Aarr Earr

   set i [get_earr_idx $ename]
   set archs ""
   for {set j 2} {$j < [llength $Earr($i)]} {incr j} {
      set a [lindex $Aarr([lindex $Earr($i) $j]) 0]
      if {$a != $aname} { lappend archs $a }
   }
   return $archs
}

proc find_confs {ename aname} {
   global Carr
   set cfgs ""
   for {set i 0} {$i < [array size Carr]} {incr i} {
      if {[lindex $Carr($i) 1] == $ename && [lindex $Carr($i) 2] == $aname} {
         lappend cfgs [lindex $Carr($i) 0]
      }
   }
   return $cfgs
}

proc check_use_of_file {fname} {
   upvar msg msg

   if {[set e [get_e_of_file $fname]] != ""} {
      append msg "\nFile is used by entity $e"
      set a [lindex [get_a_of_file $fname] 0]
      if {$a != ""} { append msg "\nFile is used by architecture $a" }
      set lst [find_archs $e $a]
      set i "archs"
   }\
   elseif {[set a [get_a_of_file $fname]] != ""} {
      set e [lindex $a 1]
      set a [lindex $a 0]
      append msg "\nFile is used by architecture $a of $e"
      set lst [find_confs $e $a]
      set i "confs"
   }\
   elseif {[set c [get_c_of_file $fname]] != ""} {
      append msg "\nFile is used by configuration $c"
      set lst [find_config_uses $c]
      set i "confs"
   }\
   elseif {[set p [get_p_of_file $fname]] != ""} {
      append msg "\nFile is used by package $p"
      set lst [find_pack_uses [get_parr_idx $p]]
      set i "items"
   }\
   else {
      append msg "\nFile is currently not used!"
      set lst ""
   }
   if {$lst != ""}  { append msg "\nAnd following $i need to be recompiled:"
     foreach i $lst { append msg "\n    $i" }
   }
}
