
proc cir2sls {cir_name} {
   set f_cir [open $cir_name.cir "r"]
   set f_sls [open $cir_name.sls "w"]
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
         set ref_arr(${cir_name}.[lindex $txt 2]) " "
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

# make the external networks
   set cmp_list ""
   for {set i [expr 0]} {$i <= $ni} {incr i} {
      set cmp_name [lindex [split $i_arr($i) "_"] 0]
      if {([lsearch $cmp_list $cmp_name] == -1) && ($cmp_name != "vss")} {
         lappend cmp_list $cmp_name
         puts -nonewline $f_sls "extern network $cmp_name (terminal"
         for {set ii [expr 0]} {$ii < [llength $term_arr($cmp_name)]} {incr ii} {
            puts -nonewline $f_sls " [lindex $term_arr($cmp_name) $ii],"
         }
         puts $f_sls " vss, vdd)"
      }
      if {$cmp_name == "vss"} {
         set ref_arr($i_arr($i).vss)  " "
      }\
      else {
         for {set ii [expr 0]} {$ii < [llength $term_arr($cmp_name)]} {incr ii} {
            set ref_arr($i_arr($i).[lindex $term_arr($cmp_name) $ii]) " "
         }
      }
   }

# make the network
   puts -nonewline $f_sls "network $cir_name (terminal"
   for {set i [expr 0]} {$i <= $np} {incr i} {
      puts -nonewline $f_sls " [lindex $p_arr($i) 1],"
   }
   puts $f_sls " vss, vdd)"
   puts $f_sls "\{"

   set nr [expr -1]
   set ns [expr -1]
   for {set i [expr 0]} {$i <= $nc} {incr i} {
      set tmpcon [split $c_arr($i) ">"]
      set t1 [lindex $tmpcon 0]
      set t2 [lindex $tmpcon 1]
      if {$ref_arr($t1) == " "} {
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

# generate the instance statements
   for {set j [expr 0]} {$j <= $ni} {incr j} {
      set cmp_name [lindex [split $i_arr($j) "_"] 0]
      if {$cmp_name == "vss"} {
         continue
      }
      set terms $term_arr($cmp_name)
      set term1 [lindex $terms 0]
      puts -nonewline $f_sls "   \{$i_arr($j)\} $cmp_name ("
      for {set jj [expr 0]} {$jj < [llength $terms]} {incr jj} {
         set sgnl $ref_arr($i_arr($j).[lindex $terms $jj])
         puts -nonewline $f_sls "$sgnl, "
      }
      puts $f_sls "vss, vdd);"
   }
   puts $f_sls "\}"
   close $f_sls
}
