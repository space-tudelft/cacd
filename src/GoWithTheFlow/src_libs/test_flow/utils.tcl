
proc get_period {time} {
   global FREQ_LIST

   set i [expr 0]
   while {[lindex [lindex $FREQ_LIST $i] 0] < $time} {
      if {$i >= [expr [llength $FREQ_LIST]-1]} {
         break
      }
      incr i
   }
   return [lindex [lindex $FREQ_LIST $i] 1]
}

proc insert_pg_data {val wdw} {
   global inp_list

   for {set i [expr 0]} {$i <[llength $inp_list]} {incr i} {
      set idx [lindex [lindex $inp_list $i] 0]
      if {[string index $val $idx] == "H"} {
         set char "1"
      }\
      else {
         set char "0"
      }
      if {$i == 0} {
         $wdw insert end $char
      }\
      else {
         $wdw insert end " $char"
      }
   }
   $wdw insert end "\n"
}

proc insert_csv_data {trigsig val wdwi wdwo} {
   global inp_list outp_list check_trig

   if {($trigsig == "1") || ($trigsig == "0")} {
      $wdw insert end "$trigsig,"
   }
   for {set i [expr 0]} {$i <[llength $inp_list]} {incr i} {
      set idx [lindex [lindex $inp_list $i] 0]
      if {[string index $val $idx] == "H"} {
         set char "1"
         if {$check_trig($i) == "L"} {
            set check_trig($i) "X"
         }
      }\
      else {
         set char "0"
         if {$check_trig($i) == "H"} {
            set check_trig($i) "L"
         }
      }
      $wdwi insert end "$char,"
   }
   # $wdw insert end " "
   $wdwi insert end "\n"
   for {set i [expr 0]} {$i <[llength $outp_list]} {incr i} {
      set idx [lindex [lindex $outp_list $i] 0]
      if {[string index $val $idx] == "h"} {
         set char "1"
      }\
      else {
         set char "0"
      }
      $wdwo insert end "$char,"
   }
   $wdwo insert end "\n"
}

proc conv2hex {str} {
   set len [string length $str]
   switch $len {
      1   {append str "000"}
      2   {append str "00"}
      3   {append str "0"}
      4   {}
   }
   switch $str {
      0000 {return 0}
      1000 {return 1}
      0100 {return 2}
      1100 {return 3}
      0010 {return 4}
      1010 {return 5}
      0110 {return 6}
      1110 {return 7}
      0001 {return 8}
      1001 {return 9}
      0101 {return A}
      1101 {return B}
      0011 {return C}
      1011 {return D}
      0111 {return E}
      1111 {return F}
   }
}

proc insert_trf_data {val nbr wdw} {
   global outp_list

   set bin_str ""
   for {set i [expr 0]} {$i <[llength $outp_list]} {incr i} {
      set idx [lindex [lindex $outp_list $i] 0]
      if {[string index $val $idx] == "h"} {
         set bin_str "1$bin_str"
      }\
      else {
         set bin_str "0$bin_str"
      }
   }
   $wdw insert end "$bin_str $nbr\n"
}

proc replace_string_all {str} {
   for {set i [expr 0]} {$i < [string length $str]} {incr i} {
      set char [string index $str $i]
      if {($char != "H") && ($char != "L") && \
          ($char != "h") && ($char != "l")} {
         set str [replace_string $str $i l]
      }
   }
   return $str
}

proc replace_string {str idx char} {
   set fstr [string range $str 0 [expr $idx-1]]
   set lstr [string range $str [expr $idx+1] end]
   return "${fstr}${char}${lstr}"
}

proc get_term_text {bb n} {
   global buf_arr bb_arr

   switch $bb_arr($bb,$n) {
      NC  {return "NC"}
      VSS {return "VSS"}
      VDD {return "VDD"}
      default {return $buf_arr($bb_arr($bb,$n))}
   }
}

proc show_pod {type  x0 xc yc name ch color lncol txtcol} {
   global fnt

   set dx [expr 110]
   set dy [expr 20]
   set xlb [expr $xc-$dx/2]
   set xrb [expr $xc+$dx/2]
   set ytb [expr $yc-$dy/2]
   set ybb [expr $yc+$dy/2]
   .cons.cv create rectangle $xlb $ytb $xrb $ybb -fill $color -outline black
   switch $type {
      LIN  {.cons.cv create line $xrb $yc $x0  $yc -arrow last\
                                                   -width 3 -fill $lncol
           }
      LOUT {.cons.cv create line $xrb $yc $x0  $yc -arrow first\
                                                   -width 3 -fill $lncol
           }
      RIN  {.cons.cv create line $x0  $yc $xlb $yc -arrow first\
                                                   -width 3 -fill $lncol
           }
      ROUT {.cons.cv create line $x0  $yc $xlb $yc -arrow last\
                                                   -width 3 -fill $lncol
           }
   }
   if {$ch >= 0} {
      .cons.cv create text $xc $yc -text "$name ch $ch" -font $fnt -fill $txtcol
   }\
   else {
      .cons.cv create text $xc $yc -text $name -font $fnt -fill $txtcol
   }
}

proc add_time {mnval addval} {
   global myMaxInt myMaxLen

   set hval [string range $mnval 0 [expr $myMaxLen-1]]
   set hval [string trimleft $hval "0"]
   if {$hval == ""} {
      set hval [expr 0]
   }
   set lval [string range $mnval $myMaxLen end]
   set lval [string trimleft $lval "0"]
   if {$lval == ""} {
      set lval [expr 0]
   }
   set lval [expr $lval + $addval]
   if {$lval > $myMaxInt} {
      set lval [expr $lval - $myMaxInt - 1]
      set hval [expr $hval + 1]
   }
   return "[format %0${myMaxLen}d $hval][format %0${myMaxLen}d $lval]"
}

proc tf_txt_dialog {label_string init_str} {
#-----------------------------------------------------------------------------#
# procedure to input a string into a program via a pop_up window              #
# two buttons are present: OK and cancel                                      #
# If cancel is clicked the dialog will disapear and 'CANCELLED' will be       #
# returned.                                                                   #
# If OK is clicked the string in the entry_window will be returned            #
#-----------------------------------------------------------------------------#
   global tk_txt_dialog_retval Fnt

   set tk_txt_dialog_retval ""

   toplevel .txt_dialog
   label    .txt_dialog.lbl       -text $label_string -bg wheat -font $Fnt
   entry    .txt_dialog.ent       -width 25 -bg wheat -font $Fnt
   frame    .txt_dialog.fr        -bg gold
   button   .txt_dialog.fr.ok     -text "OK" -bg gold -font $Fnt\
                                  -command  {
      set tk_txt_dialog_retval [.txt_dialog.ent get]
      catch {destroy .txt_dialog}
   }
   button   .txt_dialog.fr.cancel -text "cancel" -bg gold -font $Fnt\
                                  -command {
      set tk_txt_dialog_retval "CANCELLED"
      catch {destroy .txt_dialog}
   }
   pack .txt_dialog.lbl       -side top   -fill x  -pady 5
   pack .txt_dialog.ent       -side top   -padx 5
   pack .txt_dialog.fr        -side top   -fill x
   pack .txt_dialog.fr.ok     -side left  -padx 15 -pady 5
   pack .txt_dialog.fr.cancel -side right -padx 15 -pady 5

   wm geom  .txt_dialog +[winfo pointerx .]+[winfo pointery .]
   wm title .txt_dialog text_dialog
   .txt_dialog.ent insert end $init_str
   focus .txt_dialog.ent
   grab set .txt_dialog
   tkwait window .txt_dialog
   return $tk_txt_dialog_retval
}


