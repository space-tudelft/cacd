library IEEE;
use IEEE.std_logic_1164.ALL;

architecture circuit of na446 is
component na445
   port (A:in std_logic;
         B:in std_logic;
         C:in std_logic;
         D:in std_logic;
         Y:out std_logic);
end component;
begin
inst1 : na445 port map (A => A, B => B, C => C, D => D, Y => Y);
end circuit;

