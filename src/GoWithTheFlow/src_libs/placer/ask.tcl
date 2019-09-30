
package require Tcl 8.4
package require getstring
namespace import getstring::*

proc ask_write_name {} {
   global WriteCellName

   return [tk_getString .getlcellname WriteCellName "Enter cell name:"]
}

