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

class lnk {
public:
		    lnk (int);
	class lnk   *next;
	char	    type;
};

class queue : public lnk
{
	class lnk       *tail;
	short		n;
public:
			queue (int);
			~queue ();
	void		reset(void);
	class lnk	*get(void);
	void	     	put(class lnk *);
	short		length(void);
	void		print(void);
	int		empty(void);
	class lnk	*access(int);
	class lnk	*first_elem(void);
	class lnk	*next_elem(class lnk *);
	class lnk	*last_elem(void);
	void 		append(class queue *);
};
