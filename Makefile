all: release debug

IMRC_BIN_DEP=.tmp/IMRC_models.o .tmp/IMRC_aux.o .tmp/IMRC_gl.o src/IMRC_main.c

IMRC_DBG_DEP=.tmp/IMRC_models_dbg.o .tmp/IMRC_aux_dbg.o .tmp/IMRC_gl_dbg.o src/IMRC_main.c

IMRC_DEP=src/IMRC_main.c src/IMRC_models.c src/IMRC_aux.c src/IMRC_aux.h src/IMRC_gl.c src/IMRC_gl.h src/IMRC_models.h src/IMRC_types.h

release: ${IRMC_DEP}
	./prep.sh
	mkdir -p .tmp
	gcc -std=c99 -Wall -pedantic -O2 -c src/IMRC_gl.c -lglfw -lGL -o .tmp/IMRC_gl.o
	gcc -std=c99 -Wall -pedantic -O2 -c src/IMRC_aux.c -o .tmp/IMRC_aux.o
	gcc -std=c99 -Wall -pedantic -O2 -pthread -c src/IMRC_models.c -lm -o .tmp/IMRC_models.o
	gcc -std=c99 -Wall -pedantic -O2 -s -pthread ${IMRC_BIN_DEP} -lm -lglfw -lGL -L/usr/lib/ -o bin/IMRC
debug: ${IRMC_DEP} 
	./prep.sh
	mkdir -p .tmp
	gcc -DDEBUG -g -p -std=c99 -Wall -pedantic -O0 -c src/IMRC_gl.c -lglfw -lGL -o .tmp/IMRC_gl_dbg.o
	gcc -DDEBUG -g -p -std=c99 -Wall -pedantic -O0 -c src/IMRC_aux.c -o .tmp/IMRC_aux_dbg.o
	gcc -DDEBUG -g -p -std=c99 -Wall -pedantic -O0 -pthread -c src/IMRC_models.c -lm -o .tmp/IMRC_models_dbg.o
	gcc -DDEBUG -g -p -std=c99 -Wall -pedantic -O0 -pthread ${IMRC_DBG_DEP} -lm -lglfw -lGL -o bin/IMRC_dbg
clean:
	rm -rfv .tmp
