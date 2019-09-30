
proc df_txt_dialog {label_string init_str} {
#-----------------------------------------------------------------------------#
# procedure to input a string into a program via a pop_up window              #
# two buttons are present: OK and Cancel                                      #
# If Cancel is clicked the dialog will disapear and 'CANCELLED' will be       #
# returned.                                                                   #
# If OK is clicked the string in the entry_window will be returned            #
#-----------------------------------------------------------------------------#
   global tk_txt_dialog_retval Fnt

   set tk_txt_dialog_retval ""

   toplevel .txt_dialog
   frame  .txt_dialog.f1 -bg wheat -relief raised -bd 2
   label  .txt_dialog.f1.lb -text $label_string -bg wheat -font $Fnt
   entry  .txt_dialog.f1.en -width 25 -bg wheat -font $Fnt
   frame  .txt_dialog.f2 -bg gold -relief raised -bd 2
   button .txt_dialog.f2.ok -text "OK" -width 6 -bg gold -font $Fnt -command {
		set tk_txt_dialog_retval [.txt_dialog.f1.en get]
		catch {destroy .txt_dialog}
		}
   button .txt_dialog.f2.cl -text "Cancel" -bg gold -font $Fnt -command {
		set tk_txt_dialog_retval "CANCELLED"
		catch {destroy .txt_dialog}
		}
   pack .txt_dialog.f1    -side top -fill both -expand 1
   pack .txt_dialog.f1.lb -side left -pady 2
   pack .txt_dialog.f1.en -side left -fill x -padx 5 -pady 5 -expand 1
   pack .txt_dialog.f2    -side bottom -fill x
   pack .txt_dialog.f2.ok -side left  -padx 15 -pady 5
   pack .txt_dialog.f2.cl -side right -padx 15 -pady 5

   wm geom  .txt_dialog +[winfo pointerx .]+[winfo pointery .]
   wm title .txt_dialog text_dialog
   .txt_dialog.f1.en insert end $init_str
   focus .txt_dialog.f1.en
   grab set .txt_dialog
   wm attributes .txt_dialog -topmost 1
   tkwait window .txt_dialog
   return $tk_txt_dialog_retval
}

proc df_mess_dialog {label_string} {
#-----------------------------------------------------------------------------#
# procedure to show a message via a pop_up window .                           #
# If OK is clicked the window will disappear.                                 #
#-----------------------------------------------------------------------------#
   global Fnt topmostWindow

   toplevel .mess_dialog
   label  .mess_dialog.lbl -text $label_string -bg wheat -font $Fnt\
		-bd 2 -relief ridge -padx 10 -pady 10 -justify left
   frame  .mess_dialog.fr  -bg gold -bd 2 -relief ridge
   button .mess_dialog.fr.ok -text "OK" -width 6 -bg gold -font $Fnt -command {destroy .mess_dialog}

   pack .mess_dialog.lbl   -side top    -fill both -expand 1
   pack .mess_dialog.fr    -side bottom -fill x
   pack .mess_dialog.fr.ok -side top -padx 15 -pady 5

   wm geom  .mess_dialog +[winfo pointerx .]+[winfo pointery .]
   wm title .mess_dialog Message
   set old_grab [grab current]
   focus .mess_dialog
   grab set .mess_dialog
   if {[winfo exist $topmostWindow]} { wm attributes $topmostWindow -topmost 0 }
   wm attributes .mess_dialog -topmost 1
   tkwait window .mess_dialog
   if {[winfo exist $topmostWindow]} { wm attributes $topmostWindow -topmost 1 }
   if {$old_grab != ""} { grab set $old_grab }
   return 0
}

proc df_mess {label_string} {
#-----------------------------------------------------------------------------#
# procedure to show a message via a pop_up window .                           #
#-----------------------------------------------------------------------------#
   global Fnt

   toplevel .mess
   label  .mess.lbl -text $label_string -bg wheat -font $Fnt\
		-bd 2 -relief ridge -padx 10 -pady 10 -justify left
   frame  .mess.fr  -bg gold -bd 2 -relief ridge
   button .mess.fr.ok -text "Close" -width 6 -bg gold -font $Fnt -command {destroy .mess}

   pack .mess.lbl   -side top    -fill both -expand 1
   pack .mess.fr    -side bottom -fill x
   pack .mess.fr.ok -side top -padx 15 -pady 5

   wm geom  .mess +[winfo pointerx .]+[winfo pointery .]
   wm title .mess Message
}

