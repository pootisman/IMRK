all: release

IMRC_BIN_DEP=.tmp/IMRC_models.o src/IMRC_main.c

IMRC_DEP=src/IMRC_main.c src/IMRC_models.c src/IMRC_models.h src/IMRC_types.h

release: ${IRMC_DEP}
	mkdir .tmp
	gcc -std=c99 -Wall -pedantic -O2 -c src/IMRC_models.c -lm -o .tmp/IMRC_models.o
	gcc -std=c99 -Wall -pedantic -O2 -s ${IMRC_BIN_DEP} -lm -o bin/IMRC
	rm -rvf .tmp
debug: src/IMRC_main.c
	gcc -g -std=c99 -Wall -pedantic src/IMRC_main.c -lm -o bin/IMRC_dbg
