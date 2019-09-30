/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Xander Burgerhout
 *	Simon de Graaf
 *	N.P. van der Meijs
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
/******************************************************************************
 *  Name        : parse.y
 *
 *  Purpose     : file parser
 *
 *  Author      : Xander Burgerhout
 *
 *  Date        : Jul 05, 2000
 ******************************************************************************/
%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>

using namespace std;

#include "src/spock/src/datastruct/Keywords.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/ComponentTree.h"
#include "src/spock/src/datastruct/Generator.h"
#include "src/spock/src/parser/Parser.h"

#include <qobject.h>

void RemoveStringQuotes(string* str);

// yacc stuff
int yyerror (const char *s);
int nr_of_yyerrors = 0;
//int yylineno = 1;
extern char* yytext;
extern int yylex();
extern int skip_generators;

CParser* thePT = CParser::instance();

string tmp_context = "";

%}

%start spec_descr

%expect 1

%union {
    long    ival;
    char    *sval;
    char    cval;
    long    *pival;
    float   fval;
    double  dval;

    string* strval;
    int     intval;
    CComponent* compval;
}

%token DEFINE GENERATOR GENERATE MAP FOREACH IF ELSE ICONPATH
%token MEMBER LCB RCB AND OR EQ EQEQ NOTEQ NOT LT GT LPS RPS LSB RSB COMMA
%token COLON SCOLON PLUS MINUS M_INFINITY P_INFINITY APPLICATION VERSION DOLLAR

%token <strval> COMPONENT_TYPE PROPERTY IDENTIFIER STRING INTEGER DOUBLE

%type <strval> a_string  string_list a_number ident property_arg context_ident
%type <strval> ident_list member_stmt context_ext operator condition_type cond_type
%type <strval> iter_ident minus_inf plus_inf

%type <compval> expression_ident expression_arg format_stmt format_list if_stat
%type <compval> condition condition_stmt foreach_decl foreach_head condition_arg if_key else_key

%%
spec_descr          : top_decl_list
                    {
                        //debug("spec_descr");
                        //thePT->dumpGlobals();
                        //thePT->dumpStack();
                        //thePT->dumpGenerators();
                    }
                    | error
                    { yyerror("Unexpected end of file"); }
                    ;

top_decl_list       : any_decl
                    | top_decl_list any_decl
		    ;

any_decl            : define_decl
                    {
                        //debug("any_decl define_decl");
                    }
                    | component_decl
                    {
                        //debug("any_decl component_decl");
                    }
                    | generator_decl
                    {
                        //debug("any_decl generator_decl");
                    }
                    | iconpath_decl
                    ;

iconpath_decl       : ICONPATH EQ STRING
                    {
                        //debug("iconpath: %s", $3->c_str());
                        string path = *$3;
                        string::size_type pos, start;
                        pos = path.find("$(");
                        if (pos != string::npos) {
                            start = pos+2;
                            pos = path.find(")");
                            string env = path.substr(start, pos-start);
                            //debug("Extracted [%s]", env.c_str());
                            char* envpath = getenv(env.c_str());
                            if (envpath == 0 || envpath == NULL)
                                env = "./";
                            else
                                env = envpath;
                            //debug("to be replaced with [%s]", env.c_str());
                            path.replace(start-2, pos+1, env);
                        }
                        //debug("iconpath is set to [%s]", path.c_str());
                        thePT->setIconPath(path);
                    }
		    ;

define_decl         : DEFINE component_keyword ident LCB component_list RCB
                    {
                        //debug("define_decl");

                        thePT->setLineNumber();
                        //if (thePT->currentComp() == 0)
                        //    debug("Current component = 0!!");
                        thePT->currentComp()->setName(*$3);
                        thePT->currentComp()->setProperty(PROP_DEFINE, *$3);
                        thePT->addDefine(thePT->pop());
                    }
                    ;


