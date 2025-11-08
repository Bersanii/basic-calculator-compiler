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

void program(void);
void expression(void);
void simpleExp(void);
void idlist(void);
void block(void);
void varDeclaration(void);
void type(void);
void sbProgram(void);
void parmList(void);
void beginEnd(void);
void stmtList(void);
void stmt(void);
void idStmt(void);
void ifStmt(void);
void whileStmt(void);
void repStmt(void);
void expressionList(void);
