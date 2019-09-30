
proc get_carr_idx {name} {
#-----------------------------------------------------------------------------#
# procedure get the index of a name in an c_array                             #
#-----------------------------------------------------------------------------#
   global Carr

   set sz [array size Carr]
   for {set i 0} {$i < $sz} {incr i} {
      if {[lindex $Carr($i) 0] == $name} { return $i }
   }
   return -1
}

proc get_earr_idx {name} {
#-----------------------------------------------------------------------------#
# procedure get the index of a name in an e_array                             #
#-----------------------------------------------------------------------------#
   global Earr

   set sz [array size Earr]
   for {set i 0} {$i < $sz} {incr i} {
      if {[lindex $Earr($i) 0] == $name} { return $i }
   }
   return -1
}

proc get_aarr_idx {xe name} {
#-----------------------------------------------------------------------------#
# procedure get the index of a name in a_array for entity ie                  #
#-----------------------------------------------------------------------------#
   global Earr Aarr

   for {set j 2} {$j < [llength $Earr($xe)]} {incr j} {
      set i [lindex $Earr($xe) $j]
      if {[lindex $Aarr($i) 0] == $name} { return $i }
   }
   return -1
}

proc get_parr_idx {name} {
#-----------------------------------------------------------------------------#
# procedure get the index of a name in a package array                        #
#-----------------------------------------------------------------------------#
   global Parr

   set sz [array size Parr]
   for {set i 0} {$i < $sz} {incr i} {
      if {[lindex $Parr($i) 0] == $name} { return $i }
   }
   return -1
}

proc find_config_uses {conf_name} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the configurations of which the given           #
# configuration is part of.                                                   #
#-----------------------------------------------------------------------------#
   global Carr

   set config_uses ""
   for {set c 0} {$c < [array size Carr]} {incr c} {
      if {[lsearch [lindex $Carr($c) 4] $conf_name] >= 0} {
         lappend config_uses [lindex $Carr($c) 0]
      }
   }
   return $config_uses
}

proc find_entity_uses {ent_name} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the configurations of which the given           #
# entity is part of via its configurations.                                   #
#-----------------------------------------------------------------------------#
   global Carr

   set entity_uses ""
   for {set c 0} {$c < [array size Carr]} {incr c} {
      if {[lindex $Carr($c) 1] == $ent_name} {
         eval lappend entity_uses [find_config_uses [lindex $Carr($c) 0]]
      }
   }
   return $entity_uses
}

proc find_arch_uses {ent_name arch_name} {
#-----------------------------------------------------------------------------#
# procedure to make a list of the configurations of which the given           #
# entity/architecture is part of via its configuration.                       #
#-----------------------------------------------------------------------------#
   global Carr

   set arch_uses ""
   for {set c 0} {$c < [array size Carr]} {incr c} {
      if {([lindex $Carr($c) 1] == $ent_name) &&\
          ([lindex $Carr($c) 2] == $arch_name)} {
         eval lappend arch_uses [find_config_uses [lindex $Carr($c) 0]]
      }
   }
   return $arch_uses
}

proc find_arch_configs {ent_name arch_name} {
#-----------------------------------------------------------------------------#
# it returns a list of configurations of the entity/architecture pair.        #
#-----------------------------------------------------------------------------#
   global Carr

   set arch_cfgs ""
   for {set c 0} {$c < [array size Carr]} {incr c} {
      if {[lindex $Carr($c) 1] == $ent_name &&\
          [lindex $Carr($c) 2] == $arch_name} {
         lappend arch_cfgs $c
      }
   }
   return $arch_cfgs
}