component_decl      : component_keyword ident LCB component_list RCB
                    {
                        thePT->testComponent(*$2);
                        thePT->setLineNumber();
                        thePT->currentComp()->setName(*$2);
                        thePT->addComponent(thePT->pop());
                    }
                    | component_keyword ident LCB RCB
                    {
                        thePT->testComponent(*$2);
                        thePT->setLineNumber();
                        thePT->currentComp()->setName(*$2);
                        thePT->addComponent(thePT->pop());
                    }
                    | component_keyword context_ident SCOLON
                    {
                        //debug("Popping superfluous component...");
                        CComponent* del = thePT->pop(); // Don't need new comp.
                        if (del != 0) {
                            thePT->currentComp()->removeChild(del);
                            delete del;
                        }
                        //debug("Deleted superfluous component...");

                        string fullName = thePT->disambiguate(*$2);
                        //debug("Looking for %s", fullName.c_str());
                        CComponent* child = thePT->findComponent(*$2);
                        if (child != 0) {
                            //debug("Found the right child, name = %s, type = %s", child->getName().c_str(), child->getType().c_str());
                            //debug("Current component is a %s, adding child...", thePT->currentComp()->getType().c_str());
                            thePT->currentComp()->add(child);
                        } //else
                            //debug("Component not found!");

                        //debug("Done setting %s as child of %s.", $2->c_str(), thePT->currentComp()->getName().c_str());
                    }
                    ;

component_list      : component_stmt
                    | component_list component_stmt
                    ;

component_stmt      : component_decl            // excludes the usertype
                    | property_decl
                    | error RCB
                    { yyerror("Expected a component or property declaration."); }
                    ;

component_keyword   : COMPONENT_TYPE
                    {
                        thePT->newComp();
                        thePT->currentComp()->setType(*$1);
                        //debug("   %s", $1->c_str());
                    }
                    | COMPONENT_TYPE range_decl
                    {
                        if (*$1 != "real" && *$1 != "integer")
                            yyerror("A range can only be appended to 'real' and 'integer' types.");
                        //debug("   %s[range]", $1->c_str());
                        thePT->newComp();
                        thePT->currentComp()->setType(*$1);

                        vector<string> tmp = thePT->getPropertyList();
			if (tmp[0] != "" || tmp[1] != "")
			    thePT->currentComp()->setPropertyVector("range", tmp);
                        thePT->clearPropertyList();
                    }
                    | ident
                    {
                        //debug("    User defined type: '%s'", $1->c_str());
                        CComponent* comp = thePT->createFromDefine(*$1, thePT->currentComp());
                        thePT->push(comp);
                    }
                    | error ident
                    {
                        yyerror("Unknown type specified");
                    }
                    ;

property_decl       : PROPERTY LPS property_list RPS SCOLON
                    {
                        vector<string> tmp = thePT->getPropertyList();
                        if (*$1 == PROP_PIXMAP) {
                            debug("Prepending icon path...");
                            if (tmp.size()>0)
                                tmp[0] = thePT->getIconPath() + "/" + tmp[0];
                            debug("file will now be [%s]", tmp[0].c_str());
                        }
                        thePT->currentComp()->setPropertyVector(*$1, tmp);
                        thePT->clearPropertyList();
                        //debug("        property %s = %s", $1->c_str(), $3->c_str());
                        //thePT->currentComp()->setProperty(*$1, *$3);
                    }
                    | error SCOLON
                    {
                        yyerror("Syntax error in property statement");
                    }
                    ;

property_list       : property_arg
                    {
                        thePT->addToPropertyList(*$1);
                    }
                    | property_list COMMA property_arg
                    {
                        thePT->addToPropertyList(*$3);
                    }
		    ;
property_arg	    : a_string
                    | context_ident
                    ;

range_decl          : LSB range_item RSB
                    | error
                    {
                        yyerror("Syntax error in range specification");
                    }
                    ;
range_item          : minus_inf COLON plus_inf
                    {
                        thePT->addToPropertyList(*$1);
                        thePT->addToPropertyList(*$3);
                    }
                    | a_number
                    {
                        thePT->addToPropertyList(*$1);
                        thePT->addToPropertyList("");
                    }
                    ;
minus_inf	    : M_INFINITY { $$ = new string(""); }
                    | a_number
		    | { $$ = new string(""); }
                    ;
