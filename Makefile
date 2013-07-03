all: release debug

IMRC_BIN_DEP=.tmp/IMRC_models.o .tmp/IMRC_aux.o src/IMRC_main.c

IMRC_DEP=src/IMRC_main.c src/IMRC_models.c src/IMRC_aux.c src/IMRC_aux.h src/IMRC_models.h src/IMRC_types.h

release: ${IRMC_DEP}
	mkdir -p .tmp
	gcc -std=c89 -Wall -pedantic -O2 -c src/IMRC_aux.c -o .tmp/IMRC_aux.o
	gcc -std=c89 -Wall -pedantic -O2 -c src/IMRC_models.c -lm -o .tmp/IMRC_models.o
	gcc -std=c89 -Wall -pedantic -O2 -s ${IMRC_BIN_DEP} -lm -o bin/IMRC
debug: ${IRMC_DEP} 
	mkdir -p .tmp
	gcc -g -std=c89 -Wall -pedantic -O0 -c src/IMRC_aux.c -o .tmp/IMRC_aux_dbg.o
	gcc -g -std=c89 -Wall -pedantic -O0 -c src/IMRC_models.c -lm -o .tmp/IMRC_models_dbg.o
	gcc -g -std=c89 -Wall -pedantic -O0 ${IMRC_BIN_DEP} -lm -o bin/IMRC_dbg
clean:
	rm -rfv .tmp
