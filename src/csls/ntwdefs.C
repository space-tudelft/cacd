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

ntwdef::ntwdef (char *name) {
    ntw_name = strsav (name);
    inst_gen_cnt = 0;
    net_gen_cnt = 0;
    local = 0;
    termq = NULL;
    orig_termq = NULL;
    mcq = NULL;
    netq = NULL;
#ifdef DMEM
    ntwdef_nbyte += sizeof(class ntwdef);
    if (ntwdef_nbyte > ntwdef_maxnbyte) ntwdef_maxnbyte = ntwdef_nbyte;
#endif
}

ntwdef::~ntwdef () {
#ifdef DMEM
    ntwdef_nbyte -= sizeof(class ntwdef);
#endif
}

char *ntwdef::genname (int tname) {
    char name[20];

    switch(tname)
    {
	case InstanceType: sprintf(name, "_I%x", inst_gen_cnt);
    			   inst_gen_cnt++;
			   break;

	case NetType: sprintf(name, "_N%x", net_gen_cnt);
    		      net_gen_cnt++;
		      break;
    }
    return (strsav (name));
}

ntwinst::ntwinst (Network *pntw, Stack *attr) : lnk (InstanceType)
{
    type = InstanceType;
    ntw = pntw;
    ntw_attr = attr;
    inst_struct = NULL;
#ifdef DMEM
    ntwinst_nbyte += sizeof(class ntwinst);
    if (ntwinst_nbyte > ntwinst_maxnbyte) ntwinst_maxnbyte = ntwinst_nbyte;
#endif
}

ntwinst::~ntwinst()
{
    stackfree(ntw_attr, STRING);
    if(ntw_attr)
        delete ntw_attr;
    delete inst_struct;
#ifdef DMEM
    ntwinst_nbyte -= sizeof(class ntwinst);
#endif
}

void ntwinst::to_db()
{
	Stack *pinst_s = inst_struct->inst_construct;
	char attr_buf[BUFSIZ];
	int i;
	int size = pinst_s ? pinst_s -> length () : 0;
	int top = ntw_attr ? ntw_attr -> length () : 0;

	strcpy(cmc.cell_name, ntw -> ntw_name);
	strcpy(cmc.inst_name, inst_struct->inst_name);
	cmc.imported = ntw -> local ? LOCAL : IMPORTED;

	if (top > 0) {
	    char **pattr = ntw_attr -> base ();
	    char *s, *t = attr_buf;

	    i = 0;
	    do {
		s = *pattr++;
		while (*s) *t++ = *s++;
		*t++ = ';';
	    } while (++i < top);
	    *--t = 0;
	    cmc.inst_attribute = attr_buf;
	}
	else
	    cmc.inst_attribute = 0;

	cmc.inst_dim = size;

	if (size > 0) {
	    char **pxs = pinst_s -> base ();
#ifdef DMEM
    int_nbyte += 2 * size * sizeof(long);
    if (int_nbyte > int_maxnbyte) int_maxnbyte = int_nbyte;
#endif
	    cmc.inst_lower = new long [size];
	    cmc.inst_upper = new long [size];

	    for (i = 0; i < size; ++i) {
		cmc.inst_lower[i] = ((Xelem *) *pxs) -> left_bound;
		cmc.inst_upper[i] = ((Xelem *) *pxs) -> right_bound;
		++pxs;
	    }
	}

	dmPutDesignData(dsp_mc, CIR_MC);

	if (size > 0) {
#ifdef DMEM
    int_nbyte -= 2 * size * sizeof(long);
#endif
	    delete cmc.inst_lower;
	    delete cmc.inst_upper;
	}
}

instancestruct::instancestruct(char *name, Stack *construct) {
    inst_name = name;
    inst_construct = construct;
    termCnt = 0;
    nmem = NULL;
#ifdef DMEM
    inst_struct_nbyte += sizeof(class instancestruct);
    if (inst_struct_nbyte > inst_struct_maxnbyte)
	inst_struct_maxnbyte = inst_struct_nbyte;
#endif
}

instancestruct::~instancestruct() {
    delete inst_name;
    stackfree(inst_construct, XELEM);
    if(inst_construct)
        delete inst_construct;
#ifdef DMEM
    inst_struct_nbyte -= sizeof(class instancestruct);
#endif
}
