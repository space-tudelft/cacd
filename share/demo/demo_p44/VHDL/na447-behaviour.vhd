library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of na447 is
 component na210
      port (A:in std_logic;
            B:in std_logic;
            Y:out std_logic);
   end component;
 component na448
      port (A:in std_logic;
            B:in std_logic;
            Y:out std_logic);
   end component;
begin
inst1: na210 port map( a => a, b => b, y => y );
inst2: na448 port map( a => a, b => b, y => y );
end behaviour;

