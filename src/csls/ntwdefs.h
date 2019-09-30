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

class netMember;

class ntwdef {
	int inst_gen_cnt;
	int net_gen_cnt;
public:
	ntwdef (char *);
	~ntwdef (void);
	char *genname (int);
	char *ntw_name;
	int local;
	Queue *termq;
	Queue *orig_termq;
	Queue *mcq;
	Queue *netq;
};

class instancestruct {
public:
	instancestruct (char *, Stack *);
	~instancestruct (void);
	char *inst_name;
	Stack *inst_construct;
	int termCnt;
	class netMember *nmem;
};

class ntwinst : public lnk {
public:
	ntwinst (class ntwdef *, Stack *);
	~ntwinst (void);
	class ntwdef *ntw;
	Stack *ntw_attr;
	class instancestruct *inst_struct;
	void to_db(void);
};

typedef class ntwdef Network;
typedef class ntwinst NetworkInstance;
typedef class instancestruct InstanceStruct;
