-- Copyright (C) 1991-2010 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files from any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, Altera MegaCore Function License 
-- Agreement, or other applicable license agreement, including, 
-- without limitation, that your use is for the sole purpose of 
-- programming logic devices manufactured by Altera and sold by 
-- Altera or its authorized distributors.  Please refer to the 
-- applicable agreement for further details.

-- PROGRAM		"Quartus II"
-- VERSION		"Version 10.0 Build 262 08/18/2010 Service Pack 1 SJ Full Version"
-- CREATED		"Fri Sep 21 11:15:32 2012"

LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY top_de1 IS 
	PORT
	(
		clock_50mhz :  IN  STD_LOGIC;
		vga_hsync :  OUT  STD_LOGIC;
		vga_vsync :  OUT  STD_LOGIC;
		vga_b :  OUT  STD_LOGIC_VECTOR(3 DOWNTO 0);
		vga_g :  OUT  STD_LOGIC_VECTOR(3 DOWNTO 0);
		vga_r :  OUT  STD_LOGIC_VECTOR(3 DOWNTO 0)
	);
END top_de1;

ARCHITECTURE bdf_type OF top_de1 IS 

COMPONENT pre_vga_dac_4
	PORT(r : IN STD_LOGIC;
		 g : IN STD_LOGIC;
		 b : IN STD_LOGIC;
		 vga_b : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
		 vga_g : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
		 vga_r : OUT STD_LOGIC_VECTOR(3 DOWNTO 0)
	);
END COMPONENT;

COMPONENT gen6mhz
	PORT(clk50mhz : IN STD_LOGIC;
		 clk6mhz : OUT STD_LOGIC
	);
END COMPONENT;



BEGIN 



b2v_inst : pre_vga_dac_4
PORT MAP(		 vga_b => vga_b,
		 vga_g => vga_g,
		 vga_r => vga_r);


b2v_inst1 : gen6mhz
PORT MAP(clk50mhz => clock_50mhz);


END bdf_type;