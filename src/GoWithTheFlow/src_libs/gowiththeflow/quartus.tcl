
proc start_quartus {} {
   if {[file exists quartus_DE1/de1.qpf]} {
      exec quartus --64bit quartus_DE1/de1.qpf &
   } elseif {[file exists quartus_DE2/de2.qpf]} {
      exec quartus --64bit quartus_DE2/de2.qpf &
   } else {
      exec quartus --64bit &
   }
}

proc quartus_proj {board} {
   global OPPROGPATH

   restore_cwd

   if {[file exists quartus_$board]} {
      df_mess_dialog "The directory quartus_$board already exists"
      return
   }

   set bl [string tolower $board]

   df_mess "A $board Project will now be created and Qaurtus will be started.
Use 'Project -> Add/Remove Files in Project ...' to add your VHDL files from the
directory ../VHDL to the Quartus project.
Only add VHDL files for entities and for behaviour and structural/circuit architectures.
After that, you can include the top level entity of your design into the entity top_$bl.
You may do the latter by first generating a symbol for your top-level VHDL entity
(using 'File -> Create/Update -> Create Symbol Files for Current File')
and then including this symbol into the schematic for top_$bl (open top_$bl.bdf).
Next, do the following steps:
- Using the Assignment Editor (under the Assign menu) assign the ports of entity
top_$bl to the pins of the FPGA on the board (See the file ${board}_pin_assignments.txt
for finding out how FPGA pins are connected to the differents parts on the board).
- Compile the design using 'Processing -> Start Compilation'.
- Connect the $board board with the USB Blaster connector to your PC.
- Program the board using the programmer under 'Tools -> Programmer'.
Now you can test your design on the board."

   exec cp -R $OPPROGPATH/quartus/quartus_$board .
   exec quartus --64bit quartus_$board/$bl.qpf &
}
