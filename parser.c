#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

int lookahead;
// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// E -> [Ominus] T { Oplus T }
// Oplus = ['+''-']
// Ominus = ['+''-']
void E(void) { 
	if(lookahead == '+' || lookahead == '-'){
		match(lookahead);	
	}

	_Tbegin:

	_Fbegin:

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

	if (lookahead == '*' || lookahead == '/') {
		match(lookahead); goto _Fbegin;
	}

	if (lookahead == '+' || lookahead == '-') {
		match(lookahead); goto _Tbegin;
	}
}

// T -> F { Otimes F }
// Otimes = ['*''/']
// void T(void) { 
// 	_Fbegin:
// 	F(); 
// 	if (lookahead == '*' || lookahead == '/') {
// 		match(lookahead); goto _Fbegin;
// 	}
// }

// F -> '(' E ')' | DEC | OCT | HEX | FLT | ID
// void F(void)
// {
// 	switch(lookahead) {
// 		case '(':
// 			match('('); E(); match(')');
// 			break;
// 		case DEC:
// 			match(DEC); break;
// 		case OCT:
// 			match(OCT); break;
// 		case HEX:
// 			match(HEX); break;
// 		case FLT:
// 			match(FLT); break;
// 		default:
// 			match(ID);
// 	}
// }

//////////////////////////// parser components /////////////////////////////////
void match(int expected)
{
	if (lookahead == expected) {
		lookahead = gettoken(source);
	} else {
		fprintf(stderr, "token mismatch at line %d \n", lineno);
		// TODO: Não retornar erro, continuar a analise
		exit(ERRTOKEN);
	}
}
