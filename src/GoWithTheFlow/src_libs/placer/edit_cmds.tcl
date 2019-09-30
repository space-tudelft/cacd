
proc move_row_right {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set xl [lindex $mc_arr($CurCel) 0]
      set yt [lindex $mc_arr($CurCel) 1]
      set idx [array startsearch mc_arr]
      while {[array anymore mc_arr $idx] == 1} {
         set cn [array nextelement mc_arr $idx]
         if {([lindex $mc_arr($cn) 0] >= $xl) &&\
             ([lindex $mc_arr($cn) 1] == $yt)} {
            set xln [expr [lindex $mc_arr($cn) 0] + 40]
            set xrn [expr [lindex $mc_arr($cn) 2] + 40]
            set mc_arr($cn) [lreplace $mc_arr($cn) 0 0 $xln]
            set mc_arr($cn) [lreplace $mc_arr($cn) 2 2 $xrn]
            redraw_mc $cn
         }
      }
      array donesearch mc_arr $idx
      if {$Moving == "True"} {
         set MvIdx [after $MoveDelay move_row_right]
      }
   }
}

proc move_row_left {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set xl [lindex $mc_arr($CurCel) 0]
      if {$xl <= 34} {
         stop_moving
         return
      }
      set yt [lindex $mc_arr($CurCel) 1]
      set idx [array startsearch mc_arr]
      while {[array anymore mc_arr $idx] == 1} {
         set cn [array nextelement mc_arr $idx]
         if {([lindex $mc_arr($cn) 0] >= $xl) &&\
             ([lindex $mc_arr($cn) 1] == $yt)} {
            set xln [expr [lindex $mc_arr($cn) 0] - 40]
            set xrn [expr [lindex $mc_arr($cn) 2] - 40]
            set mc_arr($cn) [lreplace $mc_arr($cn) 0 0 $xln]
            set mc_arr($cn) [lreplace $mc_arr($cn) 2 2 $xrn]
            redraw_mc $cn
         }
      }
      array donesearch mc_arr $idx
      if {$Moving == "True"} {
         set MvIdx [after $MoveDelay move_row_left]
      }
   }
}

proc move_row_up {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set xl [lindex $mc_arr($CurCel) 0]
      set yt [lindex $mc_arr($CurCel) 1]
      set idx [array startsearch mc_arr]
      while {[array anymore mc_arr $idx] == 1} {
         set cn [array nextelement mc_arr $idx]
         set hght [expr [lindex $mc_arr($cn) 3]-[lindex $mc_arr($cn) 1]]
         if {([lindex $mc_arr($cn) 0] >= $xl) &&\
             ([lindex $mc_arr($cn) 1] == $yt) && ($hght == 506)} {
            set ytn [expr [lindex $mc_arr($cn) 1] + 494]
            set ybn [expr [lindex $mc_arr($cn) 3] + 494]
            set mc_arr($cn) [lreplace $mc_arr($cn) 1 1 $ytn]
            set mc_arr($cn) [lreplace $mc_arr($cn) 3 3 $ybn]
            redraw_mc $cn
         }
      }
      array donesearch mc_arr $idx
   }
}

proc move_row_down {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set ytn [expr [lindex $mc_arr($CurCel) 1] - 494]
      if {$ytn < 0} {
         return
      }
      set xl [lindex $mc_arr($CurCel) 0]
      set yt [lindex $mc_arr($CurCel) 1]
      set idx [array startsearch mc_arr]
      while {[array anymore mc_arr $idx] == 1} {
         set cn [array nextelement mc_arr $idx]
         set hght [expr [lindex $mc_arr($cn) 3]-[lindex $mc_arr($cn) 1]]
         if {([lindex $mc_arr($cn) 0] >= $xl) &&\
             ([lindex $mc_arr($cn) 1] == $yt) && ($hght == 506)} {
            set ytn [expr [lindex $mc_arr($cn) 1] - 494]
            set ybn [expr [lindex $mc_arr($cn) 3] - 494]
            set mc_arr($cn) [lreplace $mc_arr($cn) 1 1 $ytn]
            set mc_arr($cn) [lreplace $mc_arr($cn) 3 3 $ybn]
            redraw_mc $cn
         }
      }
      array donesearch mc_arr $idx
   }
}

proc move_cell_right {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set xln [expr [lindex $mc_arr($CurCel) 0] + 40]
      set xrn [expr [lindex $mc_arr($CurCel) 2] + 40]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 0 0 $xln]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 2 2 $xrn]
      redraw_mc $CurCel
      if {$Moving == "True"} {
         set MvIdx [after $MoveDelay move_cell_right]
      }
   }
}

proc move_cell_left {} {
   global CurCel mc_arr Moving MvIdx MoveDelay

   if {$CurCel != ""} {
      set xln [expr [lindex $mc_arr($CurCel) 0] - 40]
      if {$xln < 0} {
         stop_moving
         return
      }
      set xrn [expr [lindex $mc_arr($CurCel) 2] - 40]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 0 0 $xln]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 2 2 $xrn]
      redraw_mc $CurCel
      if {$Moving == "True"} {
         set MvIdx [after $MoveDelay move_cell_left]
      }
   }
}

proc move_cell_up {} {
   global CurCel mc_arr scale y0

   if {$CurCel != ""} {
      if {[expr [lindex $mc_arr($CurCel) 3]-\
          [lindex $mc_arr($CurCel) 1]] == 506} {
         set ytn [expr [lindex $mc_arr($CurCel) 1] + 494]
         set ybn [expr [lindex $mc_arr($CurCel) 3] + 494]
      }\
      else {
         set ytn [expr [lindex $mc_arr($CurCel) 1] + 988]
         set ybn [expr [lindex $mc_arr($CurCel) 3] + 988]
      }
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 1 1 $ytn]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 3 3 $ybn]
      redraw_mc $CurCel
   }
}

proc move_cell_down {} {
   global CurCel mc_arr

   if {$CurCel != ""} {
      if {[expr [lindex $mc_arr($CurCel) 3]-\
          [lindex $mc_arr($CurCel) 1]] == 506} {
         set ytn [expr [lindex $mc_arr($CurCel) 1] - 494]
         if {$ytn < 0} {
            return
         }
         set ybn [expr [lindex $mc_arr($CurCel) 3] - 494]
      }\
      else {
         set ytn [expr [lindex $mc_arr($CurCel) 1] - 988]
         if {$ytn < 0} {
            return
         }
         set ybn [expr [lindex $mc_arr($CurCel) 3] - 988]
      }
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 1 1 $ytn]
      set mc_arr($CurCel) [lreplace $mc_arr($CurCel) 3 3 $ybn]
      redraw_mc $CurCel
   }
}

proc stop_moving {} {
   global Moving MvIdx

   set Moving "False"
   after cancel $MvIdx
}
