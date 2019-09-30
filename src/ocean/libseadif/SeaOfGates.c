/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Bird eye's view on the sea-of-gates program.
 */
sea_of_gates (netlist, library, image, chipsize)
{
    initialize (&chip, library, image, chipsize);

    red (netlist);

    connect_to_pads (&chip);

    report_results (chip);
}

/* The function red() maps an arbitrary network on a sea-of-gates image.
 * It performs the job by first selecting a set of so called `green' blocks
 * from the network and leaving the remaining stuff in a bin `junkbin'.
 * The green blocks show a mutual structure (busses, stacks, ...) while the junk
 * does not seem to be related very strongly to this structure. Now a
 * placement routine plans the location of the green blocks on the chip.
 * It also plans the wires connecting these blocks. At this point we are left
 * with a chip that contains several holes. A partitioning routine then
 * empties the junk bin by assigning its content to these `red' holes.
 * Finally, red() calls for each green block a routine green() so as to map
 * the green blocks on the image, and it recursively calls red() to look
 * after the mapping of red holes on the image. In the case that network does
 * not contain any sensible stucture (greenlist is (nearly) empty, redlist is
 * possibly an exact copy of the junkbin or the input network), the function
 * cut_in_two_pieces() arranges for red() not to call itself infinitely.
 */
red (network)
{
    if (network is entirely in library) { /* maybe at highest or lowest level */
	shuffle_and_connect_terminals (); /* so as to meet the terminal */
	return;				/* requirements of the channel router */
    }

    if (network consists of only few transistors) {
	convert_small_circuit_to_physical_implementation (network);
	return;
    }

    split_in_green_and_junk (&greenlist, &junkbin, network);

    perform_placement_and_wire_planning (&placement, &wireplan, greenlist);

    fill_red_holes (&redlist, junkbin);

    perform_global_routing (&greenlist, &redlist, placement);

    for (all greensubnetworks in the greenlist) green (greensubnetwork);

    for (all redsubnetworks in the redlist) {
	if (too many vertices in this redsubnetwork and it is not in the library)
	{
	    cut_in_two_pieces (&subred1, &subred2, redsubnetwork);
	    unite (red (subred1), red (subred2)); /* recursive calls */
	}
	else
	    red (redsubnetwork); /* recursive call */
    }

    put_all_vertices_together (network);
}

/* The function green() maps a structured network onto a sea-of-gates image.
 */
green (network)
{
    if (network is entirely in library) {
	shuffle_and_connect_terminals (); /* so as to meet the terminal */
	return;				/* requirements of the channel router */
    }

    if (floorplan of network is in library) {
	read_floorplan (&placement, network);	/* may be incomplete */
    }
    else if (too many vertices in this network) { /* may depend on structure */
	cut_in_two_pieces (&subgreen1, &subgreen2, network);
	unite (green (subgreen1), green (subgreen2)); /* recursive calls */
	return;
    }

    if (we can already see the resolution of the image) {
	make_module_close_to_image (&wiremap, placement, network);
    }
    else {
	if (we have no fully specified floorplan from the library)
	    think_of_a_floorplan (&placement, network);
	perform_global_routing (&network, placement);
	for (all vertices in network) green (vertex); /* recursive call */
	put_all_vertices_together (network);
    }
}

/* This function assembles a module from a number of smaller submodules
 * whos physical representation may or may not be present in the library.
 * If a submodule is not present, it is called into existence by
 * convert_small_circuit_to_physical_implementation(). Typically, such a
 * submodule consists of 1...8 transistors. Now that the dimensions of
 * all submodules are known we can think of a placement. This placement
 * is then directly converted to a wiremap by a lee router. Thus, we skip
 * the global routing stage (= terminal assignment to submodules) and the
 * physical implementation view of the resulting module does not contain
 * any more references to submodules.
 */
make_module_close_to_image (wiremap, placement, network)
{
    for (all modules in placement which are not in library)
	convert_small_circuit_to_physical_implementation (module);

    if (placement is empty or partially empty)
	think_of_a_floorplan_close_to_image (&placement, network); /* genetic? */

    stamp_submodules (&wiremap, placement);

    do {
	lee_route (&wiremap, network);
    }
    while (not completed && tries <= maxtries);
}

/* The global routing stage consists of finding for each wire a global path
 * which preferably leads right through the `gaps' in the submodules. Then
 * the netlists of these submodules are updated and terminals are added so
 * that these submodules provide the required feed-through connections.
 */
perform_global_routing (network, placement)
{
    for (all vertices in network) inquire_capacities (vertex);

    for (all nets in network) {
	determine_path_trough_network (&path);
	for (all vertices in this path) extend_netlist_and_termlist (vertex, path);
    }
}
