library IEEE;
use IEEE.std_logic_1164.ALL;
use WORK.param_def.ALL;

PACKAGE param_def2 IS
constant N2 : integer;
END param_def2;

PACKAGE BODY param_def2 IS
constant N2 : integer := N + N;
END param_def2;

