#include <string.h>
#include "keywords.h"

char *keywords[] = {
    "begin",
    "if",
    "then",
    "else",
    "repeat",
    "until",
    "while",
    "do",
    "or",
    "div",
    "mod",
    "and",
	"true",
	"false",
    "end",
};

int isKeyword(const char * name) {
    int i;
    for (i = BEGIN; i <= END; i++)
    {
        if(strcmp(name, keywords[i - BEGIN]) == 0) return i;
    }
    
    return 0;
}