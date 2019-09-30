' \" @(#)mux.sls	 3.2	 11/03/88
network mux_netw (terminal mux_in[1..4], mux_out, mux_select[1..2])
{
    {inst_mux} @ multiplexer (mux_in[1..4], mux_out, 
					mux_select[1..2]);
}
