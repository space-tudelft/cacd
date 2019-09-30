
library ieee;
use ieee.std_logic_1164.all;

package cellslib_decl_pack is
component iv110
   port (
          A : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component buf40
   port (
          A : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component tbuf10 
  port (
          A : IN  STD_LOGIC := 'Z';
          E : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component tinv10 
  port (
          A : IN  STD_LOGIC := 'Z';
          E : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component na210
   port (
          A : IN  STD_LOGIC := 'Z';
          B : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component na310
   port (
          A : IN  STD_LOGIC := 'Z';
          B : IN  STD_LOGIC := 'Z';
          C : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component no210
   port (
          A : IN  STD_LOGIC := 'Z';
          B : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component no310
   port (
          A : IN  STD_LOGIC := 'Z';
          B : IN  STD_LOGIC := 'Z';
          C : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component ex210
   PORT (
          A : IN  STD_LOGIC;
          B : IN  STD_LOGIC;
          Y : OUT STD_LOGIC
        );
END component;

component dfn10
   port (
          D  : IN  STD_LOGIC := 'Z';
          CK : IN  STD_LOGIC := 'Z';
          Q  : OUT STD_LOGIC := 'Z'
        );
end component;

component dfr11
   port (
          D  : IN  STD_LOGIC := 'Z';
          R  : IN  STD_LOGIC := 'Z';
          CK : IN  STD_LOGIC := 'Z';
          Q  : OUT STD_LOGIC := 'Z'
        );
end component;

component dfa11
   port (
          D  : IN  STD_LOGIC := 'Z';
          R  : IN  STD_LOGIC := 'Z';
          CK : IN  STD_LOGIC := 'Z';
          Q  : OUT STD_LOGIC := 'Z'
        );
end component;

component mu111
   port (
          A : IN  STD_LOGIC := 'Z';
          B : IN  STD_LOGIC := 'Z';
          S : IN  STD_LOGIC := 'Z';
          Y : OUT STD_LOGIC := 'Z'
        );
end component;

component mu210 
   port (
          S1 : IN  STD_LOGIC;
          S2 : IN  STD_LOGIC;
          A  : IN  STD_LOGIC;
          B  : IN  STD_LOGIC;
          C  : IN  STD_LOGIC;
          D  : IN  STD_LOGIC;
          Y  : OUT STD_LOGIC
        );
END component;

component de211 
  port (A: IN STD_LOGIC;
        B: IN STD_LOGIC;
        Y0: OUT STD_LOGIC;
        Y1: OUT STD_LOGIC;
        Y2: OUT STD_LOGIC;
        Y3: OUT STD_LOGIC);
END component;

component osc10
   port (
          e  : IN    STD_LOGIC;
          f  : OUT   STD_LOGIC;
          xi : IN    STD_LOGIC;
          xo : INOUT STD_LOGIC
        );
end component;

component ln3x3
   port (
          G : IN    STD_LOGIC;
          D : INOUT STD_LOGIC;
          S : INOUT STD_LOGIC
        );
end component;
    
component lp3x3
   port (
          G : IN    STD_LOGIC;
          D : INOUT STD_LOGIC;
          S : INOUT STD_LOGIC
        );
end component;
    
component mir_nin
   port (
          I : INOUT STD_LOGIC;
          G : OUT   STD_LOGIC
        );
end component;

component mir_pin
   port (
          I : INOUT STD_LOGIC;
          G : OUT   STD_LOGIC
        );
end component;

component mir_nout
   port (
          I : IN    STD_LOGIC;
          G : IN    STD_LOGIC;
          O : INOUT STD_LOGIC
        );
end component;

component mir_pout
   port (
          I : IN    STD_LOGIC;
          G : IN    STD_LOGIC;
          O : INOUT STD_LOGIC
        );
end component;

end cellslib_decl_pack;


