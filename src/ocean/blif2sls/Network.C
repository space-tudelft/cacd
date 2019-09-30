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

#include "src/ocean/blif2sls/Network.h"
#include <ctype.h>
#include <stdlib.h>
#include <fstream>
using namespace std;

Array  Network::eqList(10,10);

Array  Network::specials(10,10);// here we keep the names of special
				// terminals (see findMissing() ).

//----------------------------------------------------------------------------
Network::Network(const String& n,Network* pro)
                              : Array(100,100),
			        proto(pro),
			        name(n),
			        args(10,10)
//
//
{
}// Network::Network  //

//----------------------------------------------------------------------------
Network::~Network()
//
//
{
}// Network::~Network  //

//----------------------------------------------------------------------------
int Network::scanPrototype(istream& is)
//
// This function scans a prototype from the stream "is" (network without
// a body. Returns 1 if something went wrong.
{
  String  externToken("extern"),
          networkToken("network"),
          terminalToken("terminal"),
          nlToken("\n");

  String  token("");

  while(is && scanToken(is,token) == nlToken);

  if(!is) return 0; // probably the end of prototypes

  if(token != externToken)
  {
    cerr << "Error: no extern token" << token << endl;
    return 1;
  }

  if(!is || scanToken(is,token) != networkToken)
  {
    cerr << "Error: no network token" << token << endl;
    return 1;
  }

  name=scanToken(is,token);

  if(!is || scanToken(is,token) != terminalToken)
  {
     cerr << "Error: no terminal token  " << token << endl;
    return 1;
  }

				// now terminals of the network

  while( is && scanToken(is,token) != nlToken)
  {
    args.add( * new String(token) );
  }
  return 0;

}// Network::scanPrototype  //

//----------------------------------------------------------------------------
String& Network::scanToken(istream& is,String& s)
//
// Also skips comments (if between /* */)
{
  char c,buf[200];
  char *ptr=buf, *end=buf+200;

  while(is.get(c) && !(isalnum(c)  || c=='/' || c=='_' || c=='\n'));

  is.putback(c);

  while(is.get(c) && ptr<end
                  &&  (isalnum(c) || c=='/' || c=='*' || c=='_' ||
		       c == '\n' || (ptr==buf && c =='\n'))  )
  {
    *ptr++=c;
    if(c=='\n') break;

    if(ptr[-2]=='/' && c=='*')	// comments skipper
    {
      char prev=' ';
      while(is.get(c) && !(prev == '*' && c == '/'))
	prev=c;

      ptr--;			// restore the buffer
      ptr--;

      if(!is)
      {
	cerr << "Could not find a matching */" << endl;
	break;
      }
      while(is.get(c) && !(isalnum(c)  || c=='/' || c=='_'));
      is.putback(c);
    }

  }
  *ptr=0;
  s=String(buf);

  return s;

}// Network::scanToken  //


//----------------------------------------------------------------------------
void Network::addTerm(String& name,int i)
//
// This function defines a name of a net for one of the network terminals.
// It is used while we scan other networks calls with a network body.
{
  if(i>=0)
    args.addAt(name,i);
  else
    args.add(name);
}// Network::addTermName  //

//----------------------------------------------------------------------------
int Network::getTermNum(const String& s)
//
// This one is used when we want to know what is the number of the formal
// terminal in a network prototype. String "s" must be a name
// of a formal terminal. It returns -1 if there\'s no such terminal.
{
  for (unsigned int i=0; i < args.contains(); i++)
  {
    String& par=(String&)args.getFrom(i);

    if(par == s) return i;
  }
  return -1;

}// Network::getTermNum  //

//----------------------------------------------------------------------------
String&  Network::getName()
//
//
{
  return name;
}// Network::name  //

//----------------------------------------------------------------------------
void Network::printOn(ostream& os)const
//
// Here we dump our sls network onto an output stream "os"
// It will behave differently depending on the fact if it\'s
// a network or a network call.
{
  if(contains())		// this is a network
  {
    os << "network " << name << "( terminal " ;

    Iterator iter(args);

    while(int(iter))
    {
      String& a=(String&)(Object&)iter;
      ++iter;
      os << a << (int(iter) ? ", " : ")\n{\n");
    }
  }
  else				// this is a call to a network
  {
    os << "  " << name  << "( " ;

    for(int i=0;i<numberOfArg();i++)
    {
      String& a=(String&)args.getFrom(i);
      if(a != NOTHING)
      {
	os << a;
      }
      else
      {				// but maybe its one of the missing
				// nets ..
	String& formal = proto->getArg(i);
	if(isSpecial(formal))
	  os << formal;
      }
      os << (i!=numberOfArg()-1 ? ", " : ");\n");
    }
  }

  if(contains())		// this is a network
  {
    Iterator iter(*this);

    int instcnt=0;
    while(int(iter))
    {
      Network& n=(Network&)(Object&)iter;
      ++iter;
      os <<  " {inst" << instcnt++ << "}  ";
      os << n;			// will call the same routine but
				// as a network call.
    }

				// now 2-point nets from equivalence
				// list
    Iterator eIter(eqList);

    while(int(eIter))
    {
      Equivalence &eq=(Equivalence&)(Object&)eIter;
      ++eIter;
      os << "            net{" << eq.s1 << "," << eq.s2 << "};\n";
    }


    os << "}\n";
  }
  if(!os)
  {
    cerr << "Error writing output file ! " << endl;
    exit(1);
  }

}// Network::printOn  //

//----------------------------------------------------------------------------
int Network::numberOfArg(void)const
//
//
{
  if(proto==NULL)		// this is a prototype itself
    return args.contains();
  return proto->numberOfArg();
}// Network::numberOfArg  //


//----------------------------------------------------------------------------
void Network::readSpecials(const char* filename)
//
// Reads defintion of special terminals from file or if name == NULL set it
// to defaults.
{
  if (filename == NULL)
  {
    specials.add( * new String("vdd"));
    specials.add( * new String("R"));
    specials.add( * new String("vss"));
    specials.add( * new String("CK"));
    return;
  }

  ifstream css(filename);

  if(!css)
  {
    cerr << "error opening configuration file \"" << filename << "\"." << endl;
    exit(1);
  }

  while(css)
  {
    String& token = * new String("");
    css >> token;
    specials.add(token);
  }
  if(0)
  {
    cerr << "Setting list of special terminals to: { " ;
    Iterator sIter(specials);
    while(int(sIter))
    {
      String& s= (String&)sIter.get();
      cerr << " " << s;
      ++sIter;
    }
    cerr << " }"<< endl;
  }

}// Network::readSpecials  //


//----------------------------------------------------------------------------
String& Network::getArg(int i)const
//
//
{
  return (String&)args.getFrom(i);
}// Network::getArg  //

//----------------------------------------------------------------------------
void Network::addEquiv(Equivalence &eq)
//
//
{
  eqList.add(eq);
}// Network::applyEqList  //

//----------------------------------------------------------------------------
int Network::isSpecial(const String& s)const
//
// Returns 1 if "s" is a name of the one of the special terminals.
{
  Iterator sIter(specials);

  while(int(sIter))
  {
    String& curr=(String&)sIter.get();
    if(s == curr)
      return 1;
    ++sIter;
  }
  return 0;
}// isSpecial  //

//----------------------------------------------------------------------------
Equivalence::Equivalence(const String& n1,const String& n2):s1(n1),s2(n2)
//
//
{
}// Equivalence::Equivalence  //

//----------------------------------------------------------------------------
void Equivalence::printOn(ostream& os)const
//
//
{
  os << s1 << " == " << s2 ;
}// Equivalence::printOn  //
