all: C FORTRAN

FORTRAN:
	gfortran -fall-intrinsics -O3 src/Integration.F -o bin/Fortran_integration
C:
	gcc -O3 src/Integration.c -o bin/C_integration
