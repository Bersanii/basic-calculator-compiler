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

// program -> PROGRAM ID '(' idlist ')' ';' block '.'
void program(void) {
	match(PROGRAM);
	match(ID);
	match('(');
	idlist();
	match(')');
	match(';');
	block();
	match('.');
}

// idlist -> ID { ',' ID }
void idlist(void) {
	match(ID);
	while (lookahead == ',') {
		match(',');
		match(ID);
	}
}
// void idlist(void) {
// 	_idHead:
// 	match(ID);
// 	if (lookahead == ',') {
// 		match(',');
// 		goto _idHead;
// 	}
// }

// block -> varDeclaration
// 			sbProgram
//			beginEnd
void block(void) {
	varDeclaration();
	sbProgram();
	beginEnd();
}

// varDeclaration -> { VAR idlist ':' type ';' { idlist ':' type ';' } }
void varDeclaration(void) {
	if (lookahead == VAR) {
		match(VAR);
		_idList_head:
		idlist();
		match(':');
		type();
		match(';');
		if(lookahead == ID) {
			goto _idList_head;
		}
	}
}

// type -> INTEGER | REAL | BOOLEAN
void type(void) {
	switch (lookahead) {
		case INTEGER:
		case REAL:
			match(lookahead);
			break;
		default:
			match(BOOLEAN);
			break;
    }
}

// sbProgram -> { PROCEDURE ID parmList ';' block ';' | FUNCTION ID parmList ':' type block ';' }
void sbProgram(void) {
	while (lookahead == PROCEDURE || lookahead == FUNCTION) {
		if(lookahead == PROCEDURE) {
			match(PROCEDURE);
			match(ID);
			parmList();
			match(';');
			block();
			match(';');
		} else {
			match(FUNCTION);
			match(ID);
			parmList();
			match(':');
			type();
			match(';');
			block();
			match(';');
		}
	}
}

// parmList -> '{' { VAR } idlist ':' type { ';' } '}' 
void parmList(void) {
	if (lookahead == '{') {
		match('{');
		_parmList_head:
		if (lookahead == VAR) {
			match(VAR);
		}
		idlist();
		match(':');
		type();
		if(lookahead == ';') {
			match(';');
			goto _parmList_head;
		}
		match('}');
	}
}

// beginEnd -> BEGIN stmtList END
void beginEnd(void) {
	match(BEGIN);
	stmtList();
	match(END);
}

// TODO: gramatica
void stmtList(void) {
	_stmtList_head:
	stmt();
	if(lookahead == ';') {
		match(';');
		goto _stmtList_head;
	}
}

// TODO: gramatica
void stmt(void) {
	switch (lookahead) {
		case ID: // Pode ser variavel funcção ou procedure
			idStmt();
			break;
		case BEGIN:
			beginEnd();
			break;
		case IF:
			ifStmt();
		case WHILE:
			whileStmt();
		case REPEAT:
			repStmt();
		default: // Empty statement
			break;
	}
}

// TODO: gramatica
void idStmt(void) {
	match(ID);
	argList();
}

// TODO: gramatica
void argList(void) {
	if(lookahead == '(') {
		match('(');
		_exprList_head:
		expression();
		if(lookahead == ',') {
			match(',');
			goto _exprList_head;
		}
		match(')');
	}
}

// TODO: gramatica
void ifStmt(void) {
	match(IF);
	expression();
	match(THEN);
	stmt();
	if (lookahead == ELSE) {
		match(ELSE);
		stmt();
	}
}

// TODO: gramatica
void whileStmt(void) {
	// TODO
}

// TODO: gramatica
void repStmt(void) {
	// TODO
}

int isRelOp(void) {
	switch (lookahead) {
		case '<':
		case LEQ:
		case NEQ:
		case '>':
		case GEQ:
			return lookahead;
	}
	return 0;
}

void expression(void) {
	simpleExp();
	if (isRelOp()) {
		match(lookahead);
		simpleExp();
	}
	
}

// Oplus = ['+''-']
// Ominus = ['+''-']
void simpleExp(void) { 

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
			match('('); expression(); match(')');
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
				expression(); // Traz o resultado no acumulador (acc)
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
