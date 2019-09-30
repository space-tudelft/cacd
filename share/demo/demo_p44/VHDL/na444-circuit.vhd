library IEEE;
use IEEE.std_logic_1164.ALL;

library CellsLib;
architecture circuit of na444 is
   component na210
      port (A:in std_logic;
            B:in std_logic;
            Y:out std_logic);
   end component;
   signal net_0: std_logic;
   signal net_1: std_logic;
   signal net_2: std_logic;
begin
   Y <= net_0; 
   na210_0 : na210 port map (A => A, B => B, Y => net_1);
   na210_1 : na210 port map (A => C, B => D, Y => net_2);
   na210_2 : na210 port map (A => net_1, B => net_2, Y => net_0);
end circuit;