proc df_large_mess_dialog {label_string} {
   global Fnt

   toplevel  .mess_dialog_large
   text      .mess_dialog_large.txt -width 80 -height 10 -font $Fnt -bg wheat\
				   -yscrollcommand ".mess_dialog_large.sb set"
   scrollbar .mess_dialog_large.sb -command ".mess_dialog_large.txt yview" -bg wheat
   frame  .mess_dialog_large.fr  -bg gold -bd 2 -relief ridge
   button .mess_dialog_large.fr.ok -text "OK" -width 6 -bg gold -font $Fnt\
				   -command {destroy .mess_dialog_large}

   pack .mess_dialog_large.fr    -side bottom -fill x
   pack .mess_dialog_large.fr.ok -side top -padx 15 -pady 5
   pack .mess_dialog_large.sb    -side right  -fill y
   pack .mess_dialog_large.txt   -side right  -fill both -expand 1

   wm title .mess_dialog_large "Message"

   .mess_dialog_large.txt insert end "$label_string\n"
   update
   .mess_dialog_large.txt see end
   focus .mess_dialog_large
   grab set .mess_dialog_large
   tkwait window .mess_dialog_large
}

proc df_choise_dialog {label_string} {
#-----------------------------------------------------------------------------#
# procedure to give a choice for a question via a pop_up window .             #
# The return value will be the answer: yes or no.                             #
#-----------------------------------------------------------------------------#
   global Answer Fnt topmostWindow posX posY posXY

   toplevel .cfg_chs_dialog
   label    .cfg_chs_dialog.lbl    -text $label_string -bg wheat -font $Fnt\
                                  -bd 2 -relief ridge -padx 10 -pady 10 -justify left
   frame    .cfg_chs_dialog.fr     -bg gold -bd 2 -relief ridge
   button   .cfg_chs_dialog.fr.yes -text "Yes" -bg gold -font $Fnt\
		-command { set Answer "yes"; destroy .cfg_chs_dialog }
   button   .cfg_chs_dialog.fr.no -text "No" -width 3 -bg gold -font $Fnt\
		-command { set Answer "no" ; destroy .cfg_chs_dialog }

   pack .cfg_chs_dialog.lbl    -side top    -fill both -expand 1
   pack .cfg_chs_dialog.fr     -side bottom -fill x
   pack .cfg_chs_dialog.fr.yes -side left  -padx 15 -pady 5 -fill x -expand 1
   pack .cfg_chs_dialog.fr.no  -side right -padx 15 -pady 5 -fill x -expand 1

if {![info exist posXY] || !$posXY} {
   set posX [expr [winfo pointerx .]-160]
   set posY [expr [winfo pointery .]-100]
   if {[info exist posXY]} { incr posXY }
}
   wm geom  .cfg_chs_dialog +$posX+$posY
   wm title .cfg_chs_dialog Question
   set old_grab [grab current]
   focus .cfg_chs_dialog
   grab set .cfg_chs_dialog
   if {[winfo exist $topmostWindow]} { wm attributes $topmostWindow -topmost 0 }
   wm attributes .cfg_chs_dialog -topmost 1
   tkwait window .cfg_chs_dialog
   if {[winfo exist $topmostWindow]} { wm attributes $topmostWindow -topmost 1 }
   if {$old_grab != ""} { catch {grab set $old_grab} }
   return $Answer
}

proc conf_choise_dialog {label_string txtwdw conf_list} {
#-----------------------------------------------------------------------------#
# procedure to give a choice for a sub_configuration if more configurations   #
# are possible.  The choice can be made via a radiobutton.                    #
#-----------------------------------------------------------------------------#
   global Fnt ConfChosen

   toplevel .cfg_chs_dialog
   label    .cfg_chs_dialog.lbl   -text $label_string -bg wheat -font $Fnt\
                                  -bd 2 -relief ridge -padx 10 -pady 10 -justify left
   frame    .cfg_chs_dialog.fr    -bg gold -bd 2 -relief ridge
   button   .cfg_chs_dialog.fr.ok -text "OK" -bg gold -font $Fnt\
                                  -command  { destroy .cfg_chs_dialog }
   pack .cfg_chs_dialog.lbl -side top    -fill both -expand yes
   pack .cfg_chs_dialog.fr  -side bottom -fill x

   for {set cfg 0} {$cfg < [llength $conf_list]} {incr cfg} {
      radiobutton .cfg_chs_dialog.rb_$cfg -text [lindex $conf_list $cfg] -font $Fnt\
                                          -variable ConfChosen -value [lindex $conf_list $cfg]\
                                          -bg wheat -anchor w
      pack .cfg_chs_dialog.rb_$cfg -side top -fill x
   }
   pack .cfg_chs_dialog.fr.ok -side left -padx 15 -pady 5 -fill x -expand yes

   set ConfChosen [lindex $conf_list 0]
   wm geom  .cfg_chs_dialog +[winfo x $txtwdw]+[winfo y $txtwdw]
   wm title .cfg_chs_dialog ConfigChoice
   set old_grab [grab current]
   focus .cfg_chs_dialog
   grab set .cfg_chs_dialog
   tkwait window .cfg_chs_dialog
   if {$old_grab != ""} { grab set $old_grab }
   return $ConfChosen
}
