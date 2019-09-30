library IEEE;
use IEEE.std_logic_1164.ALL;

architecture Behaviour of hotel is
	type lamp_state is (OFF0, OFF1, ON0, ON1);
	signal state, new_state: lamp_state;
begin
lbl1:	process (clk)
	begin
		if (clk'event and clk = '1') then
			if res = '1' then
				state <= OFF0;
			else
				state <= new_state;
			end if;
		end if;
	end process;
lbl2:	process (state, s, ov)
	begin
		case state is
		when OFF0 =>
			lamp <= '0';
			if (s = '1') and (ov = '0') then
				new_state <= ON1;
			else
				new_state <= OFF0;
			end if;
		when ON1 =>
			lamp <= '1';
			if (s = '0') and (ov = '0') then
				new_state <= ON0;
			else
				new_state <= ON1;
			end if;
		when ON0 =>
			lamp <= '1';
			if (s = '1') and (ov = '0') then
				new_state <= OFF1;
			else
				new_state <= ON0;
			end if;
		when OFF1 =>
			lamp <= '0';
			if (s = '0') and (ov = '0') then
				new_state <= OFF0;
			else
				new_state <= OFF1;
			end if;
		end case;
	end process;
end behaviour;
