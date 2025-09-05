#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <lexer.h>

// Buffer global para armazenar o lexeme (token reconhecido)
char lexeme[MAXIDLEN + 1];

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
	if ( isalpha(lexeme[0] = getc(tape)) ) { // Primeiro caractere deve ser letra
		int i = 1;
		while ( isalnum( lexeme[i] = getc(tape) ) ) i++; // Continua enquanto for alfanumérico
		
		// Devolve o último caractere lido que não pertence ao identificador
		ungetc(lexeme[i], tape); 
		lexeme[i] = 0;
		
		return ID;
	}

	// Não é identificador, desfaz leitura
	ungetc(lexeme[0], tape);
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
	if ( isdigit(lexeme[0] = getc(tape)) ) { // Primeiro caractere deve ser número
		
		// Caso especial '0'
		if (lexeme[0] == '0') {
			return DEC;
		}

		int i = 1;
		while ( isdigit(lexeme[i] = getc(tape)) ) i++; // Continua enquanto fornumérico
		
		// Devolve o último caractere lido que não pertence ao número
		ungetc(lexeme[i], tape);
		lexeme[i] = 0;
		
		return DEC;
	}

	// Não é Decimal, desfaz a leitura
	ungetc(lexeme[0], tape);
	lexeme[0] = 0;
	return 0;
}

// fpoint = DEC \.[O-9]* | \.[0-9][0-9]*
// flt = fpoint EE? | DEC EE
// EE = [eE]['+''-']?[0-9][0-9]*
/*
--------------------------------------------------------------------
Expoente em números floats
EE = [eE]['+''-']?[0-9][0-9]*
--------------------------------------------------------------------
*/
int isEE(FILE *tape) {

	int i = strlen(lexeme);

	if((toupper(lexeme[i] = getc(tape)) == 'E')){

		i++;
		
		int hassign = 0;
		if(((lexeme[i] = getc(tape)) == '+') || (lexeme[i] == '-')) {
			hassign = 1;
			i++;
		} else {
			hassign = 0;
			ungetc(lexeme[i], tape); // se não for um sinal devolve para ser processado a frente
		}

		// Checa se é digito
		if (isdigit(lexeme[i] == getc(tape))) { // Somente o primeiro digito é obrigatório
			i++;
			while (isdigit(lexeme[i] == getc(tape))) i++;
			
			ungetc(lexeme[i], tape);
			lexeme[i] = 0;
			return FLT;
		}
		
		ungetc(lexeme[i],tape);
		lexeme[i] = 0;
		i--;

		if(hassign) {
			ungetc(lexeme[i],tape);
			lexeme[i] = 0;
			i--;
		}
	}

	ungetc(lexeme[i],tape); 
	lexeme[i] = 0;
	return 0; // nao eh exponencial
}

/*
--------------------------------------------------------------------
Reconhecimento de números (inteiros, float e exponenciais)

Formas aceitas:
- DEC (inteiro decimal)
- DEC '.' [0-9]* (número com parte fracionária)
- '.' [0-9]+ (número começando com ponto)
- Qualquer número com sufixo EE (expoente)
--------------------------------------------------------------------
*/
int isNUM(FILE *tape)
{
	int token = isDEC(tape);

	if(token == DEC)
	{
		int i = strlen(lexeme); // Se tem um decimal em lexeme a analise precisa continuar após o final dele
		if((lexeme[i] = getc(tape)) == '.') {
			i++;
			while (isdigit(lexeme[i] == getc(tape))) i++;
			ungetc(lexeme[i], tape);
			lexeme[i] = 0;return 1;
			token = FLT;
		} else {
			ungetc(lexeme[i], tape);
			lexeme[i] = 0;
		}
	} else {
		if((lexeme[0] = getc(tape)) == '.') {
			if (isdigit(lexeme[1] == getc(tape))) { // Somente o primeiro digito é obrigatório
				int i = 2;
				token = FLT;
				while (isdigit(lexeme[i] == getc(tape))) i++;
			} else {
				ungetc(lexeme[1], tape);
				lexeme[1] = 0;
				ungetc(lexeme[0], tape);
				lexeme[0] = 0;
				return 0; // Não é um numero
			}
		} else {
			ungetc(lexeme[0], tape);
			lexeme[0] = 0;
			return 0; // Não é ponto
		}
	}

	// Se for inteiro pode vir um exponencial
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
	lexeme[0] = getc(tape);

	if (lexeme[0] == '0') {
		int i = 1;
		if ((lexeme[i] = getc(tape)) >= '0' && lexeme[i] <= '7') {
			i++;
			while ((lexeme[i] = getc(tape)) >= '0' && lexeme[i] <= '7') i++;
			ungetc(lexeme[i], tape);
			lexeme[i] = 0;
			return OCT;
		}
		ungetc(lexeme[i], tape);
		lexeme[i] = 0;
		ungetc('0', tape);
		lexeme[0] = 0;
	} else {
		ungetc(lexeme[0], tape);
		lexeme[0];
	}
	return 0;
}

/*
	HEX = '0'[Xx][0-9A-Fa-f]+
 	isxdigit == [0-9A-Fa-f]
*/
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
	lexeme[0] = getc(tape);
	if ( lexeme[0] == '0' ) {
		
		if ( toupper(lexeme[1] = getc(tape)) == 'X' ) {
			
			int i = 2;
			if ( isxdigit(lexeme[i] = getc(tape)) ) {
				i++;
				while ( isxdigit(lexeme[i] = getc(tape))) i++;
				ungetc(lexeme[i], tape);
				lexeme[i] = 0;
				return HEX;
			}
			
			ungetc(lexeme[i], tape); // Devolve caractere após o x
			lexeme[i] = 0;
			ungetc(lexeme[1], tape); // Devolve caractere x
			lexeme[1] = 0;
			ungetc(lexeme[0], tape);  // Devolve caractere 0
			return 0;
		}

		ungetc(lexeme[1], tape);
		lexeme[1] = 0;
		ungetc(lexeme[0], tape);
		lexeme[0] = 0;	
		return 0;
	}
	ungetc(lexeme[0], tape);
	lexeme[0] = 0;
	return 0;
}

int lineno = 1;

/*
--------------------------------------------------------------------
Função auxiliar para ignorar espaços em branco
(espaço, tab, quebras de linha etc.)
--------------------------------------------------------------------
*/
void skipspaces(FILE *tape)
{
	int head;
	while ( isspace(head = getc(tape)) );
	ungetc(head, tape);
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

	lexeme[0] = token = getc(source);
	lexeme[1] = 0;

	// return an ASCII token
	return token;
}
