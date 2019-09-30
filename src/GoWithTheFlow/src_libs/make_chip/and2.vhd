library IEEE;
use IEEE.std_logic_1164.ALL;
library CellsLib;
use CellsLib.all;

entity and2 is
   port (A: in std_logic;
         B: in std_logic;
         Y: out std_logic);
end and2;

architecture behaviour of and2 is
   component na210
      port (A: in std_logic;
            B: in std_logic;
            Y: out std_logic);
   end component;
   component iv110
      port (A: in std_logic;
            Y: out std_logic);
   end component;
   signal is0: std_logic;
begin
   na210_0: na210 port map (A, B, is0);
   iv110_1: iv110 port map (is0, Y);
end behaviour;

configuration and2_cfg of and2 is
   for behaviour
      for all: na210 use entity CellsLib.na210(dataflow);
      end for;
      for all: iv110 use entity CellsLib.iv110(dataflow);
      end for;
   end for;
end and2_cfg;
