#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

int lookahead;
// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// E -> T R
void E(void) { T(); R(); }

// T -> F Q
void T(void) { F(); Q(); }

// F -> '(' E ')' | DEC | OCT | HEX | FLT | ID
void F(void)
{
	switch(lookahead) {
		case '(':
			match('('); E(); match(')');
			break;
		case DEC:
			match(DEC); break;
		case OCT:
			match(OCT); break;
		case HEX:
			match(HEX); break;
		case FLT:
			match(FLT); break;
		default:
			match(ID);
	}
}

// Q -> '*' F Q | '/' F Q | epsilon
void Q(void)
{
	switch(lookahead) {
		case '*':
			match('*'); F(); Q(); break;
		case '/':
			match('/'); F(); Q(); break;
		default:
			;
	}
}

// R -> '+' T R | '-' T R | epsilon
void R(void)
{
	switch(lookahead) {
		case '+':
			match('+'); T(); R(); break;
		case '-':
			match('-'); T(); R(); break;
		default:
			;
	}
}

//////////////////////////// parser components /////////////////////////////////
void match(int expected)
{
	if (lookahead == expected) {
		lookahead = gettoken(source);
	} else {
		fprintf(stderr, "token mismatch\n");
		exit(ERRTOKEN);
	}
}
