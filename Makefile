thread: thread.o
	gcc -Wall -O2 -o thread thread.o

thread.o: thread.c
	gcc -c thread.c -lpthread

PHONY: check-syntax
check-syntax:
	gcc -Wall -fsyntax-only $(CHK_SOURCES) 2>&1 | sed -e 's/警告/warning/' -e 's/エラー/error/'
