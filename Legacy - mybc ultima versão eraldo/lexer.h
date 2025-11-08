
#define MAXIDLEN 256

extern char lexeme[];

extern int lineno;

int gettoken(FILE *source);

int tracked_getc(FILE *tape);
void tracked_ungetc(int ch, FILE *tape);