proc find_pack_uses {p} {
#-----------------------------------------------------------------------------#
# procedure to make a list of items which use the given package with index p. #
#-----------------------------------------------------------------------------#
   global Aarr Carr Earr Parr

   set items_found ""
         for {set i 2} {$i < [llength $Parr($p)]} {incr i} {
            set idx [string range [lindex $Parr($p) $i] 1 end]
            switch [string index [lindex $Parr($p) $i] 0] {
               "A" { set anm [lindex $Aarr($idx) 0]
                     set idx [lindex $Aarr($idx) 1]
                     lappend items_found [lindex $Earr($idx) 0]\($anm\) }
               "C" { lappend items_found [lindex $Carr($idx) 0] }
               "E" { lappend items_found [lindex $Earr($idx) 0] }
               "P" { if {$idx != $p} {lappend items_found [lindex $Parr($idx) 0]} }
               "B" { lappend items_found [lindex $Parr($idx) 0] }
            }
         }
   return $items_found
}

proc get_ename_of_vhdl_file {fname} {
#--------------------------------------------------------------------------------#
# procedure to get the entity of a VHDL file (only entity files and architecture #
# files are searched)                                                            #
#--------------------------------------------------------------------------------#
   global Earr Aarr

   set sz [array size Earr]
   for {set i 0} {$i < $sz} {incr i} {
      if {[lindex $Earr($i) 1] == $fname} {
         return [lindex $Earr($i) 0]
      }
   }
   set sz [array size Aarr]
   for {set i 0} {$i < $sz} {incr i} {
      if {[lindex $Aarr($i) 2] == $fname} {
         return [lindex $Earr([lindex $Aarr($i) 1]) 0]
      }
   }
   return ""
}

proc del_infofile {} {
   global Earr Marr Sarr Aarr Carr Parr Tarr Larr CEarr

   if {[llength [.hierarchy.tree nodes root]] > 0} {
# puts stderr "=[llength [.hierarchy.tree nodes root]]="
      .hierarchy.tree delete [.hierarchy.tree nodes root]
   }

   .cv delete all
   if {[array exists Earr]} { unset Earr }
   if {[array exists Marr]} { unset Marr }
   if {[array exists Sarr]} { unset Sarr }
   if {[array exists Aarr]} { unset Aarr }
   if {[array exists Parr]} { unset Parr }
   if {[array exists Carr]} { unset Carr }
   if {[array exists CEarr]} {unset CEarr}
   if {[array exists Tarr]} { unset Tarr }
   if {[array exists Larr]} { unset Larr }
}

