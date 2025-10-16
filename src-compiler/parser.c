#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "tokens.h"
#include "keywords.h"
#include "parser.h"

// /**/ ação semantica /**/

int lookahead; // Token atual (lookahead) usado pelo parser

// Tabela de símbolos como dicionário dos valores armazenados na memória virtual
#define MAXSTENTRIES 4096
char symtab[MAXSTENTRIES][MAXIDLEN+1];
int symtab_next_entry = 0; // uso: strcpy(symtab[symtab_next_entry], name);

// E é o símbolo inicial da gramática LL(1) de expressões simplificadas
// Gramática:
// E -> [Ominus] T { Oplus T }
// Oplus = ['+''-']
// Ominus = ['+''-']
void E(void) { 

	/*0*/char varname[MAXIDLEN+1]/**/;
	/*1*/int isNegate = 0; /**/		// Marca se deve aplicar negação
	/*2*/int isOtimes = 0; /**/		// Armazena operador multiplicativo ('*' ou '/')
	/*3*/int isOplus = 0; /**/		// Armazena operador aditivo ('+' ou '-')

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
			// Somente int32
			/**/fprintf(objcode, "\tmovl $%s, %%eax\n", lexeme);/**/
			match(DEC); 
			break;
		case FLT: // Número ponto flutuante
			match(FLT); 
			break;
		default: // Identificador (variável)
			// F -> ID [ := E ]
			/**/strcpy(varname, lexeme);/**/ // Tem que salvar antes do match senão perde o nome
			match(ID);
			if(lookahead == ASGN) {
				match(ASGN);
				E(); // Traz o resultado no acumulador (acc)
				/**/fprintf(objcode, "\tmovl %%eax, %s\n", varname);/**/
			} else {
				/**/fprintf(objcode, "\tmovl %%eax, %s\n", varname);/**/
			}
	}

	// Término do fator

	/**/ 
	if(isOtimes){ // Se havia operador multiplicativo pendente
		if(isOtimes == '*') {
			fprintf(objcode, "\timull (%%esp)\n");
			fprintf(objcode, "\taddl $4, %%esp\n");
		} else {
			fprintf(objcode, "\tmovl %%eax, %%ecx\n");
			fprintf(objcode, "\tpopl %%eax\n");
			fprintf(objcode, "\tcltq\n");
			fprintf(objcode, "\tidivl %%ecx\n");
		}
		isOtimes = 0;
	}
	/**/
	
	// Se próximo token for '*' ou '/', continua reconhecendo fator
	if (lookahead == '*' || lookahead == '/') {
		/*10*/isOtimes = lookahead;/**/ // Guarda operador multiplicativo
		fprintf(objcode, "\tpushl %%eax\n");
		match(lookahead); 
		goto _Fbegin; // Volta para reconhecer novo fator
	}

	// Término do termo
	
	/**/
	if (isNegate) { // Se havia sinal negativo, aplica
		fprintf(objcode, "\tnegl %%eax\n");
		isNegate = 0;
	}
	/**/

	/**/
	if(isOplus) { // Se havia operador aditivo pendente
		if(isOplus == '+') {
			fprintf(objcode, "\taddl %%eax, (%%esp)\n");
		} else {
			fprintf(objcode, "\tsubl %%eax, (%%esp)\n");
		}
		fprintf(objcode, "\tpopl %%eax\n");

		isOplus = 0;
	}
	/**/

	// Se próximo token for '+' ou '-', continua reconhecendo termo
	if (lookahead == '+' || lookahead == '-') {
		/**/isOplus = lookahead; /**/ // Guarda operador aditivo
		/**/fprintf(objcode, "\tpushl %%eax\n");/**/
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
