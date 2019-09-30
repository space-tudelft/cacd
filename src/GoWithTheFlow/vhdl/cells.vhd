------------------------------------------------------------------------------
-- file with the vhdl_descriptions of the cells used in the                 --
-- 'ontwerppracticum'.                                                      --
------------------------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY mir_nin IS
   PORT(I: INOUT STD_LOGIC;
        G: OUT   STD_LOGIC);
END mir_nin;

ARCHITECTURE dataflow OF mir_nin IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY mir_pin IS
   PORT(I: INOUT STD_LOGIC;
        G: OUT    STD_LOGIC);
END mir_pin;

ARCHITECTURE dataflow OF mir_pin IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY mir_nout IS
   PORT(O: INOUT STD_LOGIC;
        I: IN    STD_LOGIC;
        G: IN    STD_LOGIC);
END mir_nout;

ARCHITECTURE dataflow OF mir_nout IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY mir_pout IS
   PORT(O: INOUT STD_LOGIC;
        I: IN    STD_LOGIC;
        G: IN    STD_LOGIC);
END mir_pout;

ARCHITECTURE dataflow OF mir_pout IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY ln3x3 IS
   PORT(G: IN    STD_LOGIC;
        D: INOUT STD_LOGIC;
        S: IN    STD_LOGIC);
END ln3x3;

ARCHITECTURE dataflow OF ln3x3 IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY lp3x3 IS
   PORT(G: IN    STD_LOGIC;
        D: INOUT STD_LOGIC;
        S: IN    STD_LOGIC);
END lp3x3;

ARCHITECTURE dataflow OF lp3x3 IS
BEGIN
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY osc10 IS
   PORT(e: IN     STD_LOGIC;
	f: OUT    STD_LOGIC;
        xi: IN    STD_LOGIC;
        xo: INOUT STD_LOGIC := 'Z');
END osc10;

ARCHITECTURE dataflow OF osc10 IS
BEGIN
   lbl1:
   process(e, xo, xi)
   begin
      if (e /= '1') then
         f <= '1';
      elsif (xo = 'Z') then
         f <= xi;
      else
         f <= NOT xo;
      end if;
   end process;
