HDIR = .
CFLAGS = -g -I$(HDIR)

mypas: lexer.o main.o parser.o keywords.o symtab.o
	$(CC) $^ -o $@

clean:
	$(RM) *.o mypas

mostlyclean: clean
	$(RM) *~

targz: # Para entregar 
	tar zcvf mybc_`date "+%Y%m%d"`.tar.gz Makefile *.[ch] ./versioned

targz-x: # Para extrair
	tar zxvf mybc.tar.gz Makefile *.[ch]