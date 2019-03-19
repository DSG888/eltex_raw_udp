#include "other.h"

int getrand(int min, int max) {
	return (double)rand() / (RAND_MAX + 1.0) * (max - min) + min;
}

double wtime() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

void DieWithError(char *errorMessage) {
	if (errorMessage)
		fprintf(stderr, "%s\n", errorMessage);
	getchar();
	exit(EXIT_FAILURE);
}
