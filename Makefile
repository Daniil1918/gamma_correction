all: gamma_correction

gamma_correction:
	gcc -o progname gamma_correction.c -lpng -ldl -lm

clean:
	rm -rf progname
