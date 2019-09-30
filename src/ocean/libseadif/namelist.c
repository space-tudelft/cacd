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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sea_decl.h"
#include "src/ocean/libseadif/namelist.h"
#include <string.h>

void tonamelist (NAMELISTPTR *namelist, char *name)
{
   NAMELISTPTR l;

   NewNamelist (l);
   l->name = name;
   l->next = *namelist;
   *namelist = l;
}

void fromnamelist (NAMELISTPTR *namelist, char *name)
{
   NAMELISTPTR l1, l2;

   for (l1 = l2 = *namelist; l1; l1 = (l2 = l1)->next)
      if (l1->name == name) break;

   if (l1) {
      if (l1 == l2)
         *namelist = l1->next;
      else
         l2->next = l1->next;
      FreeNamelist (l1);
   }
}

int isinnamelist (NAMELISTPTR namelist, char *name)
{
   NAMELISTPTR l;

   for (l = namelist; l; l = l->next)
      if (l->name == name) return 1;
   return 0;
}

int isemptynamelist (NAMELISTPTR namelist)
{
   return !namelist ? 1 : 0;
}

void tonamelistlist (NAMELISTLISTPTR *namelistlist, NAMELISTPTR namelist, char *namelistname)
{
   NAMELISTLISTPTR p;

   /* check to see that namelist is not already in the namelistlist */
   for (p = *namelistlist; p; p = p->next)
      if (p->namelistname == namelistname) return;

   NewNamelistlist (p);
   p->namelistname = namelistname;
   p->next = *namelistlist;
   p->namelist = namelist;
   *namelistlist = p;
}

/* convert a path of the form "str1:str2:str3" to a namelist containing
 * the strings (str1, str2, str3 str4)
 */
void sdfpathtonamelist (NAMELISTPTR *nl, char *path)
{
   char *name, *endofname;

   *nl = NULL;
   if (!path) return;

   do {
      while (*path == ':') ++path; /* skip surplus of ':' characters */
      name = path;			  /* set name to the start of the name */
      endofname = strchr (path, ':');	  /* set endofname to the next occurenc of ':' */
      if (endofname) {
         *endofname = '\0';		  /* make ':' end of string */
         path = endofname + 1;		  /* set path to start of next name */
      }
      if (*name) tonamelist (nl, abscanonicpath (name)); /* add name to namelist */
   }
   while (endofname);
}

void tonum2list (NUM2LISTPTR *num2list, long num1, long num2)
{
   NUM2LISTPTR l;

   NewNum2list (l);
   l->num1 = num1;
   l->num2 = num2;
   l->next = *num2list;
   *num2list = l;
}

void fromnum2list (NUM2LISTPTR *num2list, long num1, long num2)
{
   NUM2LISTPTR l1, l2;

   for (l1 = l2 = *num2list; l1; l1 = (l2 = l1)->next)
      if (l1->num1 == num1 && l1->num2 == num2) break;

   if (l1) {
      if (l1 == l2)
         *num2list = l1->next;
      else
         l2->next = l1->next;
      FreeNum2list (l1);
   }
}

int isinnum2list (NUM2LISTPTR num2list, long num1, long num2)
{
   NUM2LISTPTR l;

   for (l = num2list; l; l = l->next)
      if (l->num1 == num1 && l->num2 == num2) return 1;
   return 0;
}

int isemptynum2list (NUM2LISTPTR num2list)
{
   return !num2list ? 1 : 0;
}

void tonum2listlist (NUM2LISTLISTPTR *num2listlist, NUM2LISTPTR num2list, char *num2listname)
{
   NUM2LISTLISTPTR p;

   /* check to see that num2list is not already in the num2listlist */
   for (p = *num2listlist; p; p = p->next)
      if (p->num2listname == num2listname) return;

   NewNum2listlist (p);
   p->num2listname = num2listname;
   p->next = *num2listlist;
   p->num2list = num2list;
   *num2listlist = p;
}
