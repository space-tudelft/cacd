/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
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
/*
 * Network - represents one sls network
 */

#ifndef  __NETWORK_H
#define  __NETWORK_H

#include "src/ocean/libocean/Array.h"
#include "src/ocean/libocean/String.h"

#define   NetworkDesc            __FirstUserClassDesc__
#define   EquivalenceDesc        NetworkDesc+1

// this class represents two names of
// nodes in a network that are equivalent

class Equivalence : public Object
{
public:
  Equivalence(const String&,const String&);
 ~Equivalence(){}

  virtual classType       desc() const {return EquivalenceDesc;}
  virtual const   char*   className()const {return "Equivalence";}

  virtual Boolean         isEqual(const Object& ob)const
                             { return Boolean(s1 == ((Equivalence&)ob).s1); }

  virtual Object*         copy()const {return new Equivalence(s1,s2);}

  virtual void            printOn(ostream& strm =cout)const;

  virtual unsigned        hash()const { return 0; }

  String   s1;
  String   s2;

};

// This class keeps  a sls network: name,
// * name of the network (name)
// * terminals of the network (args)
// * calls to other networks (itself) (might be empty for a network prototype)

class Network: public Array
{
public:
  Network(const String& name,Network* proto=NULL);
 ~Network();

  virtual classType       desc() const {return NetworkDesc;}
  virtual const   char*   className()const {return "Network";}

  int                     scanPrototype(istream& is);
  String&                 scanToken(istream& is,String& s);
  void                    addTerm(String&,int i=-1);
  int                     getTermNum(const String& s);
  String&                 getName();
  void                    printOn(ostream& os)const;
  int                     numberOfArg(void)const;
  String&                 getArg(int i)const;
  Network&                getProto()const {return *proto;}
  void                    addEquiv(Equivalence &eq);
  void                    readSpecials(const char* fn);
  int                     isSpecial(const String& s)const;

private:

  Network*           proto;
  String             name;
  Array              args;
  static Array       eqList;

  static Array       specials;

};

#endif
