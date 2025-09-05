#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

// /**/ ação semantica /**/

int lookahead;
// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// E -> [Ominus] T { Oplus T }
// Oplus = ['+''-']
// Ominus = ['+''-']
void E(void) { 

	/**/int isNegate = 0; /**/
	/**/int isOtimes = 0; /**/
	/**/int isOplus = 0; /**/

	if(lookahead == '+' || lookahead == '-'){
		if (lookahead == '-') {
			isNegate = lookahead;
		}
		match(lookahead);	
	}

	_Tbegin:

	_Fbegin:

	switch(lookahead) {
		case '(':
			match('('); E(); match(')');
			break;
		case DEC:
			/**/printf(" %s ", lexeme);/**/
			match(DEC); 
			break;
		case OCT:
			/**/printf(" %s ", lexeme);/**/
			match(OCT); 
			break;
		case HEX:
			/**/printf(" %s ", lexeme);/**/
			match(HEX); 
			break;
		case FLT:
			/**/printf(" %s ", lexeme);/**/
			match(FLT); 
			break;
		default:
			/**/printf(" %s ", lexeme);/**/
			match(ID);
	}

	// Término do fator

	/**/
	if(isOtimes){
		printf(" %c ", isOtimes);
		isOtimes = 0;
	}
	/**/

	if (lookahead == '*' || lookahead == '/') {
		/**/isOtimes = lookahead;/**/
		match(lookahead); goto _Fbegin;
	}

	// Término do termo
	
	/**/
	if (isNegate){
		printf(" negate ");
		isNegate = 0;
	}
	/**/

	/**/
	if(isOplus){
		printf(" %c ", isOplus);
		isOplus = 0;
	}
	/**/

	if (lookahead == '+' || lookahead == '-') {
		/**/isOplus = lookahead; /**/
		match(lookahead); goto _Tbegin;
	}

	// Término da expressão
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
