library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of na443 is
component na447
	port ( a, b : in std_logic; y : out std_logic );
end component;
component na445
	port ( a, b,c,d : in std_logic; y : out std_logic );
end component;
signal y1, y2 : std_logic;
begin
inst1:  na447 port map ( a => a, b => b, y => y1 );
inst2:  na447 port map ( a => a, b => b, y => y2 );
inst3:  na445 port map ( a => a, b => b, c => y1, d => y2, y => y );
end behaviour;

