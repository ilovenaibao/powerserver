
SUBDIRS = source include 
HAVE_TEST:=

test:
	@if [ -f tester/Makefile  ] ;then  ( cd tester && $(MAKE) ) ; else 	echo please enable test via --enable-test ;	fi 

clean_gch: clean
	$(RM) source/*.gch

