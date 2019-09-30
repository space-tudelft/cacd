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

lnk::lnk (int qt)
{
	next = NULL;
	type = qt;
}

queue::queue (int qtype) : lnk (qtype)
{
	tail = 0;
	n = 0;

#ifdef DMEM
	queue_nbyte += sizeof(class queue);
	queue_maxnbyte = queue_nbyte > queue_maxnbyte ? queue_nbyte : queue_maxnbyte;
#endif
}

queue::~queue ()
{
#ifdef DMEM
	queue_nbyte -= sizeof(class queue);
#endif
}

void queue::reset(void)
{
	n = 0;
}

short queue::length(void)
{
	return n;
}

void queue::put(class lnk *p)
{
	if(n++) {
		p->next = tail->next;
		tail->next = p;
	}
	else
		p->next = p;
	tail = p;
}

class lnk *queue::get()
{
	class lnk *p;

	if(n--) {
		p = tail->next;
		tail->next = p->next;
   		return p;
	}
	else {
		n = 0;
		return 0;
	}
}

void queue::print() {
	class lnk *cur = tail->next;
	int i = 0;

	printf("QueueB\n");
	printf("ptr: %p\tn: %d\ttail: %p\t", this, n, tail);

	if(tail)
	{
	    do {
		//printf("[%d] ptr: %p\t*ptr: %p\n", i, cur, *cur);
		printf("[%d] ptr: %p\n", i, cur);

		switch(cur -> type) {
	    	  case QueueType : printf("type: QueueType\n");
			  	((Queue *) cur)->print();
			  	break;
	    	  case NetType   : printf("type: NetType\n");
			  	((Netelem *) cur)->print();
			  	break;
	    	  case TermType   : printf("type: TermType\n");
			  	((Netelem *) cur)->print();
			  	break;
		}

		cur = cur->next;
		i++;
	    } while (cur != tail->next);
	}
	printf("QueueE\n");
}

int queue::empty(void)
{
	return (n==0);
}

class lnk *queue::access(int x)
{
	class lnk *p = tail->next;
	int i;

	for(i=0; i<x; i++)
		p = p->next;
	return p;
}

class lnk *queue::first_elem() {
	return tail->next;
}

class lnk *queue::next_elem(class lnk *cur_elem) {
	return cur_elem->next;
}

class lnk *queue::last_elem() {
	return tail;
}

void queue::append(class queue *pq)
{
	while(!pq -> empty())
	    put(pq -> get());
	delete pq;
}
