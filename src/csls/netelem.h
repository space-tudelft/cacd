/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#define  N_NET       0
#define  N_TERMINAL  1
#define  N_GLOBADDED 2

#define T_global   (N_TERMINAL | N_GLOBADDED)
#define T_input    (N_TERMINAL | 0x10)
#define T_output   (N_TERMINAL | 0x20)
#define T_inout    (N_TERMINAL | 0x30)
#define T_tristate (N_TERMINAL | 0x40)

class simpleNet {
public:
	class netMember	*nmem;
	class simpleNet *next;
	class simpleNet *prev;
};

class netelem;

class netMember {
public:
	NetworkInstance *inst;
	class netelem   *net;
	class netMember *next;
	class simpleNet *snet;
};

class netelem : public lnk {
public:
			netelem (char *, class stack *, char);
			~netelem (void);
	char 		*name;
	int             type;
	Stack		*xs;
	Queue		*eqv;
	class netMember	*nmem;
	void		print();
	void		to_db();
};

class net_ref : public lnk {
public:
			net_ref (class netelem *, class stack *);
			~net_ref (void);
	class netelem	*net;
	Stack		*net_xs;
	NetworkInstance *inst;
	Stack		*inst_xs;
	Stack		*ref_xs;
};

typedef class netMember Nmem;
