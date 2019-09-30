library IEEE;
use IEEE.std_logic_1164.ALL;

entity toets is
   port(
        state  : in  Boolean ;
        t_from : in  std_logic ;
        t_to   : out std_logic
       );
end toets;

library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of toets is
   signal bb : Boolean := FALSE;
begin
   bouncer: process
      variable mbt:time := 40 ms;
      variable bpt:time;
      variable tt  : time;
   begin
      wait on state;
      if state'event and state = TRUE then
         bpt := mbt/10;
         tt := 0 ms;
         while (state = TRUE) and (tt < mbt) loop
            bb <= TRUE;
            wait until state = TRUE for bpt;
            tt := tt + bpt;
            bb <= FALSE;
            wait until state = TRUE for bpt;
            tt := tt + bpt;
         end loop;
      end if;
      bb <= state;
   end process;

   ideal_key: process (bb, t_from)
   begin
      if bb = TRUE then
         t_to <= t_from;
      else
         t_to <= 'L';
      end if;
   end process;
end behaviour;

configuration toets_behaviour_cfg of toets is
   for behaviour
   end for;
end toets_behaviour_cfg;

library IEEE;
use IEEE.std_logic_1164.ALL;

entity tbord_cout is
   port(
        row       : in  std_logic_vector(3 downto 0) := "LLLL";
        col       : out std_logic_vector(2 downto 0) := "LLL"
       );
end tbord_cout;

library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of tbord_cout is
   component toets
      port(state : in  Boolean;
           t_from: in  std_logic;
           t_to  : out std_logic := 'L');
   end component;
   signal btn_0, btn_1, btn_2, btn_3, btn_4, btn_5, btn_6, btn_7, btn_8,
          btn_9, btn_s, btn_h:Boolean := False;
begin
   lbl1 : toets port map (btn_1, row(0), col(2)); 
   lbl2 : toets port map (btn_2, row(0), col(1)); 
   lbl3 : toets port map (btn_3, row(0), col(0)); 
   lbl4 : toets port map (btn_4, row(1), col(2)); 
   lbl5 : toets port map (btn_5, row(1), col(1)); 
   lbl6 : toets port map (btn_6, row(1), col(0)); 
   lbl7 : toets port map (btn_7, row(2), col(2)); 
   lbl8 : toets port map (btn_8, row(2), col(1)); 
   lbl9 : toets port map (btn_9, row(2), col(0)); 
   lbls : toets port map (btn_s, row(3), col(2)); 
   lbl0 : toets port map (btn_0, row(3), col(1)); 
   lblh : toets port map (btn_h, row(3), col(0)); 
end behaviour;

library IEEE;
use IEEE.std_logic_1164.ALL;

entity tbord_rout is
   port(
        row       : out std_logic_vector(3 downto 0) := "LLLL";
        col       : in std_logic_vector(2 downto 0) := "LLL"
       );
end tbord_rout;

library IEEE;
use IEEE.std_logic_1164.ALL;

architecture behaviour of tbord_rout is
   component toets
      port(state : in  Boolean;
           t_from: in  std_logic;
           t_to  : out std_logic := 'L');
   end component;
   signal btn_0, btn_1, btn_2, btn_3, btn_4, btn_5, btn_6, btn_7, btn_8,
          btn_9, btn_s, btn_h:Boolean := False;
begin
   lbl1 : toets port map (btn_1, col(2), row(0)); 
   lbl2 : toets port map (btn_2, col(1), row(0)); 
   lbl3 : toets port map (btn_3, col(0), row(0)); 
   lbl4 : toets port map (btn_4, col(2), row(1)); 
   lbl5 : toets port map (btn_5, col(1), row(1)); 
   lbl6 : toets port map (btn_6, col(0), row(1)); 
   lbl7 : toets port map (btn_7, col(2), row(2)); 
   lbl8 : toets port map (btn_8, col(1), row(2)); 
   lbl9 : toets port map (btn_9, col(0), row(2)); 
   lbls : toets port map (btn_s, col(2), row(3)); 
   lbl0 : toets port map (btn_0, col(1), row(3)); 
   lblh : toets port map (btn_h, col(0), row(3)); 
end behaviour;









