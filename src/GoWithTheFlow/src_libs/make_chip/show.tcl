
proc show_instances {} {
   global CELL_ARR N_CELL

   puts "-------------------------------------"
   puts "instances:"
   puts "-------------------------------------"
   for {set i [expr 0]} {$i <= $N_CELL} {incr i} {
      puts "$i $CELL_ARR($i)"
   }
   puts "-------------------------------------"
}

proc show_conns {} {
   global CONN_ARR N_CONN

   puts "-------------------------------------"
   puts "connections:"
   puts "-------------------------------------"
   for {set i [expr 0]} {$i <= $N_CONN} {incr i} {
      puts "$i $CONN_ARR($i)"
   }
   puts "-------------------------------------"
}

proc show_ports {} {
   global PORT_ARR N_PORT

   puts "-------------------------------------"
   puts "ports:"
   puts "-------------------------------------"
   for {set i [expr 0]} {$i <= $N_PORT} {incr i} {
      puts "$i $PORT_ARR($i)"
   }
   puts "-------------------------------------"
}
