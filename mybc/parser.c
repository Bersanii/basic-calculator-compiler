#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "tokens.h"
#include "parser.h"

// ação semantica -> /**/ [...] /**/

int lookahead; // Token atual (lookahead) usado pelo parser

double acc; // Acumulador da expressão
#define STACKSIZE 1024
double stack[STACKSIZE]; // Pilha auxiliar
int sp = -1; // Topo da pilha

// Tabela de símbolos como dicionário dos valores armazenados na memória virtual
#define MAXSTENTRIES 4096
char symtab[MAXSTENTRIES][MAXIDLEN+1]; // Nomes das variáveis
int symtab_next_entry = 0; // Próximo slot disponível

double vmem[MAXSTENTRIES]; // Valores das variáveis
int address; // Armazena o endereço da variável na memória

/*
--------------------------------------------------------------------
Interpretador de comando
mybc -> cmd { cmdsep cmd } EOF
cmd -> E | exit | quit | <epsilon>
cmdsep -> ';' | '\n'
--------------------------------------------------------------------
*/
void mybc(void) {

    // Loop principal. Continua rodando até o usuário enviar EOF (Ctrl+D).
    while (lookahead != EOF) {
        
        // Tenta executar um comando
        cmd();

        // Após um comando, esperamos um separador ou o fim da entrada
        if (lookahead == ';' || lookahead == '\n') {
            
            // Controle de linhas para print de erro
            if(lookahead == '\n') {
                lineno++;
				colno = 0;
			}
            
            match(lookahead);
            
        } else if (lookahead == EOF) { // Se for EOF, quebra o loop para terminar
            break;
        } else {
            // ERRO: Temos "lixo" na entrada. 
            // Ex: "k:9" (é analisado como "k" e depois ":") 
			
			// Explicação: Como não existe cenário previsto para derivação ID -> ID -> DEC o parser identifica ID -> ID
			// como fim da expressão, ele tenta validar se tem um novo comando checando se o lookahead é ';' ou '/n', nesse caso é 9
			// então o match(EOF) era disparado.
            
            fprintf(stderr, "Syntax error at line %d column %d. Unexpected token ", lineno, colno);
            
            char *typename_lookahead = getEnumName(lookahead);
            if (strcmp(typename_lookahead, "") != 0) {
                fprintf(stderr, "%s", typename_lookahead);
            } else {
                fprintf(stderr, "'%c'", lookahead);
            }
            fprintf(stderr, "\n");

            // Descarta todos os tokens até encontrar uma nova linha, "limpa" a entrada e deixa o usuário digitar um novo comando.
            while (lookahead != '\n' && lookahead != EOF) {
                lookahead = gettoken(source);
            }
        }
    }

    // Quando o loop quebrar consome o EOF
    match(EOF);
}

/*
--------------------------------------------------------------------
Dunção para tratar entradas de comandos e casos especiais
--------------------------------------------------------------------
*/
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

/*
--------------------------------------------------------------------
Função para localizar o endereço da variavel na memória
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
Função para armazena no endereço associado ao ponteiro name
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

	/*0*/char varname[MAXIDLEN+1];/**/	
	/*1*/int isNegate = 0;/**/		// Marca se deve aplicar negação
	/*2*/int isOtimes = 0;/**/		// Armazena operador multiplicativo ('*' ou '/')
	/*3*/int isOplus = 0;/**/		// Armazena operador aditivo ('+' ou '-')

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
			/*1*/ acc = atoi(lexeme); /**/
			match(DEC); 
			break;
		case OCT: // Número octal
			/*2*/ acc = strtol(lexeme, NULL, 8); /**/
			match(OCT); 
			break;
		case HEX: // Número hexadecimal
			/*3*/ acc = strtol(lexeme, NULL, 16); /**/
			match(HEX); 
			break;
		case FLT: // Número ponto flutuante
			/*4*/ acc = atof(lexeme); /**/
			match(FLT); 
			break;
		default: // Identificador (variável) F -> ID [ := E ]

			/*5*/ strcpy(varname, lexeme); /**/ // Para não perde o nome da variável
			match(ID);
			
			//saber se é atribuição, caso não seja, traz o valor da variável para o acumulador direto
			if(lookahead == ASGN) {
				match(ASGN);
				E(); 			// Traz o resultado no acumulador (acc)
				/*6*/store(varname);/**/  // Armazena no endereço associado a varname
			} else {
				/*7*/acc = recall(varname);/**/
			}
	}

	// Término do fator

	/*8*/
	if(isOtimes){ // Se havia operador multiplicativo pendente
		if(isOtimes == '*') {
			stack[sp] = stack[sp] * acc;
		} else {
			stack[sp] = stack[sp] / acc;
		}
		acc = stack[sp]; 
		sp--;  // Desempilha

		isOtimes = 0;
	}
	/**/
	
	// Se próximo token for '*' ou '/', continua reconhecendo fator
	if (lookahead == '*' || lookahead == '/') {
		/*9*/isOtimes = lookahead;/**/ // Guarda operador multiplicativo
		/*10*/stack[++sp] = acc;/**/ 
		match(lookahead); 
		goto _Fbegin; // Volta para reconhecer novo fator
	}

	// Término do termo
	
	// Se havia sinal negativo, aplica
	/*11*/
	if (isNegate) {
		acc = -acc;
		isNegate = 0;
	}
	/**/

	// Se havia operador aditivo pendente, aplica
	/*12*/
	if(isOplus) { 
		if(isOplus == '+') {
			stack[sp] = stack[sp] + acc;
		} else {
			stack[sp] = stack[sp] - acc;
		}
		acc = stack[sp]; sp--; // Desempilha
		isOplus = 0;
	}
	/**/

	// Se próximo token for '+' ou '-', continua reconhecendo termo
	if (lookahead == '+' || lookahead == '-') {
		/*13*/isOplus = lookahead; /**/ // Guarda operador aditivo
		/*14*/stack[++sp] = acc;/**/
		match(lookahead); 
		goto _Tbegin; 	// Volta para reconhecer novo termo
	}
	// Término da expressão
}

/*
--------------------------------------------------------------------
Função de validação de token
Recebe o token esperado e o compara com o lookahead, se forem iguais
recupera o próximo token, caso contrário, mostra um erro.
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
