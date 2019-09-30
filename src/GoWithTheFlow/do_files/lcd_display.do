
proc get_oneline_data {widget idx} {
   global myglobalvar

   set d_idx [expr ($idx + $myglobalvar($widget:sh_val)) % 80]
   return $myglobalvar($widget:data_$d_idx)
}

proc get_twoline_data {widget idx idy} {
   global myglobalvar

   if {$idy == 0} {
      set d_idx [expr ($idx + $myglobalvar($widget:sh_val)) % 40]
   }\
   elseif {$idy == 1} {
      set d_idx [expr ($idx + $myglobalvar($widget:sh_val)) % 40 + 64]
   }
   return $myglobalvar($widget:data_$d_idx)
}

proc update_cursor_pos {widget direction shift_dir} {
   global myglobalvar

   set cur_pos $myglobalvar($widget:cur_pos)
   if {$myglobalvar($widget:nl) == "1_LINE"} {
      set myglobalvar($widget:cur_pos)\
         [expr ($myglobalvar($widget:ac) - $myglobalvar($widget:sh_val)) % 80]
   }\
   elseif {$myglobalvar($widget:nl) == "2_LINE"} {
      if {$myglobalvar($widget:ac) < 40} {
         set myglobalvar($widget:cur_pos)\
                        [expr ($myglobalvar($widget:ac)\
                               - $myglobalvar($widget:sh_val)) % 40]
      }\
      else {
         set myglobalvar($widget:cur_pos)\
                        [expr ($myglobalvar($widget:ac)\
                               - $myglobalvar($widget:sh_val) - 64) % 40 + 64]
      }
   }
}

proc get_cursor_tag {widget} {
   global myglobalvar

   if {$myglobalvar($widget:nl) == "1_LINE"} {
      set c_pos $myglobalvar($widget:cur_pos)
      if {($c_pos < 0) || ($c_pos > 15) } {
          return ""
      }\
      else {
         return "curs_${c_pos}_0"
     }
   }\
   elseif {$myglobalvar($widget:nl) == "2_LINE"} {
      set c_pos $myglobalvar($widget:cur_pos)
      if {($c_pos >= 0) && ($c_pos <= 15) } {
         return "curs_${c_pos}_0"
      }\
      elseif {($c_pos >= 64) && ($c_pos <= 79) } {
         return "curs_[expr $c_pos - 64]_1"
      }\
      else {
         return ""
      }
   }
}

proc update_ram_address {widget direction} {
   global myglobalvar

   set ac $myglobalvar($widget:ac)
   if {$myglobalvar($widget:nl) == "1_LINE"} {
      if {$direction == "INCREASE"} {
         incr myglobalvar($widget:ac)
      }\
      else {
         incr myglobalvar($widget:ac) -1
      }
      set myglobalvar($widget:ac) [expr $myglobalvar($widget:ac) % 80]
   }\
   else {
      if {$direction == "INCREASE"} {
         if {$ac == 39} {
            set myglobalvar($widget:ac) [expr 64]
         }\
         elseif {$ac >= 103} {
            set myglobalvar($widget:ac) [expr 0]
         }\
         else {
            incr myglobalvar($widget:ac)
         }
      }\
      else {
         if {$ac == 64} {
            set myglobalvar($widget:ac) [expr 39]
         }\
         elseif {$ac <= 0} {
            set myglobalvar($widget:ac) [expr 103]
         }\
         else {
            incr myglobalvar($widget:ac) -1
         }
      }
   }
}

proc update_shift_val {widget shift_dir} {
   global myglobalvar

   set sh_val $myglobalvar($widget:sh_val)
   if {$shift_dir == "LEFT"} {
      incr myglobalvar($widget:sh_val)
   }\
   elseif {$shift_dir == "RIGHT"} {
      incr myglobalvar($widget:sh_val) -1
   }
   if {$myglobalvar($widget:nl) == "1_LINE"} {
      set myglobalvar($widget:sh_val) [expr $myglobalvar($widget:sh_val) % 80]
   }\
   elseif {$myglobalvar($widget:nl) == "2_LINE"} {
      set myglobalvar($widget:sh_val) [expr $myglobalvar($widget:sh_val) % 40]
   }
}

