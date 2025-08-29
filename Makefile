HDIR = .
CFLAGS = -g -I$(HDIR)

mybc: lexer.o main.o parser.o
	$(CC) $^ -o $@

clean:
	$(RM) *.o mybc

mostlyclean: clean
	$(RM) *~

targz: # Para entregar 
	tar zcvf mybc.tar.gz Makefile *.[ch]

targz-x: # Para extrair
	tar zxvf mybc.tar.gz Makefile *.[ch]