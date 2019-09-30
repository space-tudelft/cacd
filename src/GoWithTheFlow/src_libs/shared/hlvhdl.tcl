
proc highlight_vhdl {win} {

      set keywords1 [list \
         abs\
         access\
         after\
         alias\
         all\
         and\
         architecture\
         array\
         assert\
         attribute\
         begin\
         block\
         body\
         buffer\
         bus\
         case\
         component\
         configuration\
         constant\
         disconnect\
         downto\
         else\
         elsif\
         end\
         entity\
         exit\
         file\
         for\
         function\
         generate\
         generic\
         group\
         guarded\
         if\
         impure\
         in\
         inertial\
         inout\
         is\
         label\
         library\
         linkage\
         literal\
         loop\
         map\
         mod\
         nand\
         new\
         next\
         nor\
         not\
         null\
         of\
         on\
         open\
         or\
         others\
         out\
         package\
         port\
         postponed\
         procedure\
         process\
         pure\
         range\
         record\
         register\
         reject\
         return\
         rol\
         ror\
         select\
         severity\
         signal\
         shared\
         sla	\
         sli\
         sra\
         srl\
         subtype\
         then\
         to\
         transport\
         type\
         unaffected\
         units\
         until\
         use\
         variable\
         wait\
         when\
         while\
         with\
         xnor\
         xor\
                     ]
      set keywordlist ""
      foreach str $keywords1 {
         lappend keywordlist $str
         lappend keywordlist [string toupper $str]
      }
      ctext::addHighlightClass $win keywords blue $keywordlist
      ctext::addHighlightClassForRegexp $win stringval red3 {"[^\n\r"]*"}
      ctext::addHighlightClassForRegexp $win quoteval red3 {'[10]*'}
      ctext::addHighlightClassForRegexp $win comment seagreen {--[^\n\r]*}
}

