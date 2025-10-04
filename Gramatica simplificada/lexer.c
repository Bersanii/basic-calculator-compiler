#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

// Buffer global para armazenar o lexeme (token reconhecido)
char lexeme[MAXIDLEN + 1];

int lineno = 1;
int colno = 0; // Começa com 0 pois não foi lido nenhum caractere da coluna ainda
/* 
--------------------------------------------------------------------
Identificadores em estilo Pascal
ID = [A-Za-z][A-Za-z0-9]* 
- Inicia com uma letra (maiúscula ou minúscula)
- Pode conter letras e números após o primeiro caractere
--------------------------------------------------------------------
*/
int isID(FILE *tape)
{
	// Primeiro caractere deve ser letra
	if ( isalpha(lexeme[0] = tracked_getc(tape)) ) {
		int i = 1;

		// Continua enquanto for alfanumérico
		while ( isalnum( lexeme[i] = tracked_getc(tape) ) ) i++;
		
		// Devolve o último caractere lido que não pertence ao identificador
		tracked_ungetc(lexeme[i], tape); 
		lexeme[i] = 0;

		if(strcmp(lexeme, "exit") == 0) {
			return EXIT;
		}
		if(strcmp(lexeme, "quit") == 0) {
			return QUIT;
		}
		
		return ID;
	}

	// Não é identificador, desfaz leitura
	tracked_ungetc(lexeme[0], tape);
	lexeme[0] = 0;
	return 0;
}

/*
--------------------------------------------------------------------
Números decimais
DEC = [1-9][0-9]* | '0'
- Se começa com dígito != 0, aceita sequência de dígitos
- Se começa com '0', aceita apenas '0'
--------------------------------------------------------------------
*/
int isDEC(FILE *tape)
{
	// Primeiro caractere deve ser número
	if ( isdigit(lexeme[0] = tracked_getc(tape)) ) {
		
		// Caso especial '0'
		if (lexeme[0] == '0') {
			return DEC;
		}
		
		int i = 1;
		// Continua enquanto for numérico
		while ( isdigit(lexeme[i] = tracked_getc(tape)) ) i++; 
		
		// Devolve o último caractere lido que não pertence ao número
		tracked_ungetc(lexeme[i], tape);
		lexeme[i] = 0;

		return DEC;
	}

	// Não é Decimal, desfaz a leitura
	tracked_ungetc(lexeme[0], tape);
	lexeme[0] = 0;
	return 0;
}

/*
--------------------------------------------------------------------
Expoente
EE = [eE]['+''-']?[0-9][0-9]*
--------------------------------------------------------------------
*/
int isEE(FILE *tape) {
	
	int i = strlen(lexeme); // Continua a leitura considerando que já existe algo no lexeme

	// Detecta 'e' ou 'E'
	if ( toupper(lexeme[i] = tracked_getc(tape)) == 'E' ) { 
		i++;
		
		// Valida se existe o sinal, se sim ativa a flag (opcional)
		int hassign = 0;
		if(((lexeme[i] = tracked_getc(tape)) == '+') || (lexeme[i] == '-')) {
			hassign = 1;
			i++;
		} else {
			hassign = 0;
			tracked_ungetc(lexeme[i], tape); // Se não for um sinal devolve para ser processado a frente
		}

		if (isdigit(lexeme[i] = tracked_getc(tape))) { // Primeiro caractere deve ser número
			i++;
			
			// Continua enquanto for numérico
			while( isdigit(lexeme[i] = tracked_getc(tape)) ) i++; 
			
			// Devolve o último caractere lido que não pertence ao número
			tracked_ungetc(lexeme[i], tape);
			lexeme[i] = 0;
			return FLT; // EE é considerado como ponto flutuante
		}
		
		// Primeiro caractere não é um número, dezfaz a leitura
		tracked_ungetc(lexeme[i],tape);
		lexeme[i] = 0;
		i--;

		// Se foi detectado sinal, dezfaz a leitura do mesmo
		if(hassign) {
			tracked_ungetc(lexeme[i],tape);
			lexeme[i] = 0;
			i--;
		}
	}

	// Não é exponencial, desfaz a leitura
	tracked_ungetc(lexeme[i],tape); 
	lexeme[i] = 0;
	return 0;
}

/*
--------------------------------------------------------------------
Reconhecimento de números (inteiros, float e exponenciais)

Formas aceitas:
- Inteiro decimal: DEC
- Ponto Flutuante: DEC'.'[0-9]* | '.'[0-9]+
- Exponencial: DEC[eE]['+''-']?[0-9][0-9]* | FLT [eE]['+''-']?[0-9][0-9]*
--------------------------------------------------------------------
*/
int isNUM(FILE *tape)
{
	int token = isDEC(tape); // Valida se é decimal

	if(token == DEC)
	{
		int i = strlen(lexeme); // Continua a leitura considerando que já existe algo no lexeme
		
		// Valida se após o decimal existe um '.' (Marcador de ponto flutuante)
		if((lexeme[i] = tracked_getc(tape)) == '.') {
			i++;
			token = FLT; // Atualiza o tipo para ponto flutuante
			
			// Continua enquanto for numérico
			while ( isdigit( lexeme[i] = tracked_getc(tape) ) ) i++; 
			
			// Devolve o último caractere lido que não pertence ao número
			tracked_ungetc(lexeme[i], tape);
			lexeme[i] = 0;
		} else {
			// Devolve o último caractere lido (Mantém como decimal)
			tracked_ungetc(lexeme[i], tape);
			lexeme[i] = 0;
		}
	} else {
		// Se não houver decimal o primeiro caractere deve ser '.' para ser um número
		if((lexeme[0] = tracked_getc(tape)) == '.') {
			int i = 1;
			
			// Primeiro caractere deve ser digito
			if (isdigit(lexeme[i] = tracked_getc(tape))) { 
				i++;
				token = FLT; // Atualiza o tipo para ponto flutuante
				
				// Continua enquanto for numérico
				while (isdigit(lexeme[i] = tracked_getc(tape))) i++; 

				// Devolve o último caractere lido que não pertence ao número
				tracked_ungetc(lexeme[i], tape);
				lexeme[i] = 0;
			} else {
				// Não é número, desfaz leitura
				tracked_ungetc(lexeme[i], tape);
				lexeme[i] = 0;
				i--;

				// Devolve o '.'
				tracked_ungetc(lexeme[i], tape);
				lexeme[i] = 0;

				return 0; // Não é um numero de ponto flutuante
			}
		} else {
			// Não é número, desfaz leitura
			tracked_ungetc(lexeme[0], tape);
			lexeme[0] = 0;
			return 0; // Não é um numero de ponto flutuante
		}
	}

	// Se for DEC ou FLT ainda pode ser EE
	if(isEE(tape)) {
		token = FLT;
	}

	return token;
}