END dataflow;

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
ENTITY iv110 IS
  PORT (A: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END iv110;

ARCHITECTURE dataflow OF iv110 IS
BEGIN
  Y <= NOT(A) after 1.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY no210 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END no210;

ARCHITECTURE dataflow OF no210 IS
BEGIN
  Y <= A NOR B after 1.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY no310 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        C: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END no310;

ARCHITECTURE dataflow OF no310 IS
BEGIN
  Y <= NOT(A OR B OR C) after 2.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY na210 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END na210;

ARCHITECTURE dataflow OF na210 IS
BEGIN
  Y <= A NAND B after 1.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY na310 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        C: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END na310;

ARCHITECTURE dataflow OF na310 IS
BEGIN
  Y <= NOT(A AND B AND C) after 2.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY ex210 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END ex210;

ARCHITECTURE dataflow OF ex210 IS
BEGIN
  Y <= A XOR B after 2.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY buf40 IS
  PORT (A: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END buf40;

ARCHITECTURE dataflow OF buf40 IS
BEGIN
  Y <= A after 2.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY tbuf10 IS
  PORT (A: IN STD_LOGIC;
        E: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END tbuf10;

ARCHITECTURE dataflow OF tbuf10 IS
BEGIN
  Y <= A after 2.5 ns when (E = '1') else 'Z' after 2.5 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY tinv10 IS
  PORT (A: IN STD_LOGIC;
        E: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END tinv10;

ARCHITECTURE dataflow OF tinv10 IS
BEGIN
  Y <= not A after 1.5 ns when (E = '1') else 'Z' after 1.5 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY mu111 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        S: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END mu111;

ARCHITECTURE dataflow OF mu111 IS
BEGIN
  Y <= (NOT(S) AND A) OR (S AND B) after 2.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY mu210 IS
  PORT (S1:IN STD_LOGIC;
        S2:IN STD_LOGIC;
        A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        C: IN STD_LOGIC;
        D: IN STD_LOGIC;
        Y: OUT STD_LOGIC);
END mu210;

ARCHITECTURE dataflow OF mu210 IS
BEGIN
  Y <= (NOT(S1) AND NOT(S2) AND A) OR
       (    S1  AND NOT(S2) AND B) OR
       (NOT(S1) AND     S2  AND C) OR
       (    S1  AND     S2  And D) after 4.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY de211 IS
  PORT (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        Y0: OUT STD_LOGIC;
        Y1: OUT STD_LOGIC;
        Y2: OUT STD_LOGIC;
        Y3: OUT STD_LOGIC);
END de211;

ARCHITECTURE dataflow OF de211 IS
BEGIN
  Y0 <= NOT(A) AND NOT(B) after 4.0 ns;
  Y1 <= A      AND NOT(B) after 4.0 ns;
  Y2 <= NOT(A) AND B      after 4.0 ns;
  Y3 <= A      AND B      after 4.0 ns;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY dfn10 IS
  PORT (D: IN STD_LOGIC;
        CK: IN STD_LOGIC;
        Q: OUT STD_LOGIC);
END dfn10;

ARCHITECTURE dataflow OF dfn10 IS
  SIGNAL Q_INT: std_logic;
BEGIN
  Q <= Q_INT;
  PROCESS(CK, D) 
  BEGIN 
    IF(CK'event AND CK='1') THEN
      -- pragma translate_off
      assert D'quiet(2.0 ns)
      report "setup_violation"
      severity warning; 
      -- pragma translate_on
      Q_INT <= D after 4.0 ns;
    END IF;
    -- pragma translate_off
    IF (D'event AND CK = '1' AND (D = Q_INT)) THEN 
      assert CK'quiet(1.0 ns)
      report "hold_violation"
      severity warning; 
    END IF; 
    -- pragma translate_on
  END PROCESS;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY dfr11 IS
  PORT (D: IN STD_LOGIC;
        R: IN STD_LOGIC;
        CK: IN STD_LOGIC;
        Q: OUT STD_LOGIC);
END dfr11;

ARCHITECTURE dataflow OF dfr11 IS
  SIGNAL Q_INT: std_logic;
BEGIN
  Q <= Q_INT;
  PROCESS(CK, D, R) 
  BEGIN 
    IF(CK'event AND CK='1') THEN 
      -- pragma translate_off
      assert D'quiet(2.0 ns)
      report "setup_violation of D"
      severity warning; 
      -- pragma translate_on
      IF (R='1') THEN 
        -- pragma translate_off
        assert R'quiet(2.0 ns)
        report "setup_violation of R"
        severity warning; 
        -- pragma translate_on
	Q_INT <= '0' after 4.0 ns; 
      ELSE 
	Q_INT <= D after 4.0 ns; 
      END IF;
    END IF;
    -- pragma translate_off
    IF (D'event AND CK = '1' AND (D = Q_INT)) THEN
      assert CK'quiet(1.0 ns)
      report "hold_violation of D"
      severity warning; 
    END IF;
    IF (R'event AND R = '0' AND CK = '1' AND (Q_INT /= '0')) THEN 
      assert CK'quiet(1.0 ns)
      report "hold_violation of R"
      severity warning; 
    END IF;
    -- pragma translate_on
  END PROCESS;
END dataflow;


LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;

ENTITY dfa11 IS
  PORT (D: IN STD_LOGIC;
        R: IN STD_LOGIC;
        CK: IN STD_LOGIC;
        Q: OUT STD_LOGIC);
END dfa11;

ARCHITECTURE dataflow OF dfa11 IS
  SIGNAL Q_INT: std_logic;
BEGIN
  Q <= Q_INT;
  PROCESS(CK, D, R) 
  BEGIN 
    IF (R='1') THEN 
      Q_INT <= '0' after 4.0 ns; 
    ELSIF(CK'event AND CK='1') THEN 
      -- pragma translate_off
      assert D'quiet(2.0 ns)
      report "setup_violation of D"
      severity warning; 
      -- pragma translate_on
      Q_INT <= D after 4.0 ns; 
    END IF;
    -- pragma translate_off
    IF (D'event AND CK = '1' AND (D = Q_INT)) THEN
      assert CK'quiet(1.0 ns)
      report "hold_violation of D"
      severity warning; 
    END IF;
    -- pragma translate_on
  END PROCESS;
END dataflow;




