
proc mk_template {} {
#-----------------------------------------------------------------------------#
# procedure to generate the window in which an entity template can be made    #
#-----------------------------------------------------------------------------#
   global Fnt topmostWindow

   toplevel  .templ
   frame     .templ.efr      -relief raised -bd 2 -bg wheat
   frame     .templ.pfr      -relief raised -bd 2 -bg wheat
   frame     .templ.cfr      -relief raised -bd 2 -bg wheat3
   label     .templ.efr.lbl  -text "entity:" -font $Fnt -bg wheat
   entry     .templ.efr.en   -width 24 -font $Fnt -bg wheat
   label     .templ.pfr.lbl1 -text "ports:" -font $Fnt -anchor w -bg wheat
   label     .templ.pfr.lbl2 -text "name            mode  width" -font $Fnt -anchor w -bg wheat
   text      .templ.pfr.txt  -width 32 -height 12 -font $Fnt -bg wheat -yscrollcommand ".templ.pfr.sb set"
   scrollbar .templ.pfr.sb   -orient vertical -command ".templ.pfr.txt yview" -bg wheat
   button    .templ.cfr.ok   -text "OK" -width 6 -font $Fnt -bg wheat3 -command doit
   button    .templ.cfr.cnl  -text "Cancel"      -font $Fnt -bg wheat3 -command {destroy .templ}

   pack .templ.efr      -side top    -fill x
   pack .templ.cfr      -side bottom -fill x
   pack .templ.pfr      -side top    -fill both -expand 1
   pack .templ.efr.lbl  -side left
   pack .templ.efr.en   -side left   -pady 10
   pack .templ.pfr.lbl1 -side top    -fill x
   pack .templ.pfr.lbl2 -side top    -fill x
   pack .templ.pfr.txt  -side left   -fill both -expand 1
   pack .templ.pfr.sb   -side right  -fill y
   pack .templ.cfr.ok   -side left   -padx 10 -pady 3
   pack .templ.cfr.cnl  -side right  -padx 10 -pady 3
   update

   wm title .templ "new vhdl_entity"
   set txtwdth [winfo width .templ.pfr.txt]
   set tab1 [expr $txtwdth*16/32]
   set tab2 [expr $txtwdth*23/32]
   .templ.pfr.txt configure -tabs "$tab1 $tab2"
   focus .templ.efr.en

   wm attributes .templ -topmost 1; set topmostWindow .templ
   grab set .templ
   tkwait window .templ
}

proc check_port_name {port_str} {
#-----------------------------------------------------------------------------#
# procedure to check the name of a port is not too long                       #
#-----------------------------------------------------------------------------#
   if {[regexp "^\[0-9\]*\$" [lindex $port_str 2]] == 0} {
      df_mess_dialog "ERROR:\n\
	port [lindex $port_str 0] has wrong range\n (not all digits)"
      return 0
   }
   if {[string length [lindex $port_str 0]] > 14} {
      df_mess_dialog "ERROR:\n\
	port_name [lindex $port_str 0] too long:\n (more then 14 chars)"
      return 0
   }
   return 1
}

proc doit {} {
#-----------------------------------------------------------------------------#
# procedure to make a new template for a vhdl model                           #
#-----------------------------------------------------------------------------#
   set nm [.templ.efr.en get]
   set ent_name [string tolower [string trim $nm]]
   if {$ent_name == ""} {
      df_mess_dialog "INSTRUCTION:\n\
	There is no entity_name given:\n\
	Specify one in the 'entity' entry and try again"
      return
   }
   if {![string match \[a-z\]* $ent_name]} {
      df_mess_dialog "ERROR:\n Entity names must start in lower_case"
      return
   }
   if {![string is alnum [string map {_ u} $ent_name]]} {
      df_mess_dialog "ERROR:\n Entity_name '$ent_name' contains incorrect char(s)"
      return
   }
   if {[string length $ent_name] > 14} {
      df_mess_dialog "ERROR:\n Entity_name $ent_name too long:\n (more then 14 chars)"
      return
   }
       if {[get_parr_idx $ent_name] >= 0} { set item "package" }\
   elseif {[get_earr_idx $ent_name] >= 0} { set item "entity" }\
   else { set item "" }
   if {$item != ""} {
      df_mess_dialog "INSTRUCTION:\n\
	The $item '$ent_name' already exists:\n\
	Specify another entity_name and try again"
      return
   }
   set fname "VHDL/$ent_name.vhd"
   if {[file exists $fname]} {
      df_mess_dialog "INSTRUCTION:\n\
	The file '$fname' already exists:\n\
	Specify another entity_name and try again"
      return
   }
   set maxlen 0
   set nl [lindex [split [.templ.pfr.txt index end] .] 0]
   for {set i 1} {$i <= $nl} {incr i} {
      set port [.templ.pfr.txt get $i.0 $i.end]
      if {$port != ""} {
         if {[string length [lindex $port 0]] > $maxlen} {
            set maxlen [string length [lindex $port 0]]
         }
	 if {[check_port_name $port] == 0} { return }
	 lappend ports [.templ.pfr.txt get $i.0 $i.end]
      }
   }
   set wdw [edit_wdw $fname 1 $ent_name]
   $wdw.txt delete 1.0 end
   $wdw.txt insert end "library IEEE;\n"
   $wdw.txt insert end "use IEEE.std_logic_1164.ALL;\n\n"
   $wdw.txt insert end "entity $ent_name is\n"
   set type "in"
   if {[.templ.pfr.txt get 1.0 end] != "\n"} {
      set first 1
      $wdw.txt insert end "   port("
      for {set i 0} {$i < $nl} {incr i} {
         if {[lindex $ports $i] == ""} { continue }
         set tmp [lindex [lindex $ports $i] 0]
         set rng [expr [lindex [lindex $ports $i] 2] -1]
         set newtype   [lindex [lindex $ports $i] 1]
         if {$newtype != "" && $newtype != "-"} { set type $newtype }
         if {$first} {
            set first 0
         } else {
            $wdw.txt insert end ";\n        "
         }
	 if {$rng <= 0} {
	    $wdw.txt insert end "[format "%-*s : %-3s" $maxlen $tmp $type] std_logic"
	 } else {
	    $wdw.txt insert end "[format "%-*s : %-3s" $maxlen $tmp $type] std_logic_vector($rng downto 0)"
	 }
      }
      $wdw.txt insert end ");\n"
   }
   $wdw.txt insert end "end $ent_name;\n"
   $wdw.txt highlight 1.0 end
   destroy .templ
}