proc draw_lcd_display {widget} {
   global myglobalvar

   set ldw [winfo width $widget.cv]
   set ldh [winfo heigh $widget.cv]
   set lx  [expr $ldw/17.7]
   set dx  [expr $ldw/177.]
   set ly  [expr $ldh*15/33.]
   set dy  [expr $ldh/33.]
   set nx  [expr 16]
   set ny  [expr 2]
   set fnt -*-Courier-Bold-R-Normal--*-240-*
   $widget.cv delete all

   if {$myglobalvar($widget:mode) != "8_BIT"} {
      echo "LCD:only 8_bit format implemented"
      return
   }
   if {$myglobalvar($widget:nl) == "1_LINE"} {
      for {set i [expr 0]} {$i < $nx} { incr i} {
         $widget.cv create rectangle [expr $dx+$i*($lx+$dx)]\
                                     [expr $dy]\
                                     [expr ($i+1)*($lx+$dx)]\
                                     [expr $ly+$dy]\
                                     -fill wheat3 \
                                     -outline wheat\
                                     -tag lcd_bg
         if {$myglobalvar($widget:ft) == "5x10"} {
            $widget.cv create rectangle [expr $dx+$i*($lx+$dx)]\
                                        [expr $ly+2*$dy]\
                                        [expr ($i+1)*($lx+$dx)]\
                                        [expr 1.3*$ly+2*$dy]\
                                        -fill wheat3 \
                                        -outline wheat\
                                        -tag lcd_bg
         }
         if {$myglobalvar($widget:state) == "ON"} {
            $widget.cv create text [expr $dx+$i*($lx+$dx)+$lx/2.]\
                                   [expr $dy+$ly/2.]\
                                   -text [get_oneline_data $widget $i]\
                                   -font $fnt\
                                   -tag char_${i}_0
            if {$myglobalvar($widget:cursor) != "OFF"} {
               if {$myglobalvar($widget:ft) == "5x7"} {
                  $widget.cv create line [expr $dx+$i*($lx+$dx)+2]\
                                         [expr $ly+$dy-2]\
                                         [expr ($i+1)*($lx+$dx)-2]\
                                         [expr $ly+$dy-2]\
                                         -fill wheat3 -width 2\
                                         -tag  curs_${i}_0
               }\
               else {
                  $widget.cv create line [expr $dx+$i*($lx+$dx)+2]\
                                         [expr 1.3*$ly+2*$dy-2]\
                                         [expr ($i+1)*($lx+$dx)-2]\
                                         [expr 1.3*$ly+2*$dy-2]\
                                         -fill wheat3 -width 2\
                                         -tag  curs_${i}_0
               }
            }
         }
      }
   }\
   elseif {$myglobalvar($widget:nl) == "2_LINE"} {
      for {set i [expr 0]} {$i < $nx} { incr i} {
         for {set j [expr 0]} {$j < $ny} { incr j} {
            $widget.cv create rectangle [expr $dx+$i*($lx+$dx)]\
                                        [expr $dy+$j*($ly+$dy)]\
                                        [expr ($i+1)*($lx+$dx)]\
                                        [expr ($j+1)*($ly+$dy)]\
                                        -fill wheat3 \
                                        -outline wheat\
                                        -tag lcd_bg
            if {$myglobalvar($widget:state) == "ON"} {
               $widget.cv create text [expr $dx+$i*($lx+$dx)+$lx/2.]\
                                      [expr $dy+$j*($ly+$dy)+$ly/2.]\
                                      -text [get_twoline_data $widget $i $j]\
                                      -font $fnt\
                                      -tag char_${i}_${j}
               if {$myglobalvar($widget:cursor) != "OFF"} {
                  $widget.cv create line [expr $dx+$i*($lx+$dx)+2]\
                                         [expr ($j+1)*($ly+$dy)-2]\
                                         [expr ($i+1)*($lx+$dx)-2]\
                                         [expr ($j+1)*($ly+$dy)-2]\
                                         -fill wheat3 -width 2\
                                         -tag curs_${i}_${j}
               }
            }
         }

      }
   }
   if {$myglobalvar($widget:cursor) != "OFF"} {
      set curs_tag [get_cursor_tag $widget]
      if {$curs_tag != ""} {
         $widget.cv itemconfigure $curs_tag -fill red
      }
   }
}