proc read_infofile {} {
#-----------------------------------------------------------------------------#
# procedure to read the _info file of the modelsim database and show the      #
# items in the interface.                                                     #
#-----------------------------------------------------------------------------#
   global Earr Marr Sarr Aarr Carr CEarr Parr Tarr Larr SimLibName Fnt ChosenIndex MyWd DbName
   global lastcolor fnt_scl OPPROGPATH Yentity Ypos

   del_infofile

   set cmpcells ""
   foreach c [glob -nocomplain -directory $MyWd/components -tails *.cmp] {
      set c2 [lindex [split $c "."] 0]
      if {[file isdirectory $OPPROGPATH/sim_libs/CellsLib/$c2] == 0} {
         lappend cmpcells $c2
      }
   }

   set schcells ""
   foreach c [glob -nocomplain -directory $MyWd/circuits -tails *.cir] {
      lappend schcells [lindex [split $c "."] 0]
   }

   set circells ""
   if {$DbName != "" && [file readable $DbName/circuit/celllist]} {
      foreach c [glob -nocomplain $DbName/circuit/*/mc] {
         set cell [lindex [file split $c] end-1]
         if {[string match \[a-z\]* $cell]} { lappend circells $cell }
      }
   }

   set laycells ""
   if {$DbName != "" && [file readable $DbName/layout/celllist]} {
      foreach c [glob -nocomplain $DbName/layout/*/mc] {
         set cell [lindex [file split $c] end-1]
         lappend laycells $cell
      }
   }

   set xe [expr 95*$fnt_scl]
   set xm [expr 224*$fnt_scl]
   set xs [expr 300*$fnt_scl]
   set xp [expr 200*$fnt_scl]
   set xa [expr 190*$fnt_scl]
   set xc [expr 300*$fnt_scl]
   set y0 30
   set hw [expr 80*$fnt_scl]
   set hh [expr 10*$fnt_scl]
   set dy [expr 40*$fnt_scl]

   set y $y0
   set ie -1
   set ia -1
   set ic -1
   set ip -1
   set it -1
   set is -1
   set im -1
   set il -1

   if {[catch {open "$MyWd/work/_info"} fp]} {
      df_mess_dialog $fp
      return
   }

   set last_found "NF"
   while {![eof $fp]} {
      set txt [gets $fp]

      switch [string index $txt 0] {
	Z { set i [string first " " $txt]
	    set n [string range $txt 1 $i-1]
	    set txt [string range $txt $i+1 end]
	    switch [string index $txt 0] {
	      a -
	      e -
	      F { set Z($n) $txt }
	      D { if {[lindex $txt 1] != $SimLibName} continue
		  switch [string index $txt 1] {
		    C -
		    E -
		    P { set Z($n) $txt }
		    default { continue }
		  }
		}
	      ! { if {[lindex $txt 0] != "!s107"} continue
		  set Z($n) F[string trimright [lindex $txt 1] |]
		  continue
		}
	      default { continue }
	    }
	  }
	R { set n [string range $txt 1 end]
	    if {![info exist Z($n)]} continue
	    set txt $Z($n)
	  }
	! { if {[lindex $txt 0] != "!s107"} continue
	    set txt F[string trimright [lindex $txt 1] |]
	  }
      }

      switch [string index $txt 0] {
	E {
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }
	    incr ie
	    set e_name [string range $txt 1 end]
	    set Earr($ie) $e_name
	    set last_found "E"
	    if {![.hierarchy.tree exists $e_name]} {
	       .hierarchy.tree insert end root $e_name -text $e_name -font $Fnt
	       .hierarchy.tree opentree $e_name
	    }
	  }
	A {
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }\
elseif {$last_found != "EF" && $last_found != "AF"} { df_mess_dialog "WARNING: Unexpected A after $last_found" }
	    incr ia
	    set a_name [string range $txt 1 end]
	    set Aarr($ia) $a_name
	    lappend Aarr($ia) $ie
	    lappend Earr($ie) $ia
	    set last_found "A"
	  }
	C {
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }
	    incr ic
	    set c_name [string range $txt 1 end]
	    set Carr($ic) $c_name
	    set CEarr($ic) ""
	    set last_found "C"
	    set e_name ""
	    set a_name ""
	    set tmp_list ""
	  }
	P {
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }
	    set p_name [string range $txt 1 end]
	    set p_curr [get_parr_idx $p_name]
	    if {$p_curr < 0} {
	       incr ip
	       set Parr($ip) $p_name
	       set p_curr $ip
	    }
	    set last_found "P"
	  }
	B {
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }\
elseif {$last_found != "PF"} { df_mess_dialog "WARNING: Unexpected package body after $last_found" }
	    set b_name [string range $txt 1 end]
