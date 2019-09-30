configuration hotel_tb_behaviour_cfg of hotel_tb is
   for behaviour
      for all: hotel use configuration work.hotel_behaviour_cfg;
      end for;
   end for;
end hotel_tb_behaviour_cfg;