proc change_ena_signal {widget} {
   global myglobalvar

   nowhen [string range $widget 4 end]_a
   when -label [string range $widget 4 end]_a\
        $myglobalvar($widget:e_in) "send_code $widget"
}

proc change_rs_signal {widget} {
   global myglobalvar

   nowhen [string range $widget 4 end]_b
   when -label [string range $widget 4 end]_b\
        $myglobalvar($widget:rs_in) "check_rs_holdtime $widget"
}

proc change_rw_signal {widget} {
   global myglobalvar

   nowhen [string range $widget 4 end]_c
   when -label [string range $widget 4 end]_c\
        $myglobalvar($widget:rw_in) "check_rw_holdtime $widget"
}

proc change_data_signal {widget} {
   global myglobalvar

   nowhen [string range $widget 4 end]_d
   when -label [string range $widget 4 end]_d\
        $myglobalvar($widget:data_in) "check_data_holdtime $widget"
}

proc mk_lcd_display {widget} {
  global myglobalvar
  global LD_BITLEN

   set fnt "-*-Courier-Bold-R-Normal--*-120-*"

   set signals [find signals -out -internal /*]
   set possible_data_signals ""
   foreach s $signals {
      if {[string length [examine $s]] == $LD_BITLEN} {
         set idx [string last "/" $s]
         lappend possible_data_signals  [string range $s $idx end]
      }
      if {[string length [examine $s]] == 1} {
         set idx [string last "/" $s]
         lappend possible_ena_signals  [string range $s $idx end]
         lappend possible_rw_signals  [string range $s $idx end]
         lappend possible_rs_signals  [string range $s $idx end]
      }
   }
   if { ($possible_data_signals == "") || ($possible_ena_signals == "")} {
      echo "*****************************************************"
      echo "** Cannot find a suitable signals for lcd_display"
      echo "** macro lcd_display not created"
      echo "*****************************************************"
   }\
   else {
      toplevel $widget
      canvas   $widget.cv -width 354 -height 66 -bg wheat
      frame    $widget.fr1 -relief ridge -bd 2 -bg gold
      frame    $widget.fr2 -relief ridge -bd 2 -bg gold
      frame    $widget.fr3 -relief ridge -bd 2 -bg gold
      frame    $widget.fr4 -relief ridge -bd 2 -bg gold
      frame    $widget.fr5 -relief ridge -bd 2 -bg gold
      checkbutton $widget.fr5.cb1 -text time_check\
                                  -variable myglobalvar($widget:chk)\
                                  -bg gold -font $fnt
      checkbutton $widget.fr5.cb2 -text "echo data"\
                                  -variable myglobalvar($widget:verb)\
                                  -bg gold -font $fnt
      button   $widget.fr5.qt     -bg red -font $fnt -text "Quit"\
                                  -command "destroy $widget"
      label    $widget.fr1.lbl -text "rs" -width 2 -bg gold -font $fnt
      eval tk_optionMenu $widget.fr1.rs myglobalvar($widget:rs_in)\
                                                 $possible_rs_signals
      label    $widget.fr2.lbl -text "rw" -width 2 -bg gold -font $fnt
      eval tk_optionMenu $widget.fr2.rw myglobalvar($widget:rw_in)\
                                                 $possible_rw_signals
      label    $widget.fr3.lbl -text "e" -width 2 -bg gold -font $fnt
      eval tk_optionMenu $widget.fr3.e myglobalvar($widget:e_in)\
                                                 $possible_ena_signals
      label    $widget.fr4.lbl -text "db" -width 2 -bg gold -font $fnt
      eval tk_optionMenu $widget.fr4.dat myglobalvar($widget:data_in)\
                                                 $possible_data_signals

      pack $widget.cv  -side top -fill both -expand 1
      pack $widget.fr1 -side top -fill x
      pack $widget.fr2 -side top -fill x
      pack $widget.fr3 -side top -fill x
      pack $widget.fr4 -side top -fill x
      pack $widget.fr5 -side top -fill x
      pack $widget.fr1.lbl -side left -padx 10
      pack $widget.fr1.rs -side right -padx 10 -pady 2 -fill x -expand 1
      pack $widget.fr2.lbl -side left -padx 10
      pack $widget.fr2.rw -side right -padx 10 -pady 2 -fill x -expand 1
      pack $widget.fr3.lbl -side left -padx 10
      pack $widget.fr3.e -side right -padx 10 -pady 2 -fill x -expand 1
      pack $widget.fr4.lbl -side left -padx 10
      pack $widget.fr4.dat -side right -padx 10 -pady 2 -fill x -expand 1
      pack $widget.fr5.cb1 -side left -padx 10 -pady 2
      pack $widget.fr5.cb2 -side left -padx 10 -pady 2
      pack $widget.fr5.qt  -side right -padx 10 -pady 2

      wm title $widget [string range $widget 4 end]

      tkwait visibility $widget.cv

      $widget.fr1.rs configure -font $fnt -bg gold
      $widget.fr1.rs.menu configure -font $fnt -bg gold
      $widget.fr2.rw configure -font $fnt -bg gold
      $widget.fr2.rw.menu configure -font $fnt -bg gold
      $widget.fr3.e configure -font $fnt -bg gold
      $widget.fr3.e.menu configure -font $fnt -bg gold
      $widget.fr4.dat configure -font $fnt -bg gold
      $widget.fr4.dat.menu configure -font $fnt -bg gold
      bind $widget.fr1.rs.menu <ButtonRelease> "change_rs_signal $widget"
      bindtags $widget.fr1.rs.menu [list Menu all $widget.fr1.rs.menu]
      bind $widget.fr2.rw.menu <ButtonRelease> "change_rw_signal $widget"
      bindtags $widget.fr2.rw.menu [list Menu all $widget.fr2.rw.menu]
      bind $widget.fr3.e.menu <ButtonRelease> "change_ena_signal $widget"
      bindtags $widget.fr3.e.menu [list Menu all $widget.fr3.e.menu]
      bind $widget.fr4.dat.menu <ButtonRelease> "change_data_signal $widget"
      bindtags $widget.fr4.dat.menu [list Menu all $widget.fr4.dat.menu]

      set myglobalvar($widget:mode)      "8_BIT"
      set myglobalvar($widget:nl)        "1_LINE"
      set myglobalvar($widget:ft)        "5x7"
      set myglobalvar($widget:shift)     "UNKNOWN"
      set myglobalvar($widget:direction) "UNKNOWN"
      set myglobalvar($widget:state)     "OFF"
      set myglobalvar($widget:cursor)    "OFF"
      set myglobalvar($widget:cur_pos)   [expr 0]
      set myglobalvar($widget:ac)        [expr 0]
      set myglobalvar($widget:sh_val)    [expr 0]
      set myglobalvar($widget:mit)       [expr 0]
      set myglobalvar($widget:rwt)       [expr 0]
      set myglobalvar($widget:rst)       [expr 0]
      set myglobalvar($widget:edt)       [expr 0]
      set myglobalvar($widget:eut)       [expr 0]
      set myglobalvar($widget:dat)       [expr 0]
      set myglobalvar($widget:chk)       "1"
      set myglobalvar($widget:verb)      "0"
      for {set i [expr 0]} {$i <= 103} {incr i} {
         set myglobalvar($widget:data_$i) ""
      }

      bind $widget <Expose> "draw_lcd_display $widget"

      bind $widget.cv <Destroy> "lcd_remove $widget"

      draw_lcd_display $widget

      when -label [string range $widget 4 end]_a\
           $myglobalvar($widget:e_in) "send_code $widget"
      when -label [string range $widget 4 end]_b\
           $myglobalvar($widget:rs_in) "check_rs_holdtime $widget"
      when -label [string range $widget 4 end]_c\
           $myglobalvar($widget:rw_in) "check_rw_holdtime $widget"
      when -label [string range $widget 4 end]_d\
           $myglobalvar($widget:data_in) "check_data_holdtime $widget"
   }
}

proc get_lcd_name {widget} {
   set nbr [lindex [split $widget "_"] 1]
   return "LCD_$nbr"
}

proc check_rs_holdtime {widget} {
   global myglobalvar now

   set myglobalvar($widget:rst) $now
   if {$myglobalvar($widget:chk) == "1"} {
      if {[examine /$myglobalvar($widget:e_in)] == 1} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "   change of rs_signal while e = 1 is not allowed"
      }
      set rs_holdtime [expr $now - $myglobalvar($widget:edt)]
      if {($now != 0) && ($rs_holdtime < 10)} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "    rs_holdtime ($rs_holdtime) too small"
      }
   }
}

proc check_rw_holdtime {widget} {
   global myglobalvar now

   set myglobalvar($widget:rwt) $now
   if {$myglobalvar($widget:chk) == "1"} {
      if {[examine /$myglobalvar($widget:e_in)] == 1} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "    change of rw_signal while e = 1 is not allowed"
      }
      set rw_holdtime [expr $now - $myglobalvar($widget:edt)]
      if {($now != 0) && ($rw_holdtime < 10)} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "    rw_holdtime($rw_holdtime) too small"
      }
   }
}

proc check_data_holdtime {widget} {
   global myglobalvar now

   set myglobalvar($widget:dat) $now
   if {$myglobalvar($widget:chk) == "1"} {
      set data_holdtime [expr $now - $myglobalvar($widget:edt)]
      if {($now != 0) && ($data_holdtime < 10)} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "    data_holdtime($data_holdtime) too small"
      }
   }
}

proc send_code {widget} {
   global myglobalvar now

   set myglobalvar($widget:edt) $now
   if {($myglobalvar($widget:chk) == "1") & \
       ([examine /$myglobalvar($widget:e_in)]) == 1} {
      set myglobalvar($widget:eut) $now
      set rs_setup_time [expr $now - $myglobalvar($widget:rst)]
      if {($now != 0) && ($rs_setup_time < 40)} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "   Rs_setup_time($rs_setup_time) too small"
      }
      set rw_setup_time [expr $now - $myglobalvar($widget:rwt)]
      if {($now != 0) && ($rw_setup_time < 40)} {
         echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
         echo "   Rw_setup_time($rw_setup_time) too small"
      }
   }\
   elseif {[examine /$myglobalvar($widget:e_in)] == 0} {
      set e_width [expr $now - $myglobalvar($widget:eut)]
# do nothing in case of a glitch
      if {$e_width <= 10} {
         return
      }
      if {$myglobalvar($widget:chk) == "1"} {
         set $myglobalvar($widget:edt) $now
         if {($now != 0) && ($e_width < 220)} {
            echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
            echo "   pulswidth($e_width) of e too small"
         }
         set data_setup_time [expr $now - $myglobalvar($widget:dat)]
         if {($now != 0) && ($data_setup_time < 60)} {
            echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
            echo "   data_setup_time($data_setup_time) too small"
         }
         if {$now < $myglobalvar($widget:mit)} {
            echo "LCD_WARNING on [get_lcd_name $widget] at time $now:"
            echo "   previous instruction may not yet be ready"
         }
      }
       set rs  [examine /$myglobalvar($widget:rs_in)]
       set r_w [examine /$myglobalvar($widget:rw_in)]
       set db  [examine /$myglobalvar($widget:data_in)]
       if {($rs == 0) && ($r_w == 0)} {
          if { $db == "00000001"} {
             set myglobalvar($widget:mit) [expr $now + 1530000]
             clear_display $widget
          }\
          elseif {[string match 0000001? $db]} {
             set myglobalvar($widget:mit) [expr $now + 1530000]
             return_home $widget
          }\
          elseif {[string match 00000100 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:direction) "DECREASE"
             set myglobalvar($widget:shift)     "NOSHIFT"
          }\
          elseif {[string match 00000101 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:direction) "DECREASE"
             set myglobalvar($widget:shift)     "RIGHT"
          }\
          elseif {[string match 00000110 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:direction) "INCREASE"
             set myglobalvar($widget:shift)     "NOSHIFT"
          }\
           elseif {[string match 00000111 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:direction) "INCREASE"
             set myglobalvar($widget:shift)     "LEFT"
          }\
          elseif {[string match 0000100? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state)  "OFF"
             set myglobalvar($widget:cursor) "OFF"
             draw_lcd_display $widget
          }\
          elseif {[string match 00001010 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state)  "OFF"
             set myglobalvar($widget:cursor) "ON"
             draw_lcd_display $widget
          }\
          elseif {[string match 00001011 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state)  "OFF"
             set myglobalvar($widget:cursor) "BLINKING"
             draw_lcd_display $widget
          }\
          elseif {[string match 0000110? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state)  "ON"
             set myglobalvar($widget:cursor) "OFF"
             draw_lcd_display $widget
          }\
          elseif {[string match 00001110 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state) "ON"
             set myglobalvar($widget:cursor) "ON"
             draw_lcd_display $widget
          }\
          elseif {[string match 00001111 $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:state) "ON"
             set myglobalvar($widget:cursor) "BLINKING"
             draw_lcd_display $widget
          }\
          elseif {[string match 000100?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             update_ram_address $widget DECREASE
             update_cursor_pos $widget DECREASE NOSHIFT
             draw_lcd_display $widget
          }\
          elseif {[string match 000101?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             update_ram_address $widget INCREASE
             update_cursor_pos $widget INCREASE NOSHIFT
             draw_lcd_display $widget
          }\
          elseif {[string match 000110?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             update_shift_val $widget "LEFT"
             update_cursor_pos $widget DECREASE NOSHIFT
             draw_lcd_display $widget
          }\
          elseif {[string match 000111?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             update_shift_val $widget "RIGHT"
             update_cursor_pos $widget INCREASE NOSHIFT
             draw_lcd_display $widget
          }\
          elseif {[string match 001000?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "4_BIT"
             set myglobalvar($widget:nl) "1_LINE"
             set myglobalvar($widget:ft) "5x7"
             draw_lcd_display $widget
          }\
          elseif {[string match 001001?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "4_BIT"
             set myglobalvar($widget:nl) "1_LINE"
             set myglobalvar($widget:ft) "5x10"
             draw_lcd_display $widget
          }\
          elseif {[string match 001010?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "4_BIT"
             set myglobalvar($widget:nl) "2_LINE"
             set myglobalvar($widget:ft) "5x7"
             draw_lcd_display $widget
          }\
          elseif {[string match 001011?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "4_BIT"
             set myglobalvar($widget:nl) "2_LINE"
             set myglobalvar($widget:ft) "5x10"
             draw_lcd_display $widget
          }\
          elseif {[string match 001100?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "8_BIT"
             set myglobalvar($widget:nl) "1_LINE"
             set myglobalvar($widget:ft) "5x7"
             draw_lcd_display $widget
          }\
          elseif {[string match 001101?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "8_BIT"
             set myglobalvar($widget:nl) "1_LINE"
             set myglobalvar($widget:ft) "5x10"
             draw_lcd_display $widget
          }\
          elseif {[string match 001110?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "8_BIT"
             set myglobalvar($widget:nl) "2_LINE"
             set myglobalvar($widget:ft) "5x7"
             draw_lcd_display $widget
          }\
          elseif {[string match 001111?? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set myglobalvar($widget:mode) "8_BIT"
             set myglobalvar($widget:nl) "2_LINE"
             set myglobalvar($widget:ft) "5x10"
             draw_lcd_display $widget
          }\
          elseif {[string match 01?????? $db]} {
             echo "LCD_ERROR on [get_lcd_name $widget] at time $now:"
             echo "    instruction setGGRAMAddress not implemented"
          }\
          elseif {[string match 1??????? $db]} {
             set myglobalvar($widget:mit) [expr $now + 39000]
             set_ac $widget [string range $db 1 end]
         }
       }\
       elseif {($rs == 1) && ($r_w == 0)} {
          set myglobalvar($widget:mit) [expr $now + 43000]
          write_data $widget $db
       }\
       elseif {($rs == 1) && ($r_w == 1)} {
          echo "LCD_ERROR on [get_lcd_name $widget] at time $now:"
          echo "   read instruction not implemented"
       }\
       else {
          echo "LCD_ERROR on [get_lcd_name $widget] at time $now:"
          echo "   illegal instruction"
       }
   }
}

proc lcd_remove {widget} {
  nowhen [string range $widget 4 end]_*
}

proc clear_display {widget} {
   global myglobalvar

   for {set i [expr 0]} {$i <= 103} {incr i} {
      set myglobalvar($widget:data_$i) ""
   }
   set myglobalvar($widget:ac) [expr 0]
   set myglobalvar($widget:sh_val) [expr 0]
   set myglobalvar($widget:direction) "INCREASE"
   set myglobalvar($widget:cur_pos) [expr 0]
   draw_lcd_display $widget
}

proc return_home {widget} {
   global myglobalvar

   set myglobalvar($widget:ac) [expr 0]
   set myglobalvar($widget:sh_val) [expr 0]
   set myglobalvar($widget:cur_pos) [expr 0]
   draw_lcd_display $widget
}

proc set_ac {widget str} {
   global myglobalvar

   set val [expr 0]
   set ref_val [expr 1]
   set len [string length $str]
   for {set i [expr $len-1]} {$i >=0} {incr i -1} {
      if {[string index $str $i] == "1"} {
         set val [expr $val + $ref_val]
      }
      set ref_val [expr 2*$ref_val]
   }
   set myglobalvar($widget:ac) $val
   update_cursor_pos  $widget $myglobalvar($widget:direction) NOSHIFT
   draw_lcd_display $widget
}


proc write_data {widget strng} {
   global myglobalvar now

   switch $strng {
      01000001 {set char "A"}
      01000010 {set char "B"}
      01000011 {set char "C"}
      01000100 {set char "D"}
      01000101 {set char "E"}
      01000110 {set char "F"}
      01000111 {set char "G"}
      01001000 {set char "H"}
      01001001 {set char "I"}
      01001010 {set char "J"}
      01001011 {set char "K"}
      01001100 {set char "L"}
      01001101 {set char "M"}
      01001110 {set char "N"}
      01001111 {set char "O"}
      01010000 {set char "P"}
      01010001 {set char "Q"}
      01010010 {set char "R"}
      01010011 {set char "S"}
      01010100 {set char "T"}
      01010101 {set char "U"}
      01010110 {set char "V"}
      01010111 {set char "W"}
      01011000 {set char "X"}
      01011001 {set char "Y"}
      01011010 {set char "Z"}
      01100001 {set char "a"}
      01100010 {set char "b"}
      01100011 {set char "c"}
      01100100 {set char "d"}
      01100101 {set char "e"}
      01100110 {set char "f"}
      01100111 {set char "g"}
      01101000 {set char "h"}
      01101001 {set char "i"}
      01101010 {set char "j"}
      01101011 {set char "k"}
      01101100 {set char "l"}
      01101101 {set char "m"}
      01101110 {set char "n"}
      01101111 {set char "o"}
      01110000 {set char "p"}
      01110001 {set char "q"}
      01110010 {set char "r"}
      01110011 {set char "s"}
      01110100 {set char "t"}
      01110101 {set char "u"}
      01110110 {set char "v"}
      01110111 {set char "w"}
      01111000 {set char "x"}
      01111001 {set char "y"}
      01111010 {set char "z"}
      00110000 {set char "0"}
      00110001 {set char "1"}
      00110010 {set char "2"}
      00110011 {set char "3"}
      00110100 {set char "4"}
      00110101 {set char "5"}
      00110110 {set char "6"}
      00110111 {set char "7"}
      00111000 {set char "8"}
      00111001 {set char "9"}
      00101010 {set char "*"}
      00100000 {set char " "}
      01011111 {set char "_"}
      00101100 {set char ","}
      00101110 {set char "."}
      00111101 {set char "="}
      00101101 {set char "-"}
      00101011 {set char "+"}
      00111010 {set char ":"}
      00111011 {set char ";"}
      00111100 {set char "<"}
      00111110 {set char ">"}
      00100001 {set char "!"}
      00100100 {set char "$"}
      default  {set char "?"}
   }
   set ac $myglobalvar($widget:ac)
   set myglobalvar($widget:data_$ac) $char
   update_ram_address $widget $myglobalvar($widget:direction)
   update_shift_val $widget $myglobalvar($widget:shift)
   update_cursor_pos  $widget $myglobalvar($widget:direction) NOSHIFT
   draw_lcd_display $widget
   if {$myglobalvar($widget:verb) == "1"} {
      echo "written \"$char\" at position:$ac at time:$now"
   }
}

set LD_BITLEN 8
set chdren [winfo children .]
set i [expr 0]
while {[lsearch -exact $chdren .my:lcd_$i] >= 0} {
   incr i
}
mk_lcd_display .my:lcd_$i
