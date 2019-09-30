
proc read_cir_file {file_name} {
   global cir_name CELL_ARR N_CELL CONN_ARR N_CONN N_GND CELL_FNT CELL_NH
   global BBOX_ID GW CELLS_MADE SOG_DB PORT_ARR N_PORT SOG_DB
   global env

   set N_CELL [expr -1]
   set N_CONN [expr -1]
   set N_PORT [expr -1]
   .cv delete all
   set cir_name [lindex [split $file_name .] 0]
   set f_cir [open $file_name "r"]
   while {![eof $f_cir]} {
      set txt [gets $f_cir]
      if {$txt == ""} {
         continue
      }
      if {[lindex $txt 0] == "I"} {
         if {[llength $txt] != 4} {
            incr N_CELL
            set CELL_ARR($N_CELL) ""
            continue
         }
         set cname [lindex [split [lindex $txt 1] "_"] 0]
         switch $cname {
            "na210"  {do_add_na210     [lindex $txt 2] [lindex $txt 3]}
            "no210"  {do_add_no210     [lindex $txt 2] [lindex $txt 3]}
            "iv110"  {do_add_iv110     [lindex $txt 2] [lindex $txt 3]}
            "ex210"  {do_add_ex210     [lindex $txt 2] [lindex $txt 3]}
            "vss"    {do_add_gnd_term  [lindex $txt 2] [lindex $txt 3]}
            "and2"   {do_add_and2      [lindex $txt 2] [lindex $txt 3]}
            "fadder" {do_add_fadder    [lindex $txt 2] [lindex $txt 3]}
         }
      }\
      elseif {[lindex $txt 0] == "C"} {
         if {[llength $txt] != 2} {
            incr N_CONN
            set CONN_ARR($N_CONN) ""
            continue
         }
         set tags [split [lindex $txt 1] ">"]
         connect [lindex $tags 0] [lindex $tags 1]
      }\
      elseif {[lindex $txt 0] == "B"} {
         set BBOX_ID [.cv create rectangle [lindex $txt 1]\
                                           [lindex $txt 2]\
                                           [lindex $txt 3]\
                                           [lindex $txt 4]\
                                           -tag border]
         .cv create rectangle [lindex $txt 1] [lindex $txt 2]\
                              [lindex $txt 3]\
                              [expr [lindex $txt 2]+2*$CELL_NH]\
                              -fill gold -tag border
         set xtxt [expr ([lindex $txt 1] + [lindex $txt 3])/2.]
         set ytxt [expr [lindex $txt 2] + $CELL_NH]
         .cv create text $xtxt $ytxt\
                         -text "\($env(USER)\)[lindex $txt 5]" -font $CELL_FNT\
                         -tag border
        # make a grid
         set xc [expr ([lindex $txt 1]+[lindex $txt 3])/2.]
         set yc [expr ([lindex $txt 2]+2*$CELL_NH+[lindex $txt 4])/2.]
         .cv create text $xc $yc -text "" -tag "border origin"
         set x $xc
         while {$x > [lindex $txt 1]} {
            .cv create line $x  [expr [lindex $txt 2]+2*$CELL_NH+1] $x [lindex $txt 4] \
                                -fill wheat2  -tag "border grid"
            set x [expr $x-$GW]
         }
         set x [expr $xc+$GW]
         while {$x < [lindex $txt 3]} {
            .cv create line $x  [expr [lindex $txt 2]+2*$CELL_NH+1] $x [lindex $txt 4] \
                                -fill wheat2  -tag "border grid"
            set x [expr $x+$GW]
         }
         set y $yc
         while {$y > [expr [lindex $txt 2]+2*$CELL_NH]} {
            .cv create line [expr [lindex $txt 1]+1]  $y [lindex $txt 3] $y\
                                -fill wheat2  -tag "border grid"
            set y [expr $y-$GW]
         }
         set y [expr $yc+ $GW]
         while {$y < [lindex $txt 4]} {
            .cv create line [expr [lindex $txt 1]+1]  $y [lindex $txt 3] $y\
                                -fill wheat2  -tag "border grid"
            set y [expr $y+$GW]
         }

      }\
      elseif {[lindex $txt 0] == "T"} {
         add_term [lindex $txt 3] [lindex $txt 4]\
                  [lindex $txt 1] [lindex $txt 2]
      }\
   }
   close $f_cir
   .fr1.rb1 configure -state normal
   .fr1.rb2 configure -state normal
   .fr1.rb3 configure -state normal
   .fr1.rb4 configure -state normal
   .fr2.rb1 configure -state normal
   .fr2.rb2 configure -state normal
   .fr2.rb3 configure -state normal
   if {$cir_name == "and2"} {
      .fr1.rb10 configure -state disabled
      .fr1.rb11 configure -state normal
   }\
   elseif {$cir_name == "fadder"} {
      .fr1.rb10 configure -state normal
      .fr1.rb11 configure -state disabled
   }\
   else {
      .fr1.rb10 configure -state normal
      .fr1.rb11 configure -state normal
   }
   set CELLS_MADE ""
   set ftmp [open $SOG_DB/layout/celllist]
   while {![eof $ftmp]} {
      set txt [gets $ftmp]
      if {$txt == "and2"} {
         lappend CELLS_MADE "and2"
      }\
      elseif {$txt == "fadder"} {
         lappend CELLS_MADE "fadder"
      }
   }
   close $ftmp
   .fr2.file.cmds entryconfigure Write -state normal
   .fr2.file.cmds entryconfigure PrintCircuit -state normal
   if {[file exists $SOG_DB/layout/$cir_name] == 1} {
      .fr2.file.cmds entryconfigure PrintLayout -state normal
   }\
   else {
      .fr2.file.cmds entryconfigure PrintLayout -state disabled
   }
}
