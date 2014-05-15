CFLAGS=-Wall -pedantic

OPT_SUP=-O2 -s

DBG_SUP=-O0 -g -p

LDFLAGS=-lglfw -lGL -lm -pthread -lgslcblas -lgsl

IMRC_BIN_DEP=.tmp/IMRC_models.o .tmp/IMRC_aux.o .tmp/IMRC_ver.o .tmp/IMRC_gl.o .tmp/IMRC_pretty_output.o src/IMRC_main.c

IMRC_DBG_DEP=.tmp/IMRC_models_dbg.o .tmp/IMRC_aux_dbg.o .tmp/IMRC_ver_dbg.o .tmp/IMRC_gl_dbg.o .tmp/IMRC_pretty_output_dbg.o src/IMRC_main.c

IMRC_DEP=src/IMRC_main.c src/IMRC_models.c src/IMRC_aux.c src/IMRC_aux.h src/IMRC_ver.h src/IMRC_ver.c src/IMRC_gl.c src/IMRC_gl.h src/IMRC_models.h src/IMRC_types.h src/IMRC_pretty_output.h

all: release debug testtools

testtools:
	./prep.sh
	gcc $(CFLAGS) $(OPT_SUP) src/tests/M_test.c $(LDFLAGS) -o bin/M_test
	gcc $(CFLAGS) $(OPT_SUP) src/tests/CLR_test.c $(LDFLAGS) -o bin/CLR_test
release: ${IRMC_DEP}
	./prep.sh
	mkdir -p .tmp
	gcc $(CFLAGS) $(OPT_SUP) -c src/IMRC_pretty_output.c -o .tmp/IMRC_pretty_output.o
	gcc $(CFLAGS) $(OPT_SUP) -c src/IMRC_gl.c -o .tmp/IMRC_gl.o
	gcc $(CFLAGS) $(OPT_SUP) -c src/IMRC_ver.c -o .tmp/IMRC_ver.o
	gcc $(CFLAGS) $(OPT_SUP) -c src/IMRC_aux.c -o .tmp/IMRC_aux.o
	gcc $(CFLAGS) $(OPT_SUP) -c src/IMRC_models.c -o .tmp/IMRC_models.o
	gcc $(CFLAGS) $(OPT_SUP) ${IMRC_BIN_DEP} $(LDFLAGS) -o bin/IMRC
debug: ${IRMC_DEP} 
	./prep.sh
	mkdir -p .tmp
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) -c src/IMRC_pretty_output.c -o .tmp/IMRC_pretty_output_dbg.o
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) -c src/IMRC_gl.c -o .tmp/IMRC_gl_dbg.o
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) -c src/IMRC_ver.c -o .tmp/IMRC_ver_dbg.o
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) -c src/IMRC_aux.c -o .tmp/IMRC_aux_dbg.o
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) -c src/IMRC_models.c -o .tmp/IMRC_models_dbg.o
	gcc -DDEBUG $(CFLAGS) $(DBG_SUP) ${IMRC_DBG_DEP} $(LDFLAGS) -o bin/IMRC_dbg
clean:
	rm -rfv .tmp
