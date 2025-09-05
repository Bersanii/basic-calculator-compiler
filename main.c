#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <main.h>
#include <parser.h>

FILE *source;

int main(void)
{
	lookahead = gettoken(source = stdin);

	E();

	printf("\n");

	return 0;
}
