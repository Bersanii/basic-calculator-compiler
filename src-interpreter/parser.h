#define ERRTOKEN -0x10000000

extern int lookahead;
extern double acc;

extern char lexeme[];

extern int lineno;
extern int colno;

extern FILE *source;
extern FILE *objcode;

void match(int expected);

char* getEnumName(int value);

extern int gettoken(FILE *);

void mybc(void);
void cmd(void);
void E(void);
void T(void);
void F(void);
void Q(void);
void R(void);