if {$b_name != "body"} { df_mess_dialog "WARNING: Unexpected package body name: $b_name" }
	    set last_found "B"
	  }
	a { set a_name [string range $txt 1 end] }
	e { set e_name [string range $txt 1 end]
	    if {![.hierarchy.tree exists $e_name]} {
	       .hierarchy.tree insert end root $e_name -text $e_name -font $Fnt
	       .hierarchy.tree opentree $e_name
	    }
	  }
	F { if {[string len $last_found] != 1} {
	       if {$fname != [string range $txt 1 end]} { df_mess_dialog "WARNING: unequal fname for $last_found" }
	       continue
	    }
	    set fname [string range $txt 1 end]
	    switch $last_found {
		P { set Parr($p_curr) [linsert $Parr($p_curr) 1 $fname] }
		B { if {$fname != [lindex $Parr($p_curr) 1]} {
			incr ip
			set Parr($ip) $b_name
			lappend Parr($ip) $fname
			lappend Parr($ip) "P$p_curr"
			lappend Parr($p_curr) "B$ip"
		    }
		  }
		E { lappend Earr($ie) $fname }
		A { lappend Aarr($ia) $fname }
		C {
if {$e_name == ""} { df_mess_dialog "WARNING: e_name NOT found for config: $c_name" }
if {$a_name == ""} { df_mess_dialog "WARNING: a_name NOT found for config: $c_name" }
		    lappend Carr($ic) $e_name
		    lappend Carr($ic) $a_name
		    lappend Carr($ic) $fname
		    lappend Carr($ic) $tmp_list
		  }
	    }
	    append last_found "F"
	  }
	D {
if {[string len $last_found] != 1} { df_mess_dialog "WARNING: Unexpected depend after source file for $last_found" }
	    if {[lindex $txt 1] != $SimLibName} continue
	    switch [string index $txt 1] {
	      C { lappend tmp_list [lindex $txt 3] }
	      E { if {$last_found == "C"} {
		    set de_name [lindex $txt 3]
		    if {$de_name != $e_name} {
		      lappend CEarr($ic) $de_name
if {$e_name == ""} { df_mess_dialog "WARNING: e_name NOT found for depend of config: $c_name" }
		      if {![.hierarchy.tree exists $de_name]} {
			 .hierarchy.tree insert end $e_name $de_name -text $de_name -font $Fnt
			 .hierarchy.tree opentree $de_name
		      } elseif {[.hierarchy.tree parent $de_name] == "root"} {
			 .hierarchy.tree move $e_name $de_name end
		      } elseif {[.hierarchy.tree parent $de_name] != $e_name} {
			if {![.hierarchy.tree exists "$de_name $e_name"]} {
			 .hierarchy.tree insert end $e_name "$de_name $e_name" -text "$de_name ->" -font $Fnt
			 .hierarchy.tree opentree "$de_name $e_name"
			}
		      }
		    }
		  }
		}
	      P { set p_name [lindex $txt 3]
		  set p_idx [get_parr_idx $p_name]
		  if {$p_idx < 0} {
		     incr ip
		     set Parr($ip) $p_name
		     set p_idx $ip
		  }
		  switch $last_found {
			A { lappend Parr($p_idx) "A$ia" }
			E { lappend Parr($p_idx) "E$ie" }
			P { lappend Parr($p_idx) "P$p_curr" }
		  }
		  # C { lappend Parr($p_idx) "C$ic" }
		}
	    }
	  }
      }
   }
