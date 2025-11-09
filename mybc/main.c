// Grupo 3
// 1 - Vitor Bersani Balan
// 2 - Luan de Souza Araújo
// 3 - Juliano dos Reis Cruz

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "main.h"
#include "parser.h"

FILE *source, *objcode;

void handle_sigint(int sig) {
    (void)sig;
    printf("\n");
    fflush(stdout);
}

int main(void)
{
	signal(SIGINT, handle_sigint);  // Captura Ctrl+C

	lookahead = gettoken(source = stdin);
	objcode = stdout;

	mybc();

	return 0;
}
