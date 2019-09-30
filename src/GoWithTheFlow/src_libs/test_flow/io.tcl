
proc do_read_chs_file {} {
   global chs_file_name outp_list inp_list
   global ProgName

   set new_outp_list ""
   set new_inp_list ""

   set f_chs [open $chs_file_name "r"]
   while {![eof $f_chs]} {
      set txt [gets $f_chs]
      if {[lindex $txt 2] == "O"} {
         set idx [lsearch $outp_list "* [lindex $txt 0] *"]
         set item [lindex $outp_list $idx]
         set new_outp_list [lappend new_outp_list $item]
      }\
      elseif {[lindex $txt 2] == "I"} {
         set idx [lsearch $inp_list "* [lindex $txt 0] *"]
         set item [lindex $inp_list $idx]
         set new_inp_list [lappend new_inp_list $item]
      }
   }
   close $f_chs

   set outp_list $new_outp_list
   set inp_list $new_inp_list

   update_term_window
}

proc update_term_window {} {

   global inp_list outp_list

   .termtxt delete 1.0 end

   .termtxt insert end "INPUTS:\n"
   for {set i [expr 0]} {$i < [llength $inp_list]} {incr i} {
      if {[llength [lindex $inp_list $i]] > 2} {
         set term [lindex [lindex $inp_list $i] 1]
         set t_name [lindex [lindex $inp_list $i] 2]
      }\
      else {
         set term ""
         set t_name [lindex [lindex $inp_list $i] 1]
      }
      .termtxt insert end [format "   %-20s %-20s\n" $t_name $term]
   }

   .termtxt insert end "\nOUTPUTS:\n"
   for {set i [expr 0]} {$i < [llength $outp_list]} {incr i} {
      if {[llength [lindex $outp_list $i]] > 2} {
         set term [lindex [lindex $outp_list $i] 1]
         set t_name [lindex [lindex $outp_list $i] 2]
      }\
      else {
         set term ""
         set t_name [lindex [lindex $outp_list $i] 1]
      }
      .termtxt insert end [format "   %-20s %-20s\n" $t_name $term]
   }
}

proc write_pgfile {} {
   global data_name message

   set message "writing pattern_data to file $data_name.pg"
   update
   set f_pg [open $data_name.pg "w"]
   puts $f_pg [.pgtxt.txt get 1.0 end]
   close $f_pg
   set message "pattern_data written to file $data_name.pg"
}

proc old_write_csvfile {} {
   global data_name message

   set message "writing pattern_data to file $data_name.csv"
   update
   set f_csv [open $data_name.csv "w"]
   puts $f_csv [.csvtxt.txt get 1.0 end]
   close $f_csv
   set message "pattern_data written to file $data_name.csv"
}

proc old2_write_csvfile {} {
#-----------------------------------------------------------------------------#
# procedure to write the csv_file to a file                                   #
#-----------------------------------------------------------------------------#
   global data_name csvmessage

   set csvmessage "Writing csv files .... "
   update

   set nl [expr 1]
   set txtI [.csvtxt.txti get $nl.0 $nl.end]
   set txtO [.csvtxt.txto get $nl.0 $nl.end]
   set lenI [expr [string length $txtI] / 2]
   set lenO [expr [string length $txtO] / 2]
   set allFileNames ""
   set separator ""
   set GenCVSFileName [format "%s1A.csv ..." $data_name]
   set f [expr 1]
   set i [expr 1]
   while {[expr ($i - 1) * 8] < $lenI} {
      set CVSFileName [format "%s_%dA.csv" $data_name $f]
      set allFileNames [format "%s%s%s" $allFileNames $separator [file tail $CVSFileName]]
      set separator ", "
      set fpo($f) [open $CVSFileName w]
      incr i 
      incr f 
   }
   if {$f == 2} { incr f }\
   elseif {$f == 4} { incr f }
   set i [expr 1]
   while {[expr ($i - 1) * 8] < $lenO} {
      set CVSFileName [format "%s_%dAref.csv" $data_name $f]
      set allFileNames [format "%s%s%s" $allFileNames $separator [file tail $CVSFileName]]
      set separator ", "
      set fpo($f) [open $CVSFileName w]
      incr i 
      incr f 
   }
   while {[expr [string length $txtI]] > 0} {
      set f [expr 1]
      set i [expr 1]
      while {[expr ($i - 1) * 8] < $lenI} {
         set first [expr [expr $i - 1] * 16]
         set last [expr $first + 14]
         for {set j $last} {$j >= $first} {incr j -2} {
            if {$j >= [string length $txtI]} {
               puts -nonewline $fpo($f) "0,"
            }\
            else {
               puts -nonewline $fpo($f) [string range $txtI $j [expr $j + 1] ] 
            }
         }
         puts $fpo($f) ""
         incr i 
         incr f 
      }
      set i [expr 1]
      while {[expr ($i - 1) * 8] < $lenO} {
         set first [expr [expr $i - 1] * 16]
         set last [expr $first + 14]
         for {set j $last} {$j >= $first} {incr j -2} {
            if {$j >= [string length $txtO]} {
               puts -nonewline $fpo($f) "0,"
            }\
            else {
               puts -nonewline $fpo($f) [string range $txtO $j [expr $j + 1] ] 
            }
         }
         puts $fpo($f) ""
         incr i 
         incr f 
      }
      incr nl 
      set txtI [.csvtxt.txti get $nl.0 $nl.end]
      set txtO [.csvtxt.txto get $nl.0 $nl.end]
   }
   set f [expr 1]
   set i [expr 1]
   while {[expr ($i - 1) * 8] < $lenI} {
      close $fpo($f)
      incr i 
      incr f 
   }
   set i [expr 1]
   while {[expr ($i - 1) * 8] < $lenO} {
      close $fpo($f)
      incr i 
      incr f 
   }
   wm title .csvtxt "[file tail $GenCVSFileName]"
   set csvmessage "file(s) '$allFileNames' (over)written"
   update
}

