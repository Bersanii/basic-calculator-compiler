#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

// /**/ ação semantica /**/

int lookahead; // Token atual (lookahead) usado pelo parser

// Interpretador de comando
//
// mybc -> cmd { cmdsep cmd } EOF
// cmd -> E | exit | quit | <epsilon>
// cmdsep -> ';' | '\n'

void mybc(void){
	cmd();

	while (lookahead != EOF) {
		// cmdsep
		if(lookahead == ';' || lookahead == '\n'){
			match(lookahead);
		}

		cmd();
	}

	match(EOF);
}

void cmd(void) {
	switch (lookahead) {
		case EXIT:
		case QUIT:
			exit(0);
			break;
		// First(E)
		case '+':
		case '-':
		case '(':
		case DEC:
		case FLT:
		case HEX:
		case OCT:
		case ID:
			E();
			printf("%lg \n", acc);
			break;

		default:
			;
	}
}

double acc;
#define STACKSIZE 1024
double stack[STACKSIZE];
int sp = -1;

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
			/*1*/acc = atoi(lexeme);/**/
			match(DEC); 
			break;
		case OCT: // Número octal
			/*2*/acc = strtol(lexeme, NULL, 8); /**/ 
			match(OCT); 
			break;
		case HEX: // Número hexadecimal
			/*3*/acc = strtol(lexeme, NULL, 16); /**/
			match(HEX); 
			break;
		case FLT: // Número ponto flutuante
			/*4*/acc = atof(lexeme);/**/
			match(FLT); 
			break;
		default: // Identificador (variável)
			// /*5*/fprintf(objcode, " %s ", lexeme);/**/
			match(ID);
	}

	// Término do fator

	/**/
	if(isOtimes){ // Se havia operador multiplicativo pendente
		// fprintf(objcode, " %c ", isOtimes);
		if(isOtimes == '*') {
			stack[sp] = stack[sp] * acc;
		} else {
			stack[sp] = stack[sp] / acc;
		}
		acc = stack[sp]; sp--;

		isOtimes = 0;
	}
	/**/

	// Se próximo token for '*' ou '/', continua reconhecendo fator
	if (lookahead == '*' || lookahead == '/') {
		/*10*/isOtimes = lookahead;/**/ // Guarda operador multiplicativo
		/*10a*/stack[++sp] = acc;/**/
		match(lookahead); 
		goto _Fbegin; // Volta para reconhecer novo fator
	}

	// Término do termo
	
	/**/
	if (isNegate) { // Se havia sinal negativo, aplica
		// fprintf(objcode, " negate ");
		acc = -acc;
		isNegate = 0;
	}
	/**/

	/**/
	if(isOplus) { // Se havia operador aditivo pendente
		// fprintf(objcode, " %c ", isOplus);
		if(isOplus == '+') {
			stack[sp] = stack[sp] + acc;
		} else {
			stack[sp] = stack[sp] - acc;
		}
		acc = stack[sp]; sp--;
		isOplus = 0;
	}
	/**/

	// Se próximo token for '+' ou '-', continua reconhecendo termo
	if (lookahead == '+' || lookahead == '-') {
		/**/isOplus = lookahead; /**/ // Guarda operador aditivo
		/*10a*/stack[++sp] = acc;/**/
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
		fprintf(stderr, "Token mismatch at line %d column %d. ", lineno, colno);
		
		char *typename = getEnumName(expected); // Função retorna uma string com o nome do identificar caso esteja mapeado, caso contrário, retorna vazio
		if (typename != "") {
			fprintf(stderr, "Expected %s got ", typename);
		} else {
			fprintf(stderr, "Expected %d got ", expected);
		}

		switch (lookahead) {
			case '\n': 
				fprintf(stderr, "\\n");
				break;
			case '\t': 
				fprintf(stderr, "\\t");
				break;
			case '\r': 
				fprintf(stderr, "\\r");
				break;
			default:
				fprintf(stderr, "%c", lookahead);
				break;
		}

		fprintf(stderr, "\n");
		 
		// TODO: Não retornar erro, continuar a analise
		exit(ERRTOKEN);
	}
}