/*
--------------------------------------------------------------------
Octal
OCT = '0'[0-7]+
--------------------------------------------------------------------
*/
int isOCT(FILE *tape)
{
	lexeme[0] = tracked_getc(tape);

	if (lexeme[0] == '0') {  // Um número octal deve começar com '0'
		int i = 1;

		// Verifica se o próximo caractere está entre '0' e '7'
		if ((lexeme[i] = tracked_getc(tape)) >= '0' && lexeme[i] <= '7') {
			i++;

			// Continua lendo enquanto for dígito válido em octal (0–7)
			while ((lexeme[i] = tracked_getc(tape)) >= '0' && lexeme[i] <= '7') i++; 

			// Devolve o caractere que não pertence ao número
			tracked_ungetc(lexeme[i], tape);
			lexeme[i] = 0;

			return OCT; // Reconhecido como número octal
		}

		// Caso o segundo caractere não seja octal, desfaz a leitura 
		tracked_ungetc(lexeme[i], tape);
		lexeme[i] = 0;

		tracked_ungetc('0', tape);
		lexeme[0] = 0;
	} else {
		// Se não começar com '0', não é octal, desfaz leitura
		tracked_ungetc(lexeme[0], tape);
		lexeme[0] = 0;
	}

	return 0; // Não é octal
}

/*
--------------------------------------------------------------------
Hexadecimal
HEX = '0'[Xx][0-9A-Fa-f]+
- Começa com '0x' ou '0X'
- Usa função isxdigit() para validar [0-9A-Fa-f]
--------------------------------------------------------------------
*/
int isHEX(FILE *tape)
{
	lexeme[0] = tracked_getc(tape);

	// Hexadecimal deve começar com '0'
	if ( lexeme[0] == '0' ) {
		
		// Verifica se o próximo caractere é 'x' ou 'X'
		if ( toupper(lexeme[1] = tracked_getc(tape)) == 'X' ) {
			int i = 2;

			// O próximo caractere deve ser um dígito hexadecimal válido
			if ( isxdigit(lexeme[i] = tracked_getc(tape)) ) {
				i++;

				// Continua lendo enquanto for dígito hexadecimal
				while ( isxdigit(lexeme[i] = tracked_getc(tape))) i++;

				// Devolve o primeiro caractere inválido
				tracked_ungetc(lexeme[i], tape);
				lexeme[i] = 0;

				return HEX; // Reconhecido como número hexadecimal
			}
			
			// Caso não haja dígito válido após o '0x'
			tracked_ungetc(lexeme[i], tape); // Devolve caractere inválido
			lexeme[i] = 0;

			tracked_ungetc(lexeme[1], tape); // Devolve o 'x' ou 'X'
			lexeme[1] = 0;

			tracked_ungetc(lexeme[0], tape); // Devolve o '0'
			return 0;
		}

		// Se depois do '0' não vier 'x' ou 'X', desfaz leitura
		tracked_ungetc(lexeme[1], tape);
		lexeme[1] = 0;

		tracked_ungetc(lexeme[0], tape);
		lexeme[0] = 0;	
		
		return 0;
	}

	// Se o primeiro caractere não for '0', desfaz leitura
	tracked_ungetc(lexeme[0], tape);
	lexeme[0] = 0;

	return 0;
}

/*
--------------------------------------------------------------------
Função auxiliar para ignorar espaços em branco
(espaço, tab, quebras de linha etc.)
--------------------------------------------------------------------
*/
void skipspaces(FILE *tape)
{
	int head;
	while ( isspace(head = tracked_getc(tape)) ){
		if(head == '\n'){
			lineno++;
			colno = 0;
			break;
		}
	}
	tracked_ungetc(head, tape);
}

/*
--------------------------------------------------------------------
Função principal de análise léxica (scanner)
- Ignora espaços
- Testa ordem: Identificadores, Hexadecimal, Octal, Números
- Caso contrário, retorna caractere ASCII isolado como token
--------------------------------------------------------------------
*/
int gettoken(FILE *source)
{
	int token;

	skipspaces(source);

	if ( (token = isID(source)) ) return token;
	if ( (token = isHEX(source)) ) return token;
	if ( (token = isOCT(source)) ) return token;
	if ( (token = isNUM(source)) ) return token;

	lexeme[0] = token = tracked_getc(source);
	lexeme[1] = 0;

	// return an ASCII token
	return token;
}

/*
--------------------------------------------------------------------
Função wrapper para contar as colunas
--------------------------------------------------------------------
*/
int tracked_getc(FILE *tape) {
    int ch = getc(tape);
    colno++;
    return ch;
}

void tracked_ungetc(int ch, FILE *tape) {
    ungetc(ch, tape);
    colno--;
}
