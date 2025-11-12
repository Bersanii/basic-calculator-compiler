#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "tokens.h"
#include "parser.h"

// /**/ ação semantica /**/

int lookahead; // Token atual (lookahead) usado pelo parser

// Interpretador de comando
// mybc -> cmd { cmdsep cmd } EOF
// cmd -> E | exit | quit | <epsilon>
// cmdsep -> ';' | '\n'

void mybc(void){
	cmd();

	while (lookahead == ';' || lookahead == '\n') {
		// Controle de linhas para print de erro
		if(lookahead == '\n')
			lineno++;
		colno = 0;
		
		// cmdsep
		match(lookahead);
		
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

// Tabela de símbolos como dicionário dos valores armazenados na memória virtual
#define MAXSTENTRIES 4096
char symtab[MAXSTENTRIES][MAXIDLEN+1];
int symtab_next_entry = 0; // uso: strcpy(symtab[symtab_next_entry], name);

double vmem[MAXSTENTRIES];

int address; //armazena o endereço da variável na memória

/*
--------------------------------------------------------------------
Vai localizar o endereço da variavel na memória
--------------------------------------------------------------------
*/
double recall(const char *name) {

	// Busca bottom-up a variável chamada name
	// Começa de -1 pois symtab_next_entry sempre aponta para o próximo item vazio da tabela
	for(address = symtab_next_entry-1; address > -1; address--) {
		if(strcmp(symtab[address], name) == 0) {
			return vmem[address];
		}
	}

	// Variável ainda não existe
	address = symtab_next_entry;
	symtab_next_entry++;
	strcpy(symtab[address], name);

	return 0.e+00; // Enfatiza que é ponto flutuante
}

/*
--------------------------------------------------------------------
Armazena no endereço associado ao ponteiro name
--------------------------------------------------------------------
*/
void store(const char *name) {
	recall(name); 
	vmem[address] = acc;
}


/*
--------------------------------------------------------------------
E é o símbolo inicial da gramática LL(1) de expressões simplificadas
E -> [Ominus] T { Oplus T }
Oplus = ['+''-']
Ominus = ['+''-']
--------------------------------------------------------------------
*/
void E(void) { 

	/*0*/char varname[MAXIDLEN+1];
	/*1*/int isNegate = 0; /**/		// Marca se deve aplicar negação
	/*2*/int isOtimes = 0; /**/		// Armazena operador multiplicativo ('*' ou '/')
	/*3*/int isOplus = 0; /**/		// Armazena operador aditivo ('+' ou '-')

	// Trata opcional (+ ou -) antes do termo
	if(lookahead == '+' || lookahead == '-'){

		if (lookahead == '-') {// Se for '-', guarda para aplicar negação depois
			isNegate = lookahead; 
		}

		match(lookahead);
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
			/*1*/acc = atoi(lexeme);
			match(DEC); 
			break;
		case OCT: // Número octal
			/*2*/acc = strtol(lexeme, NULL, 8); 
			match(OCT); 
			break;
		case HEX: // Número hexadecimal
			/*3*/acc = strtol(lexeme, NULL, 16); 
			match(HEX); 
			break;
		case FLT: // Número ponto flutuante
			/*4*/acc = atof(lexeme);
			match(FLT); 
			break;
		default: // Identificador (variável) F -> ID [ := E ]

			strcpy(varname, lexeme); // Para não perde o nome da variável
			match(ID);
			
			//saber se é atribuição, caso não seja, traz o valor da variável para o acumulador direto
			if(lookahead == ASGN) {
				match(ASGN);
				E(); 			// Traz o resultado no acumulador (acc)
				store(varname); // Armazena no endereço associado a varname
			} else {
				acc = recall(varname);
			}
	}

	// Término do fator


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
	

	// Se próximo token for '*' ou '/', continua reconhecendo fator
	if (lookahead == '*' || lookahead == '/') {
		isOtimes = lookahead;// Guarda operador multiplicativo
		stack[++sp] = acc;
		match(lookahead); 
		goto _Fbegin; // Volta para reconhecer novo fator
	}

	// Término do termo
	
	// Se havia sinal negativo, aplica
	if (isNegate) { 
		// fprintf(objcode, " negate ");
		acc = -acc;
		isNegate = 0;
	}


	// Se havia operador aditivo pendente
	if(isOplus) { 
		// fprintf(objcode, " %c ", isOplus);
		if(isOplus == '+') {
			stack[sp] = stack[sp] + acc;
		} else {
			stack[sp] = stack[sp] - acc;
		}
		acc = stack[sp]; sp--;
		isOplus = 0;
	}

	// Se próximo token for '+' ou '-', continua reconhecendo termo
	if (lookahead == '+' || lookahead == '-') {
		isOplus = lookahead;	// Guarda operador aditivo
		stack[++sp] = acc;
		match(lookahead); 
		goto _Tbegin; 	// Volta para reconhecer novo termo
	}
	// Término da expressão
}

/*
--------------------------------------------------------------------
Parser components 
--------------------------------------------------------------------
*/
void match(int expected)
{
	if (lookahead == expected) {
		// Se o token atual é o esperado, consome e avança para o próximo
		lookahead = gettoken(source);
	} else {

		// Caso contrário, erro de análise
		fprintf(stderr, "Token mismatch at line %d column %d. ", lineno, colno);

		fprintf(stderr, "Expected ");
		
		
		char *typename = getEnumName(expected); // Função retorna uma string com o nome do identificar caso esteja mapeado, caso contrário, retorna vazio
		if (strcmp(typename, "") != 0) {
			fprintf(stderr, "%s", typename);
		} else {
			fprintf(stderr, "'%c'", expected);
		}

		fprintf(stderr, " got ");

		switch (lookahead) {
			case '\n': 
				fprintf(stderr, "'\\n'");
				break;
			case '\t': 
				fprintf(stderr, "'\\t'");
				break;
			case '\r': 
				fprintf(stderr, "'\\r'");
				break;
			case 27: 
				fprintf(stderr, "ESCAPE");
				break;
			default:
				char *typename_lookahead = getEnumName(lookahead); // Função retorna uma string com o nome do identificar caso esteja mapeado, caso contrário, retorna vazio
				if (strcmp(typename_lookahead, "") != 0) {
					fprintf(stderr, "%s", typename_lookahead);
				} else {
					fprintf(stderr, "'%c'", lookahead);
				}
				break;
		}

		fprintf(stderr, "\n");
	}
}
