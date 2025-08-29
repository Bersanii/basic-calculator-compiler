#define ERRTOKEN -0x10000000

extern int lookahead;

extern char lexeme[];

extern FILE *source;

void match(int expected);

extern int gettoken(FILE *);

void E(void);
void T(void);
void F(void);
void Q(void);
void R(void);