plus_inf	    : P_INFINITY { $$ = new string(""); }
                    | a_number
		    | { $$ = new string(""); }
                    ;

generator_decl      : generator_keyword ident LCB generator_list RCB
                    {
                        //debug(" generator %s", $4->c_str());
                        thePT->currentComp()->setName(*$2);
                        CComponent* comp = thePT->pop();
                        CGeneratorComp* gen = dynamic_cast<CGeneratorComp*>(comp);
                        if (gen != 0) // which should always be the case here...
                            thePT->addGeneratorComp(gen);
                    }
                    ;

generator_keyword   : GENERATOR
                    {
                        CGeneratorComp* comp = new CGeneratorComp(thePT->currentComp());
                        comp->setType(GEN_GENERATOR);
                        thePT->push(comp);
                    }
                    ;
generator_list      : generator_stmt
                    | generator_list generator_stmt
                    ;
generator_stmt      : generate_decl
                    | property_decl
                    | map_decl
                    ;

map_decl            : map_start LCB map_list RCB
                    {
                        CComponent* comp = thePT->currentComp();
                        CGeneratorComp* gen = dynamic_cast<CGeneratorComp*>(comp);
                        if (gen != 0)
                            gen->addMapping(thePT->getValueMap());
                        thePT->newValueMap();
                    }
                    ;
map_start           : MAP ident
                    {
                        string context = thePT->disambiguate(*$2);
                        thePT->getValueMap()->setContext(context);
                    }
                    | error LCB
                    { yyerror ("Syntax error"); }
                    ;
map_list            : map_stmt
                    | map_list map_stmt
                    ;
map_stmt            : ident EQ a_string SCOLON
                    {
                        string tmp = thePT->getValueMap()->getContext() + "." + *$1;
                        string context = thePT->disambiguate(tmp);
                        CComponent* comp = thePT->findComponent(context);
                        if (comp != 0) {
                            thePT->getValueMap()->makeMap(comp->getProperty(PROP_TITLE), *$3);
                            //debug("    map %s = %s", $1->c_str(), $3->c_str());
                        }
                    }
                    | error RCB
                    {
                        yyerror("Syntax error");
                    }
                    | error SCOLON
                    {
                        yyerror("Syntax error");
                    }
                    ;

generate_decl       : GENERATE LCB format_list RCB
                    {
                        debug("      generate {}");
                    }
                    ;
format_list         : format_stmt
                    | format_list format_stmt
                    ;
format_stmt         : a_string
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_LITERAL);
                        thePT->currentComp()->setProperty(GEN_VALUE, *$1);
                        thePT->pop();
                        //debug("   format: %s", $1->c_str());
                    }
                    | foreach_decl
                    {
			$$ = $1; // all actions done in foreach rule actions
                    }
                    | expression_ident
                    {
			$$ = $1; // all actions done in expression_ident rule...
                    }
                    | iter_ident
                    {
                        string tmp = thePT->getMappedIterator(*$1);
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_IDENTIFIER);
                        thePT->currentComp()->setName(tmp);
                        thePT->currentComp()->setProperty(GEN_FIELD, GEN_VALUE);
                        thePT->currentComp()->setProperty(PROP_ITERIDENT, *$1);
                        thePT->pop();
                    }
                    | expression_arg operator expression_arg
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(*$2);
                        thePT->currentComp()->add($1);
                        thePT->currentComp()->add($3);
                        thePT->pop();
                    }
                    | APPLICATION LSB ident RSB
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_APPLICATION);
                        thePT->currentComp()->setProperty(GEN_FIELD, *$3);
                        thePT->pop();
                    }
                    | if_stat
                    {
                        // all actions done in if_stat
                    }
                    ;

foreach_decl        : foreach_head foreach_body
                    {
                        $$ = $1;
                        thePT->unmapForEachIterator($1->getProperty(PROP_ITERIDENT));
                        tmp_context = "";
                        thePT->pop();
                    }
                    ;
