configuration na443_behaviour_cfg of na443 is
   for behaviour
      for all: na447 use configuration work.na447_behaviour_cfg;
      end for;
      for all: na445 use configuration work.na445_circuit_cfg;
      end for;
   end for;
end na443_behaviour_cfg;

