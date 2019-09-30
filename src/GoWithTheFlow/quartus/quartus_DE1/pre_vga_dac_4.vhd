library ieee;
use ieee.std_logic_1164.all; 
   
entity pre_vga_dac_4 is 
   port (
	r   :  in  std_logic;
	g   :  in  std_logic;
	b   :  in  std_logic;
	vga_r :  out  std_logic_vector(3 downto 0);
	vga_g :  out  std_logic_vector(3 downto 0);
	vga_b :  out  std_logic_vector(3 downto 0)
   );
end pre_vga_dac_4;
   
architecture signal_flow of pre_vga_dac_4 is 
   
begin
    
   vga_r(0) <= r;
   vga_r(1) <= r;
   vga_r(2) <= r;
   vga_r(3) <= r;
   
   vga_g(0) <= g;
   vga_g(1) <= g;
   vga_g(2) <= g;
   vga_g(3) <= g;
   
   vga_b(0) <= b;
   vga_b(1) <= b;
   vga_b(2) <= b;
   vga_b(3) <= b;
   
end; 
