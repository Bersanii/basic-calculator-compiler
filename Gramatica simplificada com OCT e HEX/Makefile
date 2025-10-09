HDIR = .
CFLAGS = -g -I$(HDIR)

mybc: lexer.o main.o parser.o
	$(CC) $^ -o $@

clean:
	$(RM) *.o mybc

mostlyclean: clean
	$(RM) *~

targz: # Para entregar 
	tar zcvf mybc_`date "+%Y%m%d"`.tar.gz Makefile *.[ch] ./versioned

targz-x: # Para extrair
	tar zxvf mybc.tar.gz Makefile *.[ch]