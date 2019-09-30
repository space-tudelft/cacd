
proc newsim {file_name} {
   global comp_name_arr comp_type_arr sig_arr inst_arr

   set fp [open $file_name "r"]

   if {[array exists sig_arr]} {
      unset sig_arr
   }
   if {[array exists comp_name_arr]} {
      unset comp_name_arr
   }
   if {[array exists comp_type_arr]} {
      unset comp_type_arr
   }
   if {[array exists inst_arr]} {
      unset inst_arr
   }
   set comp_name ""
   array set sig_arr {OPEN -1}
   array set comp_name_arr ""
   array set comp_type_arr ""
   array set inst_arr ""
   while {![eof $fp]} {
      set txt [gets $fp]
      if {$txt == ""} {
         continue
      }
      if {[lindex $txt 0] == "entity"} {
         set where "IN_ENTITY"
      }\
      elseif {[lindex $txt 0] == "architecture"} {
         set where "IN_ARCHITECTURE"
      }\
      elseif {[lindex $txt 0] == "component"} {
         set comp_name [lindex $txt 1]
         set where "IN_COMPONENT"
      }\
      elseif {[lindex $txt 0] == "begin"} {
         set where "IN_ARCHITECTURE"
      }\
       elseif {[lindex $txt 0] == "configuration"} {
         set where "IN_CONFIGURATION"
      }\
     elseif {[lindex $txt 0] == "signal"} {
         set sig_name [string trimright [lindex $txt 1]  ":"]
         if {$sig_name == "vss"} {
            set sig_arr($sig_name) 0
         }\
          else {
            set sig_arr($sig_name) -1
         }
         set where "IN_ARCHITECTURE"
      }\
      elseif {[lindex $txt 0] == "port"} {
         if {$where == "IN_ENTITY"} {
            set sig_name [string trim [lindex $txt 1]  ":("]
            set sig_arr($sig_name) -1
         }\
         elseif {$where == "IN_COMPONENT"} {
            set comp_name_arr($comp_name) [string trim [lindex $txt 1]  ":("]
            set comp_type_arr($comp_name) [lindex $txt 2]
         }
      }\
      elseif {[string first ":" [lindex $txt 0]] >= 0} {
         if {$where == "IN_ENTITY"} {
            set sig_name [string trimright [lindex $txt 0]  ":"]
            set sig_arr($sig_name) -1
         }\
         elseif {$where == "IN_ARCHITECTURE"} {
            set inst_name [string trimright [lindex $txt 0]  ":"]
            set len [llength $txt]
            for {set i [expr 4]} {$i < $len} {incr i} {
               lappend inst_arr($inst_name)\
                          [string trim [lindex $txt $i] ",();"]
            }
         }\
         elseif {$where == "IN_COMPONENT"} {
            lappend comp_name_arr($comp_name)\
                    [string trimright [lindex $txt 0]  ":"]
            lappend comp_type_arr($comp_name) [lindex $txt 1]
         }
      }

   }
   close $fp
}

proc update_signal {name value} {
   global sig_arr comp_name_arr comp_type_arr inst_arr cir_name

   set sig_arr($name) $value
   set instances [array names inst_arr]
   foreach i $instances {
      set idx [lsearch $inst_arr($i) $name]
      if {$idx >= 0} {
         set s_idx [expr [string last "_" $i] -1]
         set comp_name [string range $i 0 $s_idx]
         set t_name [lindex $comp_name_arr($comp_name) $idx]
         switch -- $value {
            -1 {.cv itemconfigure $i.$t_name -fill wheat3}
             0 {.cv itemconfigure $i.$t_name -fill blue}
             1 {.cv itemconfigure $i.$t_name -fill red}
         }
      }
   }
   set inst_conn [find_con_inst $name]
   foreach i $inst_conn {
      set s_idx [expr [string last "_" $i] -1]
      set comp_name [string range $i 0 $s_idx]
      set val_str ""
      for {set j [expr 0]}\
              {$j < [llength $comp_name_arr($comp_name)]} {incr j}  {
         if {[lindex $comp_type_arr($comp_name) $j] == "in"} {
            set signl [lindex $inst_arr($i) $j]
            append val_str $sig_arr($signl)
         }
      }
      for {set j [expr 0]}\
              {$j < [llength $comp_name_arr($comp_name)]} {incr j}  {
         if {[lindex $comp_type_arr($comp_name) $j] == "out"} {
            set out_term [lindex $comp_name_arr($comp_name) $j]
            set oldval $sig_arr([lindex $inst_arr($i) $j])
            set newval [update_val ${comp_name}_$out_term $val_str]
            set sig_arr($out_term) $newval
            switch -- $newval {
               -1 {.cv itemconfigure $i.$out_term -fill wheat3}
                0 {.cv itemconfigure $i.$out_term -fill blue}
                1 {.cv itemconfigure $i.$out_term -fill red}
            }
            if {$newval != $oldval} {
               set newname [lindex $inst_arr($i) $j]
               set outterm [array get sig_arr $newname]
               if {$outterm != ""} {
                  set tag $cir_name.[lindex $outterm 0]
                  switch -- $newval {
                     -1 {.cv itemconfigure $tag -fill wheat3}
                      0 {.cv itemconfigure $tag -fill blue}
                      1 {.cv itemconfigure $tag -fill red}
                  }
               }
               if {$newname != "OPEN"} {
                  update_signal $newname $newval
               }
            }
         }
      }

   }
}

proc find_con_inst {sig_name} {
   global inst_arr

   set ret_list ""
   set instances [array names inst_arr]
   foreach i $instances {
      if {[lsearch $inst_arr($i) $sig_name] >= 0} {
         lappend ret_list $i
      }
   }
   return $ret_list
}

proc update_val {name val} {
  global na210_Y no210_Y iv110_Y ex210_Y and2_Y fadder_CO fadder_S

   if {[string first "-" $val] >= 0} {
      return -1
   }
   switch -- $name {
      na210_Y    {return $na210_Y($val)}
      no210_Y    {return $no210_Y($val)}
      ex210_Y    {return $ex210_Y($val)}
      iv110_Y    {return $iv110_Y($val)}
      and2_Y     {return $and2_Y($val)}
      fadder_CO  {return $fadder_CO($val)}
      fadder_S   {return $fadder_S($val)}
   }
}
