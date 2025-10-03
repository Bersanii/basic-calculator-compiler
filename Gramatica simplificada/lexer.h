
#define MAXIDLEN 256

extern char lexeme[];

enum {
	ID = 1024,
	DEC,
	OCT,
	HEX,
	FLT,
	EXIT,
	QUIT
};

extern int lineno;

int gettoken(FILE *source);
