all: release debug testtools

IMRC_BIN_DEP=.tmp/IMRC_models.o .tmp/IMRC_aux.o .tmp/IMRC_ver.o .tmp/IMRC_gl.o src/IMRC_main.c

IMRC_DBG_DEP=.tmp/IMRC_models_dbg.o .tmp/IMRC_aux_dbg.o .tmp/IMRC_ver_dbg.o .tmp/IMRC_gl_dbg.o src/IMRC_main.c

IMRC_DEP=src/IMRC_main.c src/IMRC_models.c src/IMRC_aux.c src/IMRC_aux.h src/IMRC_ver.h src/IMRC_ver.c src/IMRC_gl.c src/IMRC_gl.h src/IMRC_models.h src/IMRC_types.h

testtools:
	./prep.sh
	gcc -Wall -pedantic -O2 src/tests/M_test.c -lm -lgsl -lgslcblas -o bin/M_test
	gcc -Wall -pedantic -O2 src/tests/CLR_test.c -lm -o bin/CLR_test
release: ${IRMC_DEP}
	./prep.sh
	mkdir -p .tmp
	gcc -Wall -pedantic -O2 -c src/IMRC_gl.c -lglfw -o .tmp/IMRC_gl.o
	gcc -Wall -pedantic -O2 -c src/IMRC_ver.c -lglfw -o .tmp/IMRC_ver.o
	gcc -Wall -pedantic -O2 -c src/IMRC_aux.c -lm -o .tmp/IMRC_aux.o
	gcc -Wall -pedantic -O2 -pthread -c src/IMRC_models.c -lm -o .tmp/IMRC_models.o
	gcc -Wall -pedantic -O2 -s -pthread ${IMRC_BIN_DEP} -lm -lglfw -lGL -o bin/IMRC
debug: ${IRMC_DEP} 
	./prep.sh
	mkdir -p .tmp
	gcc -DDEBUG -g -p -Wall -pedantic -O0 -c src/IMRC_gl.c -lglfw -lGL -o .tmp/IMRC_gl_dbg.o
	gcc -DDEBUG -g -p -Wall -pedantic -O0 -c src/IMRC_ver.c -lglfw -o .tmp/IMRC_ver_dbg.o
	gcc -DDEBUG -g -p -Wall -pedantic -O0 -c src/IMRC_aux.c -lm -o .tmp/IMRC_aux_dbg.o
	gcc -DDEBUG -g -p -Wall -pedantic -O0 -pthread -c src/IMRC_models.c -lm -o .tmp/IMRC_models_dbg.o
	gcc -DDEBUG -g -p -Wall -pedantic -O0 -pthread ${IMRC_DBG_DEP} -lm -lglfw -lGL -o bin/IMRC_dbg
clean:
	rm -rfv .tmp
