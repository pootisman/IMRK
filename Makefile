all: release

release: src/IMRC_main.c
	gcc -std=c99 -Wall -pedantic -O2 -s src/IMRC_main.c -lm -o bin/IMRC
debug: src/IMRC_main.c
	gcc -g -std=c99 -Wall -pedantic src/IMRC_main.c -lm -o bin/IMRC_dbg