if {[string len $last_found] != 2} { df_mess_dialog "WARNING: Source file not found for $last_found" }
   close $fp

   set cfgcells ""
   for {set i 0} {$i <= $ic} {incr i} {
      set e_name [lindex $Carr($i) 1]
      set a_name [lindex $Carr($i) 2]
      if {[set ei [get_earr_idx $e_name]] >= 0} {
         if {[set j [get_aarr_idx $ei $a_name]] >= 0} { lappend Aarr($j) $i } { lappend Xarr($ei) $i }
      } else {
         lappend cfgcells $e_name
      }
   }

 # set fpdebug [open "debug" w]
 # for {set i 0} {$i < [array size Parr]} {incr i} { puts $fpdebug "P$i $Parr($i)" }
 # for {set i 0} {$i < [array size Earr]} {incr i} { puts $fpdebug "E$i $Earr($i)" }
 # for {set i 0} {$i < [array size Aarr]} {incr i} { puts $fpdebug "A$i $Aarr($i)" }
 # for {set i 0} {$i < [array size Carr]} {incr i} { puts $fpdebug "C$i $Carr($i)" }
 # close $fpdebug

   for {set i 0} {$i <= $ip} {incr i} {
      set p_name [lindex $Parr($i) 0]
      .cv create rectangle [expr $xp-$hw] [expr $y-$hh]\
                           [expr $xp+$hw] [expr $y+$hh]\
                           -fill gold3 -tag "P_$i RP_$i"
      .cv create text $xp $y -text $p_name -tag P_$i -font $Fnt
      set yel [expr $y+$hh]
      set y [expr $y+$dy]
   }

   for {set i 0} {$i <= $ie} {incr i} {
      set e_name  [lindex $Earr($i) 0]
      set e_fname [lindex $Earr($i) 1]
      .cv create rectangle [expr $xe-$hw] [expr $y-$hh]\
                           [expr $xe+$hw] [expr $y+$hh]\
                           -fill gold -tag "E_$i RE_$i"
      .cv create text $xe $y -text $e_name -tag E_$i -font $Fnt

      set Yentity($e_name) [expr $y-2*$hh]

      set cirfound 0
      set layfound 0

      if {[file readable "$MyWd/components/$e_name.cmp"]} {
          incr im
          .cv create rectangle [expr $xm-2*$hw/8] [expr $y-$hh]\
                               [expr $xm+2*$hw/8] [expr $y+$hh]\
                              -fill chocolate1 -tag "M_$im RM_$im"
          .cv create text $xm $y -text comp -tag M_$im -font $Fnt
	  set Marr($im) $e_name
      }

      if {[file readable "$MyWd/circuits/$e_name.cir"]} {
          incr is
          .cv create rectangle [expr $xs-3*$hw/8] [expr $y-$hh]\
                               [expr $xs+3*$hw/8] [expr $y+$hh]\
                              -fill mediumaquamarine -tag "S_$is RS_$is"
          .cv create text $xs $y -text schematic -tag S_$is -font $Fnt
	  set Sarr($is) $e_name
      }

      set arch_name ""
      if {$DbName != "" && [file readable "$DbName/circuit/$e_name/src_arch"]} {
	 set fp [open "$DbName/circuit/$e_name/src_arch"]
	 set arch_name [gets $fp]
	 close $fp
      }

      set yel [expr $y+$hh]
      set y [expr $y+$dy]

      for {set ii 2} {$ii < [llength $Earr($i)]} {incr ii} {
         set a_idx [lindex $Earr($i) $ii]
         set a_name [lindex $Aarr($a_idx) 0]
         set a_fname [lindex $Aarr($a_idx) 2]
         .cv create rectangle [expr $xa-3*$hw/4] [expr $y-$hh]\
                              [expr $xa+3*$hw/4] [expr $y+$hh]\
                              -fill gold -tag "A_$a_idx RA_$a_idx"
         .cv create text $xa $y  -text $a_name -tag A_$a_idx -font $Fnt

         if {$a_fname == $e_fname} { set lw 4 } { set lw 1 }
         .cv create line $xe $yel $xe $y [expr $xa-3*$hw/4] $y -width $lw

	 if {$a_name == $arch_name} {
	    incr it
	    .cv create rectangle [expr $xa+8*$hw/8] [expr $y-$hh]\
				[expr $xa+14*$hw/8] [expr $y+$hh]\
                              -fill lavender -tag "T_$it RT_$it"
	    .cv create text [expr $xa+11*$hw/8] $y -text "circuit" -tag "T_$it" -font $Fnt
	    .cv create line [expr $xa+6*$hw/8] $y [expr $xa+8*$hw/8] $y
	    set cirfound 1
	    set Tarr($it) $e_name

            if {[lsearch $laycells $e_name] >= 0} {
               file stat "$DbName/circuit/$e_name/mc" cirstat
               file stat "$DbName/layout/$e_name/mc" laystat
               if {$laystat(mtime) > $cirstat(mtime)} {
                  .cv create line [expr $xa+14*$hw/8] $y [expr $xa+16*$hw/8] $y
               }
               incr il
               .cv create rectangle [expr $xa+16*$hw/8] [expr $y-$hh]\
                                    [expr $xa+22*$hw/8] [expr $y+$hh]\
                                    -fill pink -tag "L_$il RL_$il"
               .cv create text [expr $xa+19*$hw/8] $y -text "layout" -tag L_$il -font $Fnt
               set layfound 1
               set Larr($il) $e_name
            }
         }

         set yal [expr $y+$hh]
         set y [expr $y+$dy]
         for {set iii 3} {$iii < [llength $Aarr($a_idx)]} {incr iii} {
            set c_idx [lindex $Aarr($a_idx) $iii]
            set c_name [lindex $Carr($c_idx) 0]
            set c_fname [lindex $Carr($c_idx) 3]
            .cv create rectangle [expr $xc-$hw] [expr $y-$hh]\
                                 [expr $xc+$hw] [expr $y+$hh]\
                                 -fill gold -tag "C_$c_idx RC_$c_idx"
            .cv create text $xc $y -text $c_name -tag C_$c_idx -font $Fnt
            if {$c_fname == $a_fname} { set lw 4 } { set lw 1 }
            .cv create line $xa $yal $xa $y [expr $xc-$hw] $y -width $lw
            set y [expr $y+$dy]
         }
      }
      if {[info exists Xarr($i)]} {
         foreach c_idx $Xarr($i) {
            set c_name [lindex $Carr($c_idx) 0]
            .cv create rectangle [expr $xc-$hw] [expr $y-$hh]\
                                 [expr $xc+$hw] [expr $y+$hh]\
				 -fill gold -tag "C_$c_idx RC_$c_idx"
				 .cv create text $xc $y -text $c_name -tag C_$c_idx -font $Fnt
            set y [expr $y+$dy]
         }
      }

      set found 0
      if {$cirfound == 0 && [lsearch $circells $e_name] >= 0} {
          incr it
          .cv create rectangle [expr $xa+8*$hw/8] [expr $y-$hh]\
                              [expr $xa+14*$hw/8] [expr $y+$hh]\
                          -fill lavender -tag "T_$it RT_$it"
          .cv create text [expr $xa+11*$hw/8] $y -text "circuit" -tag T_$it -font $Fnt
          set Tarr($it) $e_name
	  incr found
      }

      if {$layfound == 0 && [lsearch $laycells $e_name] >= 0} {
          if {[file readable "$DbName/circuit/$e_name/mc"]
              && [file readable "$DbName/layout/$e_name/mc"]} {
             file stat "$DbName/circuit/$e_name/mc" cirstat
             file stat "$DbName/layout/$e_name/mc" laystat
             if {$laystat(mtime) > $cirstat(mtime)} {
                .cv create line [expr $xa+14*$hw/8] $y [expr $xa+16*$hw/8] $y
             }
          }
          incr il
          .cv create rectangle [expr $xa+16*$hw/8] [expr $y-$hh]\
                               [expr $xa+22*$hw/8] [expr $y+$hh]\
                         -fill pink -tag "L_$il RL_$il"
          .cv create text [expr $xa+19*$hw/8] $y -text "layout" -tag L_$il -font $Fnt
          set Larr($il) $e_name
	  incr found
      }
      if {$found} { set y [expr $y+$dy] }
   }

   set cmp_sch_cir_lay_cells [concat $cmpcells $schcells $circells $laycells $cfgcells]
   set cmp_sch_cir_lay_cells [lsort -unique $cmp_sch_cir_lay_cells]

   for {set i 0} {$i < [llength $cmp_sch_cir_lay_cells]} {incr i} {
      set e_name [lindex $cmp_sch_cir_lay_cells $i]

      # cell_in_working_lib
      if {[get_earr_idx $e_name] >= 0} continue

      # .cv create rectangle [expr $xe-$hw] [expr $y-$hh]\
      #                      [expr $xe+$hw] [expr $y+$hh]\
      #                      -fill gold -tag "EE_$i REE_$i"
      .cv create text $xe $y -text $e_name -tag EE_$i -font $Fnt

      set Yentity($e_name) [expr $y-2*$hh]

      set found 0
      if {[lsearch $cmpcells $e_name] >= 0} {
          incr im
          .cv create rectangle [expr $xm-2*$hw/8] [expr $y-$hh]\
                               [expr $xm+2*$hw/8] [expr $y+$hh]\
                              -fill chocolate1 -tag "M_$im RM_$im"
          .cv create text $xm $y -text "comp" -tag M_$im -font $Fnt
	  set Marr($im) $e_name
	  incr found
      }
      if {[lsearch $schcells $e_name] >= 0} {
          incr is
          .cv create rectangle [expr $xs-3*$hw/8] [expr $y-$hh]\
                               [expr $xs+3*$hw/8] [expr $y+$hh]\
                              -fill mediumaquamarine -tag "S_$is RS_$is"
          .cv create text $xs $y -text "schematic" -tag S_$is -font $Fnt
	  set Sarr($is) $e_name
	  incr found
      }
      if {$found} { set y [expr $y+$dy] }

      set found 0
      if {[lsearch $circells $e_name] >= 0} {
          incr it
          .cv create rectangle [expr $xa+8*$hw/8] [expr $y-$hh]\
                              [expr $xa+14*$hw/8] [expr $y+$hh]\
                          -fill lavender -tag "T_$it RT_$it"
          .cv create text [expr $xa+11*$hw/8] $y -text "circuit" -tag T_$it -font $Fnt
          set Tarr($it) $e_name
	  incr found
      }
      if {[lsearch $laycells $e_name] >= 0} {
          if {[file readable "$DbName/circuit/$e_name/mc"]
              && [file readable "$DbName/layout/$e_name/mc"]} {
             file stat "$DbName/circuit/$e_name/mc" cirstat
             file stat "$DbName/layout/$e_name/mc" laystat
             if {$laystat(mtime) > $cirstat(mtime)} {
                .cv create line [expr $xa+14*$hw/8] $y [expr $xa+16*$hw/8] $y
             }
          }
          incr il
          .cv create rectangle [expr $xa+16*$hw/8] [expr $y-$hh]\
                               [expr $xa+22*$hw/8] [expr $y+$hh]\
                         -fill pink -tag "L_$il RL_$il"
          .cv create text [expr $xa+19*$hw/8] $y -text "layout" -tag L_$il -font $Fnt
          set Larr($il) $e_name
	  incr found
      }
      if {$found} { set y [expr $y+$dy] }

      if {[lsearch $cfgcells $e_name] >= 0} {
	 for {set c_idx 0} {$c_idx <= $ic} {incr c_idx} {
	    if {[lindex $Carr($c_idx) 1] == $e_name} {
	       set c_name [lindex $Carr($c_idx) 0]
	       .cv create rectangle [expr $xc-$hw] [expr $y-$hh]\
				    [expr $xc+$hw] [expr $y+$hh]\
				    -fill gold -tag "C_$c_idx RC_$c_idx"
	       .cv create text $xc $y -text $c_name -tag C_$c_idx -font $Fnt
	       set y [expr $y+$dy]
	    }
	 }
      }
   }

   # scale factor for Yentity
   set Ypos [expr {$y + 60.0}]

   .cv configure -scrollregion "0 0 600 [expr $y+$dy]"
   for {set i 0} {$i <= $ip} {incr i} {
      .cv bind P_$i <ButtonPress-1> {
         global Parr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set p_tag [lindex [.cv gettags current] 0]
         set ii [string range $p_tag 2 end]
         set name [lindex $Parr($ii) 0]
         .cv itemconfigure R$p_tag -fill green
         .cv addtag choise withtag R$p_tag
         set lastcolor "gold3"
         set ChosenIndex $ii
         tk_popup .menup %X %Y 0
      }
   }
   for {set i 0} {$i <= $ie} {incr i} {
      .cv bind E_$i <ButtonPress-1> {
         global Earr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set e_tag [lindex [.cv gettags current] 0]
         set ii [string range $e_tag 2 end]
         set name [lindex $Earr($ii) 0]
         .cv itemconfigure R$e_tag -fill green
         .cv addtag choise withtag R$e_tag
         set lastcolor "gold"
         set ChosenIndex $ii
         tk_popup .menue %X %Y 0
      }
   }
   for {set i 0} {$i <= $it} {incr i} {
      .cv bind T_$i <ButtonPress-1> {
         global Tarr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set t_tag [lindex [.cv gettags current] 0]
         set ii [string range $t_tag 2 end]
         set name [lindex $Tarr($ii) 0]
         .cv itemconfigure R$t_tag -fill green
         .cv addtag choise withtag R$t_tag
         set lastcolor "lavender"
         set ChosenIndex $ii
         tk_popup .menut %X %Y 0
      }
   }
   for {set i 0} {$i <= $is} {incr i} {
      .cv bind S_$i <ButtonPress-1> {
         global Sarr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set s_tag [lindex [.cv gettags current] 0]
         set ii [string range $s_tag 2 end]
         set name [lindex $Sarr($ii) 0]
         .cv itemconfigure R$s_tag -fill green
         .cv addtag choise withtag R$s_tag
         set lastcolor "mediumaquamarine"
         set ChosenIndex $ii
         tk_popup .menus %X %Y 0
      }
   }
   for {set i 0} {$i <= $im} {incr i} {
      .cv bind M_$i <ButtonPress-1> {
         global Marr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set m_tag [lindex [.cv gettags current] 0]
         set ii [string range $m_tag 2 end]
         set name [lindex $Marr($ii) 0]
         .cv itemconfigure R$m_tag -fill green
         .cv addtag choise withtag R$m_tag
         set lastcolor "chocolate1"
         set ChosenIndex $ii
         tk_popup .menum %X %Y 0
      }
   }
   for {set i 0} {$i <= $il} {incr i} {
      .cv bind L_$i <ButtonPress-1> {
         global Larr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set l_tag [lindex [.cv gettags current] 0]
         set ii [string range $l_tag 2 end]
         set name [lindex $Larr($ii) 0]
         .cv itemconfigure R$l_tag -fill green
         .cv addtag choise withtag R$l_tag
         set lastcolor "pink"
         set ChosenIndex $ii
         tk_popup .menul %X %Y 0
      }
   }
   for {set i 0} {$i <= $ia} {incr i} {
      .cv bind A_$i <ButtonPress-1> {
         global Earr Aarr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set a_tag [lindex [.cv gettags current] 0]
         set ii [string range $a_tag 2 end]
         set a_name [lindex $Aarr($ii) 0]
         set e_nbr  [lindex $Aarr($ii) 1]
         set e_name [lindex $Earr($e_nbr) 0]
         .cv itemconfigure R$a_tag -fill green
         .cv addtag choise withtag R$a_tag
         set lastcolor "gold"
         set ChosenIndex $ii
         if {$a_name == "synthesised"} {
             tk_popup .menua2 %X %Y 0
         } elseif {$a_name == "circuit"} {
             tk_popup .menua2 %X %Y 0
         } else {
             tk_popup .menua %X %Y 0
         }
      }
   }
   for {set i 0} {$i <= $ic} {incr i} {
      .cv bind C_$i <ButtonPress-1> {
         global Earr Aarr Carr lastcolor
         .cv itemconfigure choise -fill $lastcolor
         set item [.cv find withtag choise]
         .cv dtag $item choise
         set c_tag [lindex [.cv gettags current] 0]
         set ii [string range $c_tag 2 end]
         set c_name [lindex $Carr($ii) 0]
         set e_name [lindex $Carr($ii) 1]
         set a_name [lindex $Carr($ii) 2]
         .cv itemconfigure R$c_tag -fill green
         .cv addtag choise withtag R$c_tag
         set lastcolor "gold"
         set ChosenIndex $ii
         tk_popup .menuc %X %Y 0
      }
##    .cv bind C_$i <ButtonPress-3> {
##       global Carr
##       set c_tag [lindex [.cv gettags current] 0]
##       set ii [string range $c_tag 2 end]
##       set cfg_name [lindex $Carr($ii) 0]
##    }
   }

   set lastcolor "blue"
}

proc outdate_cir {name} {
   global DbName

   set fn "$DbName/circuit/$name/src_arch"
   if {[file readable $fn]} {
      file delete -force $fn
      read_infofile
   }
}

proc canvas_focus {name} {
   global Yentity Ypos

   set s [lindex $name 0]
   .cv yview moveto [expr $Yentity($s)/$Ypos]
   .hierarchy.tree selection set $s $name
}
