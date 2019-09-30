library IEEE;
use IEEE.std_logic_1164.ALL;

--use WORK.hotel_pkg.ALL;

entity Hotel is
   port(clk :in    std_logic;
        res :in    std_logic;
        s   :in    std_logic;
        ov  :in    std_logic;
        lamp:out   std_logic);
end hotel;
