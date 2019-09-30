library ieee;
use ieee.std_logic_1164.all; 
   
entity pre_vga_dac is 
   port (
	clk :  in  std_logic;
	r   :  in  std_logic;
	g   :  in  std_logic;
	b   :  in  std_logic;
	vga_sync  :  out  std_logic;
	vga_clk   :  out  std_logic;
	vga_blank :  out  std_logic;
	vga_r :  out  std_logic_vector(9 downto 0);
	vga_g :  out  std_logic_vector(9 downto 0);
	vga_b :  out  std_logic_vector(9 downto 0)
   );
end pre_vga_dac;
   
architecture signal_flow of pre_vga_dac is 
   
begin
    
   vga_sync <= '0';
   vga_clk <= clk;
   vga_blank <= '1';
   
   vga_r(0) <= r;
   vga_r(1) <= r;
   vga_r(2) <= r;
   vga_r(3) <= r;
   vga_r(4) <= r;
   vga_r(5) <= r;
   vga_r(6) <= r;
   vga_r(7) <= r;
   vga_r(8) <= r;
   vga_r(9) <= r;
   
   vga_g(0) <= g;
   vga_g(1) <= g;
   vga_g(2) <= g;
   vga_g(3) <= g;
   vga_g(4) <= g;
   vga_g(5) <= g;
   vga_g(6) <= g;
   vga_g(7) <= g;
   vga_g(8) <= g;
   vga_g(9) <= g;
   
   vga_b(0) <= b;
   vga_b(1) <= b;
   vga_b(2) <= b;
   vga_b(3) <= b;
   vga_b(4) <= b;
   vga_b(5) <= b;
   vga_b(6) <= b;
   vga_b(7) <= b;
   vga_b(8) <= b;
   vga_b(9) <= b;
   
end; 
