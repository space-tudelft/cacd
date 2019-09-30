library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of hotel_tb is
	component hotel
		port (	clk : in  std_logic;
			res : in  std_logic;
			s   : in  std_logic;
			ov  : in  std_logic;
			lamp: out std_logic);
	end component;

	signal clk: std_logic;
	signal res: std_logic;
	signal s  : std_logic;
	signal ov : std_logic;
	signal lamp:std_logic;
begin
lbl1:	hotel port map (clk, res, s, ov, lamp);

	clk <=  '0' after 0 ns,
		'1' after 100 ns when clk /= '1' else '0' after 100 ns;
	res <=  '1' after 0 ns,
		'0' after 200 ns;
	s <=	'0' after 0 ns,
		'1' after 600 ns,
		'0' after 1000 ns,
		'1' after 1400 ns,
		'0' after 1800 ns,
		'1' after 2200 ns,
		'0' after 2600 ns,
		'1' after 3000 ns,
		'0' after 3400 ns,
		'1' after 3800 ns;
	ov <=	'0' after 0 ns,
		'1' after 1800 ns,
		'0' after 2600 ns;
end behaviour;


