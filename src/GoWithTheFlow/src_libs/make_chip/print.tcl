
proc print_circuit {} {
   global cir_name

   set bbox [.cv bbox border]
   set FX [lindex $bbox 0]
   set FY [lindex $bbox 1]
   set FW [expr [lindex $bbox 2] - [lindex $bbox 0]]
   set FH [expr [lindex $bbox 3] - [lindex $bbox 1]]


   .cv itemconfigure grid -fill white
   .cv postscript -file $cir_name.ps\
                  -pagewidth  200m\
                  -pageheight 280m\
                  -width      $FW\
                  -height     $FH\
                  -x          $FX\
                  -y          $FY\
                  -rotate     1
   exec lp $cir_name.ps
   .cv itemconfigure grid -fill wheat2
}

proc print_layout {} {
   global cir_name SOG_DB my_wd env

   cd $my_wd/$SOG_DB
   set env(CWD) $my_wd/$SOG_DB
   catch {exec getepslay -t  -r $cir_name}
   add_user_name $cir_name
   exec lp $cir_name.ps
   cd $my_wd
}
