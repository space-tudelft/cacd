
proc cir2vhd {cir_name} {
   set VSS_FOUND "False"
   set OP_COMPONENTS "na210 no210 ex210 iv110"
   set f_cir [open $cir_name.cir "r"]
   set f_vhd [open $cir_name.vhd "w"]
   set f_def [open components.def "r"]
   while {![eof $f_def]} {
      set txt [gets $f_def]
      set tmp ""
      if {[lindex $txt 0] == "ports"} {
         for {set i [expr 2]} {$i < [llength $txt]} {incr i} {
            lappend tmp [lindex $txt $i]
         }
         set term_arr([lindex $txt 1]) $tmp
      }\
      elseif {[lindex $txt 0] == "types"} {
         for {set i [expr 2]} {$i < [llength $txt]} {incr i} {
            lappend tmp [lindex $txt $i]
         }
         set type_arr([lindex $txt 1]) $tmp
      }
   }
   close $f_def
   set np [expr -1]
   set ni [expr -1]
   set nc [expr -1]
   while {![eof $f_cir]} {
      set txt [gets $f_cir]
      if {[lindex $txt 0] == "T"} {
         incr np
         set p_arr($np) "[lindex $txt 1] [lindex $txt 2]"
         set ref_arr(${cir_name}.[lindex $txt 2]) "OPEN"
      }\
      elseif {[lindex $txt 0] == "I"} {
         if {[llength $txt] > 1} {
            incr ni
            set i_arr($ni) [lindex $txt 1]
         }
      }\
      elseif {[lindex $txt 0] == "C"} {
         if {[llength $txt] > 1} {
            incr nc
            set c_arr($nc) [lindex $txt 1]
         }
      }
   }
   close $f_cir

# set the CellsLib library
   puts $f_vhd "library IEEE;"
   puts $f_vhd "use IEEE.std_logic_1164.ALL;"
   puts $f_vhd "library CellsLib;"
   puts $f_vhd "use CellsLib.all;\n"

# make the entity
   puts $f_vhd "entity $cir_name is"
   puts $f_vhd "   port ([lindex $p_arr(0) 1]: [lindex $p_arr(0) 0] std_logic;"
   for {set i [expr 1]} {$i < $np} {incr i} {
      puts $f_vhd "         [lindex $p_arr($i) 1]: [lindex $p_arr($i) 0] std_logic;"
   }
   puts $f_vhd "         [lindex $p_arr($np) 1]: [lindex $p_arr($np) 0] std_logic);"
   puts $f_vhd "end $cir_name;\n"

# start the architecture generation
   puts $f_vhd "architecture behaviour of $cir_name is"
# make the components
   set cmp_list ""
   for {set i [expr 0]} {$i <= $ni} {incr i} {
      set cmp_name [lindex [split $i_arr($i) "_"] 0]
      if {([lsearch $cmp_list $cmp_name] == -1) && ($cmp_name != "vss")} {
         lappend cmp_list $cmp_name
         puts $f_vhd "   component $cmp_name"
         puts -nonewline $f_vhd "      port ([lindex $term_arr($cmp_name) 0]:\
                                             [lindex $type_arr($cmp_name) 0] std_logic"
         for {set ii [expr 1]} {$ii < [llength $term_arr($cmp_name)]} {incr ii} {
            puts -nonewline $f_vhd ";\n            [lindex $term_arr($cmp_name) $ii]:\
                                    [lindex $type_arr($cmp_name) $ii] std_logic"
         }
         puts $f_vhd ");"
         puts $f_vhd "   end component;"
      }
      if {$cmp_name == "vss"} {
         set ref_arr($i_arr($i).vss)  "OPEN"
      }\
      else {
         for {set ii [expr 0]} {$ii < [llength $term_arr($cmp_name)]} {incr ii} {
            set ref_arr($i_arr($i).[lindex $term_arr($cmp_name) $ii]) "OPEN"
         }
      }
   }

# generate the signals
   set nr [expr -1]
   set ns [expr -1]
   for {set i [expr 0]} {$i <= $nc} {incr i} {
      set tmpcon [split $c_arr($i) ">"]
      set t1 [lindex $tmpcon 0]
      set t2 [lindex $tmpcon 1]
      if {$ref_arr($t1) == "OPEN"} {
         set tmp_con_list "$t1 $t2"
         set prev_len [expr 2]
         set len [expr -1]
         while {$len != $prev_len} {
            set prev_len $len
            for {set ii [expr $i+1]} {$ii <= $nc} {incr ii} {
               set tmpcon [split $c_arr($ii) ">"]
               set t1 [lindex $tmpcon 0]
               set t2 [lindex $tmpcon 1]
               set in1 [lsearch $tmp_con_list $t1]
               set in2 [lsearch $tmp_con_list $t2]
               if {($in1 < 0) && ($in2 >= 0)} {
                  lappend  tmp_con_list $t1
               }\
               elseif {($in1 >= 0) && ($in2 < 0)} {
                  lappend  tmp_con_list $t2
               }
            }
            set len [llength $tmp_con_list]
         }
         if {[lsearch $tmp_con_list vss*] >= 0} {
            set sig_name "vss"
            set VSS_FOUND "True"
         }\
         else {
            set sr [lsearch $tmp_con_list $cir_name.*]
            if {$sr == -1} {
               incr ns
               set sig_name is$ns
               set s_arr($ns) $sig_name
            }\
            else {
               set sig_name [lindex [split [lindex $tmp_con_list $sr] .] 1]
            }
         }
         for {set il [expr 0]} {$il < $len} {incr il} {
            set ref_arr([lindex  $tmp_con_list $il]) $sig_name
         }
      }
   }
   if {$VSS_FOUND == "True"} {
      puts $f_vhd "   signal vss: std_logic;"
   }
   for {set j [expr 0]} {$j <= $ns} {incr j} {
      puts $f_vhd "   signal is$j: std_logic;"
   }
   puts $f_vhd "begin"
   if {$VSS_FOUND == "True"} {
      puts $f_vhd "   vss <= '0';"
   }


# generate the port map statements
   for {set j [expr 0]} {$j <= $ni} {incr j} {
      set cmp_name [lindex [split $i_arr($j) "_"] 0]
      if {$cmp_name == "vss"} {
         continue
      }
      set terms $term_arr($cmp_name)
      set term1 [lindex $terms 0]
      set sgnl $ref_arr($i_arr($j).$term1)
      puts -nonewline $f_vhd "   $i_arr($j): $cmp_name port map ($sgnl"
      for {set jj [expr 1]} {$jj < [llength $terms]} {incr jj} {
         set sgnl $ref_arr($i_arr($j).[lindex $terms $jj])
         puts -nonewline $f_vhd ", $sgnl"
      }
      puts $f_vhd ");"
   }
   puts $f_vhd "end behaviour;\n"

# generate the configuration
   puts $f_vhd "configuration ${cir_name}_cfg of ${cir_name} is"
   puts $f_vhd "   for behaviour"
   for {set i [expr 0]} {$i < [llength $cmp_list]} {incr i} {
      set cmp [lindex $cmp_list $i]
      if {$cmp == "vss"} {
         continue
      }\
      elseif {[lsearch $OP_COMPONENTS $cmp] >= 0} {
         puts $f_vhd "      for all: $cmp use entity CellsLib.${cmp}(dataflow);"
         puts $f_vhd "      end for;"
      }\
      else {
         puts $f_vhd "      for all: $cmp use configuration work.${cmp}_cfg;"
         puts $f_vhd "      end for;"
      }
   }
   puts $f_vhd "   end for;"
   puts $f_vhd "end ${cir_name}_cfg;"
   close $f_vhd
}
