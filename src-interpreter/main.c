#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "parser.h"

FILE *source, *objcode;

int main(void)
{
	lookahead = gettoken(source = stdin);
	objcode = stdout;

	mybc();

	return 0;
}