foreach_head        : foreach_key LPS context_ext COMMA ident RPS
                    {
                        //string context = thePT->disambiguate(*$3);
                        $$ = thePT->currentComp();
                        thePT->currentComp()->setProperty(GEN_CONTEXT, *$3);
                        thePT->currentComp()->setProperty(GEN_ITERATOR, *$5);
                        //debug("   foreach: %s", $3->c_str());

                    }
                    | foreach_key iter_ident LPS context_ext COMMA ident RPS
                    {
                        //string context = thePT->disambiguate(*$4);
                        $$ = thePT->currentComp();
                        thePT->mapForEachIterator(*$2, *$6);
                        thePT->currentComp()->setProperty(GEN_CONTEXT, *$4);
                        thePT->currentComp()->setProperty(GEN_ITERATOR, *$6);
                        thePT->currentComp()->setProperty(PROP_ITERIDENT, *$2);
                        //debug("   foreach: %s", $4->c_str());
                    }
                    ;
foreach_body        : LCB format_list RCB
                    ;

iter_ident          : DOLLAR ident
                    { $$ = $2; }
                    ;

if_key              : IF
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_IF);
                    }
		    ;
else_key            : ELSE
                    {
                        $$ = thePT->newComp();
                        // thePT->currentComp()->setType("else");
                    }
		    ;
if_stat             : if_key LPS condition RPS LCB format_list RCB
                    {
                        $$ = $1;
                        // thePT->currentComp()->add($3);
                        thePT->pop();
                    }
		| if_key LPS condition RPS LCB format_list RCB else_key LCB format_list RCB
		{
                        $$ = $1;
                        $3->add($8);
                        thePT->pop();
                        thePT->pop();
		}
		;

cond_type	: AND { $$ = new string("&&"); }
		| OR  { $$ = new string("||"); }
		;
condition	: condition_stmt
		{
			$$ = $1;
		}
		| condition_stmt cond_type condition_stmt
		{
			$$ = thePT->newComp();
			thePT->currentComp()->setType(KEY_IFCONDITION);
			thePT->currentComp()->setName(*$2);
			thePT->currentComp()->add($1);
			thePT->currentComp()->add($3);
			thePT->pop();
		}
		;
condition_type      : NOTEQ { $$ = new string("!="); }
                    | EQEQ  { $$ = new string("=="); }
                    ;
condition_stmt      : condition_arg condition_type condition_arg
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(KEY_IFCONDITION);
                        thePT->currentComp()->setName(*$2);
                        thePT->currentComp()->add($1);
                        thePT->currentComp()->add($3);
                        thePT->pop();
                    }
		| condition_arg
		{
			$$ = thePT->newComp();
			thePT->currentComp()->setType(KEY_IFCONDITION);
			thePT->currentComp()->setName("!=");
			thePT->currentComp()->add($1);
			CComponent* xx = thePT->newComp();
			thePT->currentComp()->setType(GEN_LITERAL);
			thePT->currentComp()->setProperty(GEN_VALUE, "");
			thePT->pop();
			thePT->currentComp()->add(xx);
			thePT->pop();
		}
		| NOT condition_arg
		{
			$$ = thePT->newComp();
			thePT->currentComp()->setType(KEY_IFCONDITION);
			thePT->currentComp()->setName("==");
			thePT->currentComp()->add($2);
			CComponent* xx = thePT->newComp();
			thePT->currentComp()->setType(GEN_LITERAL);
			thePT->currentComp()->setProperty(GEN_VALUE, "");
			thePT->pop();
			thePT->currentComp()->add(xx);
			thePT->pop();
		}
		;

condition_arg       : context_ident
		    {
			string tmp = thePT->disambiguate(*$1, tmp_context);
			$$ = thePT->newComp();
			thePT->currentComp()->setType(GEN_IDENTIFIER);
			thePT->currentComp()->setName(tmp);
			thePT->currentComp()->setProperty(GEN_FIELD, GEN_VALUE);
			thePT->pop();
		    }
		    | context_ident LSB ident RSB
		    {
			string tmp = thePT->disambiguate(*$1, tmp_context);
			$$ = thePT->newComp();
			thePT->currentComp()->setType(GEN_IDENTIFIER);
			thePT->currentComp()->setName(tmp);
			thePT->currentComp()->setProperty(GEN_FIELD, *$3);
			thePT->pop();
		    }
                    | iter_ident
                    {
                        string tmp = thePT->getMappedIterator(*$1);
                        tmp = thePT->disambiguate(tmp, tmp_context);
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_IDENTIFIER);
                        thePT->currentComp()->setName(tmp);
                        thePT->currentComp()->setProperty(GEN_FIELD, GEN_VALUE);
                        thePT->currentComp()->setProperty(PROP_ITERIDENT, *$1);
                        thePT->pop();
                    }
                    | a_string
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_LITERAL);
                        thePT->currentComp()->setProperty(GEN_VALUE, *$1);
                        thePT->pop();
                    }
                    ;

