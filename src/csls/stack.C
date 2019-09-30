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

stack::stack (short size) {
    top = min = new char * [size];
    max = min + size;
    while (top < max) *top++ = 0;
    top = min;
#ifdef DMEM
    stack_nbyte += sizeof(class stack);
    stack_maxnbyte = stack_nbyte > stack_maxnbyte ? stack_nbyte:stack_maxnbyte;
#endif
}

stack::~stack() {
#ifdef DMEM
    stack_nbyte -= sizeof(class stack);
#endif
}

void stack::reset() {
    top = min;
}

int stack::empty() {
    return (top == min);
}

int stack::push(char *elem) {
    if(top < max) {
	*top++ = elem;
	return STACK_OK;
    }
    else
	return STACK_OVERFLOW;
}

char * stack::pop() {
    if(top > min)
	return (*--top);
    else
	return (0);
}

void stack::print() {
	char **pxs;
	printf("StackB\n");
	printf("ptr: %p\tlength: %d\tmin: %p\t max: %p\ttop: %p\n",
		this, (int)(top - min), min, max, top);
	for(pxs = min; pxs < top; pxs++)
	    printf("[%d] ptr: %p\t*ptr: %p\n", (int)(pxs-min), pxs, *pxs);
	printf("StackE\n");
}

char *stack::access(int x) {
	if (top > min)
	{
	    if(x < 0 || x >= (top - min))
		fprintf(stderr,"stack: Invalid index");
	    else
		return (*(min+x));
	}
	return (0);
}

class stack *stack::copy() {
	class stack *pstack = 0;
	char **p;

	if (top > min)
	{
	    pstack = new class stack (top-min);
	    for(p = min; p < top; p++)
		pstack->push(*p);
	}
	return pstack;
}

char **stack::base() {
	return min;
}

char **stack::limit() {
	return max;
}

int stack::length() {
	return (top - min);
}
