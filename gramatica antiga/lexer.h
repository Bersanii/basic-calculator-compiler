
#define MAXIDLEN 256

extern char lexeme[];

enum {
	ID = 1024,
	DEC,
	FLT,
	OCT,
	HEX,
};

extern int lineno;

int gettoken(FILE *source);