proc write_csvfile {} {
#-----------------------------------------------------------------------------#
# procedure to write the csv_file to a file                                   #
#-----------------------------------------------------------------------------#
   global data_name csvmessage i_trigsig

   set csvmessage "Writing csv file .... "
   update

   set nl [expr 1]
   set txtI [.csvtxt.txti get $nl.0 $nl.end]
   set txtO [.csvtxt.txto get $nl.0 $nl.end]
   set lenI [expr [string length $txtI] / 2]
   set lenO [expr [string length $txtO] / 2]
   set GenCVSFileName [format "%s.csv" $data_name]
   set fpo [open $GenCVSFileName w]
   puts -nonewline $fpo "#"
   set i [expr 0]
   while {[expr $i] < $lenI} {
      set ch $i
      puts -nonewline $fpo ",P$ch"
      incr i 
   }
   set i [expr 0]
   while {[expr $i] < $lenI} {
      set ch $i
      puts -nonewline $fpo ",$ch"
      incr i 
   }
   set i [expr 0]
   while {[expr $i] < $lenO} {
      set ch [expr $i + 40]
      puts -nonewline $fpo ",$ch"
      incr i 
   }
   puts $fpo ""
   while {[expr [string length $txtI]] > 0} {
      set k [expr $nl - 1]
      puts -nonewline $fpo "$k,"
      puts -nonewline $fpo $txtI
      puts -nonewline $fpo $txtI
      puts $fpo $txtO
      incr nl 
      set txtI [.csvtxt.txti get $nl.0 $nl.end]
      set txtO [.csvtxt.txto get $nl.0 $nl.end]
   }
   close $fpo
   write_ini_file
   wm title .csvtxt "[file tail $GenCVSFileName]"
   set csvmessage "file '$GenCVSFileName' (over)written"
   update
}

proc write_ini_file {} {
   global TestFlowLibPath data_name
   global outp_list inp_list
   global pg_offset

   set IniFileName [format "%s.ini" $data_name]
   set fpw [open $IniFileName w]
   set fpr [open "$TestFlowLibPath/la5000.ini" r]

   while {![eof $fpr]} {
      set txt [gets $fpr]
      if {[string equal $txt "\[ChannelData\]"]} {
         puts $fpw $txt
         set il [expr 0]
         for {set i [expr 0]} {$i < [llength $inp_list] + $pg_offset} {incr i} {
            if {$i == 0 && $pg_offset == 1} {
               set t_name "trig_gen"
            }\
            elseif {[llength [lindex $inp_list $il]] > 2} {
               set t_name [lindex [lindex $inp_list $il] 2]
               incr il
            }\
            else {
               set t_name [lindex [lindex $inp_list $il] 1]
               incr il
            }
            set p [expr [expr $i ]/ 8 + 1]
            set cnr [format "%2d" $i]
            puts $fpw "N$i=${p}A $cnr $t_name"
         }
         set il [expr 0]
         for {set i [expr 0]} {$i < [expr [llength $outp_list] + 1]} {incr i} {
            if {$i == 0} {
               set t_name "trigger"
            }\
            elseif {[llength [lindex $outp_list $il]] > 2} {
               set t_name [lindex [lindex $outp_list $il] 2]
               incr il
            }\
            else {
               set t_name [lindex [lindex $outp_list $il] 1]
               incr il
            }
            set c [expr $i + 40]
            set p [expr [expr $i ]/ 8 + 1]
            set cnr [format "%2d" $i]
            puts $fpw "N$c=${p}B $cnr $t_name"
         }
      }\
      elseif {[string equal $txt "TimingLines=0"]} {
         set n 0
         set nr [expr [llength $inp_list] + $pg_offset + [llength $outp_list] + 1]
         puts $fpw "TimingLines=$nr"
         for {set i [expr 0]} {$i < [llength $inp_list] + $pg_offset} {incr i} {
             puts $fpw "LSE$n=$i,5,0"
             incr n
         }
         for {set i [expr 0]} {$i < [expr [llength $outp_list] + 1]} {incr i} {
             set c [expr $i + 40]
             puts $fpw "LSE$n=$c,5,0"
             incr n
         }
      }\
      elseif {[string equal $txt "\[Group\]"]} {
         set txt [gets $fpr]
         set nro [llength $outp_list]

         puts $fpw "\[Group10\]"
         puts $fpw "M=0"
         puts $fpw "N=Group outputs"
         puts $fpw "S=$nro"
         puts -nonewline $fpw "L="
         for {set i [expr [llength $outp_list] - 1]} {$i >= 0} {incr i -1} {
            set c [expr $i + 41]
            puts -nonewline $fpw "$c,"
         }
         puts $fpw ""
         puts $fpw "Color10=0,0,0"
         puts $fpw ""
         puts $fpw "\[Group\]"
         puts $fpw "TotGroups=11"
      }\
      else {
         puts $fpw $txt
      }
   }

   close $fpw
   close $fpr
}

proc write_orffile {} {
   global data_name message

   set message "writing reference_data to file $data_name.orf"
   update
   set f_orf [open $data_name.orf "w"]
   puts $f_orf [.orftxt.txt get 1.0 end]
   close $f_orf
   set message "orf_data written to file $data_name.orf"
}

proc write_ps {} {
   global data_name

   .cons.cv postscript -file ${data_name}_con.ps
}

