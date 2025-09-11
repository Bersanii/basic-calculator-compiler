#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

// /**/ ação semantica /**/

int lookahead; // Token atual (lookahead) usado pelo parser

// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// Gramática:
// E -> [Ominus] T { Oplus T }
// Oplus = ['+''-']
// Ominus = ['+''-']
void E(void) { 

	/**/int isNegate = 0; /**/		// Marca se deve aplicar negação
	/**/int isOtimes = 0; /**/		// Armazena operador multiplicativo ('*' ou '/')
	/**/int isOplus = 0; /**/		// Armazena operador aditivo ('+' ou '-')

	// Trata opcional (+ ou -) antes do termo
	if(lookahead == '+' || lookahead == '-'){
		if (lookahead == '-') {
			isNegate = lookahead; // Se for '-', guarda para aplicar negação depois
		}
		match(lookahead); // Consome o operador
	}

	// Início do termo (T)
	_Tbegin:

	// Início do fator (F)
	_Fbegin:

	switch(lookahead) {
		case '(': // Expressão entre parênteses
			match('('); E(); match(')');
			break;
		case DEC: // Número decimal
			/**/printf(" %s ", lexeme);/**/
			match(DEC); 
			break;
		case OCT: // Número octal
			/**/printf(" %s ", lexeme);/**/
			match(OCT); 
			break;
		case HEX: // Número hexadecimal
			/**/printf(" %s ", lexeme);/**/
			match(HEX); 
			break;
		case FLT: // Número ponto flutuante
			/**/printf(" %s ", lexeme);/**/
			match(FLT); 
			break;
		default: // Identificador (variável)
			/**/printf(" %s ", lexeme);/**/
			match(ID);
	}

	// Término do fator

	/**/
	if(isOtimes){ // Se havia operador multiplicativo pendente, imprime
		printf(" %c ", isOtimes);
		isOtimes = 0;
	}
	/**/

	// Se próximo token for '*' ou '/', continua reconhecendo fator
	if (lookahead == '*' || lookahead == '/') {
		/**/isOtimes = lookahead;/**/ // Guarda operador multiplicativo
		match(lookahead); 
		goto _Fbegin; // Volta para reconhecer novo fator
	}

	// Término do termo
	
	/**/
	if (isNegate) { // Se havia sinal negativo, aplica
		printf(" negate ");
		isNegate = 0;
	}
	/**/

	/**/
	if(isOplus) { // Se havia operador aditivo pendente, imprime
		printf(" %c ", isOplus);
		isOplus = 0;
	}
	/**/

	// Se próximo token for '+' ou '-', continua reconhecendo termo
	if (lookahead == '+' || lookahead == '-') {
		/**/isOplus = lookahead; /**/ // Guarda operador aditivo
		match(lookahead); 
		goto _Tbegin; // Volta para reconhecer novo termo
	}

	// Término da expressão
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
