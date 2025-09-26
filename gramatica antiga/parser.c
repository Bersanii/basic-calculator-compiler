#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

// /**/ ação semantica /**/

int lookahead; // Token atual (lookahead) usado pelo parser

// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// E -> [Ominus] T { Oplus T }
// Oplus = ['+''-']
// Ominus = ['+''-']
void E(void) { 
	/**/int isOminus = 0;/**/
	/**/int isOplus = 0;/**/

	/**/
	if(lookahead == '+' || lookahead == '-') {
		if (lookahead == '-') {
			isOminus = lookahead;
			match(lookahead);
		}
	}
	/**/
	
	T();
	
	/**/fprintf(objcode, " %c ", isOminus);/**/
	/**/isOminus = 0;/**/

	while (lookahead == '+' || lookahead == '-') {
		/**/isOplus = lookahead;/**/

		match(lookahead); 
		
		T();

		/**/fprintf(objcode, " %c ", isOplus);/**/
		/**/isOplus = 0;/**/
	}

}

// T -> F { Otimes F }
// Otimes = ['*''/']
void T(void) { 
	/**/int isOtimes = 0;/**/

	F(); 
	while (lookahead == '*' || lookahead == '/') {
		/**/isOtimes = lookahead;/**/

		match(lookahead);
		
		F();

		/**/fprintf(objcode, " %c ", isOtimes);/**/
		/**/isOtimes = 0;/**/
	}
}

// F -> '(' E ')' | DEC | OCT | HEX | FLT | ID
void F(void)
{
	switch(lookahead) {
		case '(':
			match('('); E(); match(')');
			break;
		case DEC:
			/**/fprintf(objcode, " %s ", lexeme);/**/
			match(DEC); break;
		case OCT:
			/**/fprintf(objcode, " %s ", lexeme);/**/
			match(OCT); break;
		case HEX:
			/**/fprintf(objcode, " %s ", lexeme);/**/
			match(HEX); break;
		case FLT:
			/**/fprintf(objcode, " %s ", lexeme);/**/
			match(FLT); break;
		default:
			/**/fprintf(objcode, " %s ", lexeme);/**/
			match(ID);
	}
}


//////////////////////////// parser components /////////////////////////////////
void match(int expected)
{
	if (lookahead == expected) {
		// Se o token atual é o esperado, consome e avança para o próximo
		lookahead = gettoken(source);
	} else {
		// Caso contrário, erro de análise
		fprintf(stderr, "token mismatch at line %d \n", lineno);
		// TODO: Não retornar erro, continuar a analise
		exit(ERRTOKEN);
	}
}