operator            : PLUS
                    { $$ = new string("+"); }
                    | MINUS
                    { $$ = new string("-"); }
                    ;

expression_arg      : expression_ident
                    | a_number
                    {
                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_INTEGER);
                        thePT->currentComp()->setProperty(GEN_VALUE, *$1);
                        thePT->currentComp()->setProperty(GEN_FIELD, GEN_VALUE);
                        thePT->pop();
                    }
                    ;

expression_ident    : context_ident
                    {
                        //debug("!!!!!!!!!!>>> Disambigauting");
                        string* context = new string (thePT->disambiguate(*$1, tmp_context));
                        //debug("   format: %s", context->c_str());

                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_IDENTIFIER);
                        thePT->currentComp()->setName(*context);
                        thePT->currentComp()->setProperty(GEN_FIELD, GEN_VALUE);
                        thePT->pop();
                    }
                    | context_ident LSB ident RSB
                    {
                        //debug("!!!!!!!!!!>>> Disambiguting");
                        string* context = new string (thePT->disambiguate(*$1, tmp_context));
                        //debug("   format: %s, application property %s", context->c_str(), $3->c_str());

                        $$ = thePT->newComp();
                        thePT->currentComp()->setType(GEN_IDENTIFIER);
                        thePT->currentComp()->setName(*context);
                        thePT->currentComp()->setProperty(GEN_FIELD, *$3);
                        thePT->pop();
                    }
                  ;

foreach_key         : FOREACH
                    {
                        //debug("foreach keyword");
                        thePT->newComp();
                        thePT->currentComp()->setType(GEN_FOREACH);
                    }
                    ;

context_ext	: context_ident
		{
			$$ = new string(thePT->disambiguate(*$1));
			tmp_context = *$$;
		}
		;

context_ident       : ident
                    | ident_list
                    ;

ident_list	    : ident member_stmt
                    { $$ = new string(*$1 + "." + *$2); }
                    ;

member_stmt         : MEMBER ident
                    { $$ = $2; }
                    | member_stmt MEMBER ident
                    { $$ = new string(*$1 + "." + *$3); }
                    ;

ident               : IDENTIFIER
                    {
                        //printf("Ident = [%s]\n",$1->c_str());
                        $$ = new string(*$1);
                    }
                    ;

string_list         : STRING
                    {
                        $$ = new string(*$1);
                    }
                    | string_list STRING
                    {
                        $$ = new string(*$1 + *$2);
                    }
                    ;

a_string            : string_list
                    {
                        //RemoveStringQuotes($1);
                        //$$ = new string(*$1);
                        $$ = $1;
                    }
		    ;

a_number            : INTEGER
                    {
                        $$ = new string(*$1);
                    }
                    | DOUBLE
                    {
                        $$ = new string(*$1);
                    }
                    ;
%%

#include <stdio.h>
#include "lex.h"

int yyerror(const char *s)
{
    ostringstream tmp;

    if (nr_of_yyerrors == 0) {
	tmp << "Errors found in the user interface generation file:" << endl;
	tmp << "---------------------------------------------------" << endl;
    }

    tmp << "line " << yylineno << ": " << s << endl << ends;
    thePT->parseError(tmp.str());

    nr_of_yyerrors++;
    if (nr_of_yyerrors > 3) thePT->displayErrorAndExit();
    return 0;
}

void RemoveStringQuotes(string* str)
{
    *str = str->substr(1, str->length()-2);
}
