
#define MAXIDLEN 256

extern char lexeme[];

enum {
	ID = 1024,
	DEC,
	FLT,
	OCT,
	HEX,
};

int gettoken(FILE *source);
